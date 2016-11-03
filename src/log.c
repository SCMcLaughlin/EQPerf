
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

#define MSG_OVERHEAD 64

static int log_msg_overhead(int type, char* str)
{
    time_t rawTime;
    struct tm* curTime;
    int len;
    int tagLen;
    
    rawTime = time(NULL);
    
    if (rawTime == -1) goto fail;
    
    curTime = localtime(&rawTime);
    
    if (!curTime) goto fail;
    
    len = strftime(str, MSG_OVERHEAD, "[%Y-%m-%d : %H:%M:%S] ", curTime);
    
    if (len <= 0) goto fail;
    
    switch (type)
    {
    case Log_Info:
        tagLen = snprintf(str + len, MSG_OVERHEAD - len, "|Info| ");
        break;
    
    case Log_Warning:
        tagLen = snprintf(str + len, MSG_OVERHEAD - len, "|Warning| ");
        break;
    
    case Log_Error:
        tagLen = snprintf(str + len, MSG_OVERHEAD - len, "|ERROR| ");
        break;
    
    case Log_Fatal:
        tagLen = snprintf(str + len, MSG_OVERHEAD - len, "|FATAL| ");
        break;
    
    case Log_Init:
        tagLen = snprintf(str + len, MSG_OVERHEAD - len, "|Init| ");
        break;
    
    case Log_SQL:
        tagLen = snprintf(str + len, MSG_OVERHEAD - len, "|SQL| ");
        break;
    
    case Log_Lua:
        tagLen = snprintf(str + len, MSG_OVERHEAD - len, "|Lua| ");
        break;
    
    default:
        tagLen = -1;
        break;
    }
    
    return (tagLen > 0) ? len + tagLen : len;
    
fail:
    return 0;
}

static void log_impl(int srcId, int type, const char* fmt, va_list args, va_list check)
{
    RingPacket rp;
    int len;
    int checkLen;
    int overheadLen;
    char* str;
    
    if (!sLog || !sLog->ringBuf)
        return;
    
    len = vsnprintf(NULL, 0, fmt, check);
    
    if (len <= 0)
        return;
    
    len += MSG_OVERHEAD + 1;
    str = alloc_array_type(len, char);
    
    if (!str)
        return;
    
    overheadLen = log_msg_overhead(type, str);
    checkLen    = vsnprintf(str + overheadLen, len - overheadLen, fmt, args);
    
    if (checkLen <= 0 || checkLen >= len)
    {
    abort:
        free(str);
        return;
    }
    
    ring_packet_init_src(&rp, srcId, EQPID_LogThread, RingOp_LogMessage, checkLen + overheadLen, str);
    
    if (ringbuf_push(sLog->ringBuf, &rp))
    {
        /* Push failed; this thread still has ownership of the string, so we destroy it here */
        goto abort;
    }
    
    thread_trigger(&sLog->thread);
}

#undef MSG_OVERHEAD

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
    
    if (!sLog || !sLog->ringBuf)
        return ERR_NotInitialized;
    
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
    
    if (!str)
        return;
    
    if (!lf || !lf->fp || !len)
        goto free_str;
    
    fp = lf->fp;
    
    fwrite(str, sizeof(char), len, fp);
    fputc('\n', fp);
    fflush(fp);
    
    lf->size += len + 1;
    /*fixme: check size limit*/
    
free_str:
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
        
        if (thread_wait(thread))
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
        
        if (thread_should_stop(thread))
            break;
    }
    
    /* Thread was told to stop, or we had an error waiting on the semaphore */
}
