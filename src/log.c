
#include "log.h"

static LogThread* sLog;

int log_init(void)
{
    sLog = alloc_type(LogThread);
    
    if (!sLog) return -1;
}

int log_deinit(void)
{
    if (!sLog) return;
    
    free(sLog);
}

static void log_impl(int srcId, int type, const char* fmt, va_list args, va_list check)
{
    RingPacket rp;
    int len;
    int checkLen;
    char* str;
    va_list args;
    
    len = vsnprintf(NULL, 0, fmt, check);
    
    if (len <= 0)
        return;
    
    len++;
    str = alloc_array_type(len + sizeof(int), char);
    
    if (!str)
        return;
    
    memcpy(str, &type, sizeof(int));
    
    checkLen = vsnprintf(str + sizeof(int), len, fmt, args);
    
    if (checkLen <= 0 || checkLen >= len)
    {
        free(str);
        return;
    }
    
    rp.srcId    = srcId;
    rp.dstId    = EQPID_LogThread;
    rp.opcode   = 0;//RingOp_LogMessage;
    rp.length   = checkLen;
    rp.data     = str;
    
    if (ringbuf_push(&gLog->ringBuf, &rp))
    {
        /* Push failed; this thread still has ownership of the string, so we destroy it here */
        free(str);
    }
}

void log(int type, const char* fmt, ...)
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

int log_register(int srcId)
{
    
}

int log_deregister(int srcId)
{
    
}
