
#include "log.h"

static LogThread* sLog;

static void log_thread_proc(Thread*, void*);
static void log_close_all_files(void* ptr);

int log_init(void)
{
    int rc;
    
    sLog = alloc_type(LogThread);
    
    if (!sLog)
        return ERR_OutOfMemory;
    
    memset(sLog, 0, sizeof(LogThread));
    
    sLog->ringBuf = ringbuf_create();
    
    if (!sLog->ringBuf)
        return ERR_OutOfMemory;
    
    tbl_init(&sLog->logFiles, LogFile);
    
    rc = thread_start(EQPID_LogThread, log_thread_proc, &sLog->thread, NULL);
    
    return (rc != ERR_None) ? rc : ERR_None;
}

void log_deinit(void)
{
    if (!sLog) return;
    
    /* Gracefully stop the thead */
    if (thread_is_running(&sLog->thread) && !thread_send_stop_signal(&sLog->thread))
        thread_wait_until_stopped(&sLog->thread);
    
    tbl_deinit(&sLog->logFiles, log_close_all_files);
    
    if (sLog->ringBuf)
    {
        ringbuf_destroy(sLog->ringBuf);
        sLog->ringBuf = NULL;
    }
    
    free(sLog);
    sLog = NULL;
}

static void log_impl(int srcId, int type, const char* fmt, va_list args, va_list check)
{
    RingPacket rp;
    int len;
    int checkLen;
    char* str;
    
    len = vsnprintf(NULL, 0, fmt, check);
    
    if (len <= 0)
        return;
    
    len++;
    str = alloc_array_type(len + sizeof(int), char);
    
    if (!str)
        return;
    
    /* First four bytes encode the 'type' of the log message */
    memcpy(str, &type, sizeof(int));
    
    checkLen = vsnprintf(str + sizeof(int), len, fmt, args);
    
    if (checkLen <= 0 || checkLen >= len)
    {
    abort:
        free(str);
        return;
    }
    
    ring_packet_init_src(&rp, srcId, EQPID_LogThread, RingOp_LogMessage, checkLen, str);
    
    if (ringbuf_push(sLog->ringBuf, &rp))
    {
        /* Push failed; this thread still has ownership of the string, so we destroy it here */
        goto abort;
    }
    
    thread_trigger(&sLog->thread);
}

void log_msg(int type, const char* fmt, ...)
{
    int srcId;
    va_list args;
    va_list check;
    
    if (tls_get_int(TlsKey_Id, &srcId))
        return;
    
    va_start(args, fmt);
    va_start(check, fmt);
    
    log_impl(srcId, type, fmt, args, check);
    
    va_end(check);
    va_end(args);
}

void log_as_id(int srcId, int type, const char* fmt, ...)
{
    va_list args;
    va_list check;
    
    va_start(args, fmt);
    va_start(check, fmt);
    
    log_impl(srcId, type, fmt, args, check);
    
    va_end(check);
    va_end(args);
}

static int log_reg_impl(int srcId, int opcode)
{
    RingPacket rp;
    int rc;
    
    if (!sLog || !sLog->ringBuf) return ERR_NotInitialized;
    
    ring_packet_init_src(&rp, srcId, EQPID_LogThread, opcode, 0, NULL);
    rc = ringbuf_push(sLog->ringBuf, &rp);
    
    if (!rc)
        thread_trigger(&sLog->thread);
    
    return rc;
}

int log_register(int srcId)
{
    return log_reg_impl(srcId, RingOp_LogRegister);
}

int log_deregister(int srcId)
{
    return log_reg_impl(srcId, RingOp_LogDeregister);
}

void log_close_all_files(void* ptr)
{
    LogFile* lf = (LogFile*)ptr;
    
    if (lf->fp)
    {
        fclose(lf->fp);
        lf->fp = NULL;
    }
}

static void log_write_msg(RingPacket* rp, HashTbl* logFiles)
{
    LogFile* lf     = tbl_get_int(logFiles, rp->srcId, LogFile);
    char* str       = (char*)rp->data;
    uint32_t len    = rp->length;
    FILE* fp;
    
    if (!lf || !lf->fp || !str || !len)
        goto free_str;
    
    fp = lf->fp;
    
    fwrite(str + sizeof(int), sizeof(char), len, fp);
    fputc('\n', fp);
    fflush(fp);
    
    lf->size += len + 1;
    /*fixme: check size limit*/
    
free_str:
    if (str)
        free(str);
}

#define NAME_LEN 256

static int log_name_by_src(int srcId, char* name)
{
    int len;
    
    switch (srcId)
    {
    case EQPID_MainThread:
        len = snprintf(name, NAME_LEN, "log/main.log");
        break;
    
    default:
        len = snprintf(name, NAME_LEN, "log/unknown.log");
        break;
    }
    
    return (len <= 0);
}

static void log_register_impl(RingPacket* rp, HashTbl* logFiles)
{
    int srcId = rp->srcId;
    char name[NAME_LEN];
    LogFile lf;
    
    if (log_name_by_src(srcId, name))
        return;
    
    /* Do we already have a log open for this srcId? */
    if (tbl_has_int(logFiles, srcId))
        return;
    
    /* Find out the file's size, if it already exists */
    lf.fp   = fopen(name, "r");
    lf.size = 0;
    
    if (lf.fp)
    {
        fseek(lf.fp, 0, SEEK_END);
        lf.size = ftell(lf.fp);
        fclose(lf.fp);
    }
    
    /* Open the file for appending */
    lf.fp = fopen(name, "a");
    
    if (lf.fp)
        tbl_set_int(logFiles, srcId, &lf);
}

#undef NAME_LEN

static void log_deregister_impl(RingPacket* rp, HashTbl* logFiles)
{
    int srcId   = rp->srcId;
    LogFile* lf = tbl_get_int(logFiles, srcId, LogFile);
    
    if (!lf) return;
    
    if (lf->fp)
    {
        fclose(lf->fp);
        lf->fp = NULL;
    }
    
    tbl_remove_int(logFiles, srcId);
}

void log_thread_proc(Thread* thread, void* unused)
{
    RingBuf* ringBuf    = sLog->ringBuf;
    HashTbl* logFiles   = &sLog->logFiles;
    
    (void)unused;
    
    for (;;)
    {
        RingPacket rp;
        
        if (thread_wait(thread) || thread_should_stop(thread))
            break;
        
        while (!ringbuf_pop(ringBuf, &rp))
        {
            switch (rp.opcode)
            {
            case RingOp_LogMessage:
                log_write_msg(&rp, logFiles);
                break;
            
            case RingOp_LogRegister:
                log_register_impl(&rp, logFiles);
                break;
            
            case RingOp_LogDeregister:
                log_deregister_impl(&rp, logFiles);
                break;
            
            default:
                break;
            }
        }
    }
    
    /* Thread was told to stop, or we had an error waiting on the semaphore */
}
