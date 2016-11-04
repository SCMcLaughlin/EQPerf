
#include "log.h"

static LogThread* sLog;

#define NAME_LEN 256

static void log_thread_proc(Thread*, void*);
static void log_close_all_files(void* ptr);
static void log_close_all_compress_threads(void* ptr);
static int log_name_by_src(int srcId, char* name, const char* dir);

int log_init(void)
{
    sLog = alloc_type(LogThread);
    
    if (!sLog)
        return ERR_OutOfMemory;
    
    memset(sLog, 0, sizeof(LogThread));
    
    sLog->ringBuf = ringbuf_create();
    
    if (!sLog->ringBuf)
        return ERR_OutOfMemory;
    
    tbl_init(&sLog->logFiles, LogFile);
    array_init(&sLog->activeCompressThreads, Thread*);
    
    return thread_start(EQPID_LogThread, log_thread_proc, &sLog->thread, NULL);
}

void log_deinit(void)
{
    if (!sLog) return;
    
    /* Gracefully stop the thead */
    thread_stop_all_in_one(&sLog->thread);
    
    tbl_deinit(&sLog->logFiles, log_close_all_files);
    array_deinit(&sLog->activeCompressThreads, log_close_all_compress_threads);
    
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

void log_close_all_compress_threads(void* ptr)
{
    Thread** pp = (Thread**)ptr;
    
    if (pp)
    {
        Thread* thread = *pp;
        
        if (thread)
        {
            thread_wait_until_stopped(thread);
            free(thread);
        }
    }
}

static void log_compress_proc(Thread* thread, void* ptr)
{
    char cmd[1024];
    char* name = (char*)ptr;
    
    (void)thread;
    
    if (snprintf(cmd, sizeof(cmd), "gzip %s", name) > 0)
    {
        system(cmd);
    }
    
    free(name);
}

static void log_compress(LogFile* lf, int srcId)
{
    char cmd[1024];
    char oldName[NAME_LEN];
    char* newName;
    Thread* thread;
    int len;
    time_t rawTime;
    struct tm* curTime;
    
    if (log_name_by_src(srcId, oldName, "log") <= 0)
        return;
    
    newName = alloc_array_type(NAME_LEN, char);
    
    if (!newName)
        return;
    
    len = log_name_by_src(srcId, newName, "log/archive");
    
    if (len <= 0)
    {
    abort:
        free(newName);
        return;
    }
    
    rawTime = time(NULL);
    
    if (rawTime == -1)
        goto abort;
    
    curTime = localtime(&rawTime);
    
    if (!curTime)
        goto abort;
    
    len = strftime(newName + len, NAME_LEN - len, "-%Y-%m-%d-%H:%M:%S", curTime);
    
    if (len <= 0)
        goto abort;
    
    len = snprintf(cmd, sizeof(cmd), "mv %s %s", oldName, newName);
    
    if (len <= 0)
        goto abort;
    
    system(cmd);
    
    /*
        Beyond this point, we assume the log was successfully moved,
        and the open currently file (if any) can no longer be written to.
    
        If the thread or compression fails, that's okay -- we at least
        have the full log moved to log/archive/ and have the timestamp
        appended to its name. We can create a new file and move on
        without issue.
    */
    
    thread = alloc_type(Thread);
    
    if (!thread)
    {
    abort_thread:
        free(newName);
        goto close_open;
    }
    
    if (!array_push_back(&sLog->activeCompressThreads, (void*)&thread))
    {
        free(thread);
        goto abort_thread;
    }
    
    thread_start(EQPID_None, log_compress_proc, thread, newName);

close_open:
    if (lf->fp)
    {
        fclose(lf->fp);
        lf->fp = NULL;
    }
    
    lf->fp      = fopen(oldName, "w+");
    lf->size    = 0;
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

#ifdef EQP_LINUX
    if (lf->size >= EQP_LOG_COMPRESS_THRESHOLD)
        log_compress(lf, rp->srcId);
#endif
    
free_str:
    free(str);
}

int log_name_by_src(int srcId, char* name, const char* dir)
{
    int len;
    
    switch (srcId)
    {
    case EQPID_MainThread:
        len = snprintf(name, NAME_LEN, "%s/main.log", dir);
        break;
    
    case EQPID_DbThread:
        len = snprintf(name, NAME_LEN, "%s/db_thread.log", dir);
        break;
    
    default:
        len = snprintf(name, NAME_LEN, "%s/unknown.log", dir);
        break;
    }
    
    return len;
}

static void log_register_impl(RingPacket* rp, HashTbl* logFiles)
{
    int srcId = rp->srcId;
    char name[NAME_LEN];
    LogFile lf;
    
    if (log_name_by_src(srcId, name, "log") <= 0)
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
        lf.fp = NULL;
    }
    
#ifdef EQP_LINUX
    if (lf.size >= EQP_LOG_COMPRESS_THRESHOLD)
        log_compress(&lf, rp->srcId);
    else
        lf.fp = fopen(name, "a");
#else
    /* Open the file for appending */
    lf.fp = fopen(name, "a");
#endif
    
    if (lf.fp)
        tbl_set_int(logFiles, srcId, &lf);
}

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

static void log_check_threads(void)
{
    Array* array    = &sLog->activeCompressThreads;
    uint32_t index  = 0;
    
    for (;;)
    {
        Thread** pThread = array_get(array, index, Thread*);
        Thread* thread;
        
        if (!pThread)
            break;
        
        thread = *pThread;
            
        if (thread && !thread_is_running(thread))
        {
            free(thread);
            *pThread = NULL;
            array_swap_and_pop(array, index);
            continue;
        }
        
        index++;
    }
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
        
        log_check_threads();
        
        if (thread_should_stop(thread))
            break;
    }
    
    /* Thread was told to stop, or we had an error waiting on the semaphore */
}

#undef NAME_LEN
