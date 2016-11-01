
#ifndef LOG_H
#define LOG_H

#include "define.h"
#include "eqp_tls.h"

enum LogType
{
    Log_Info,
    Log_Warning,
    Log_Error,
    Log_Fatal,
    Log_SQL,
    Log_Lua
};

int log_init(void);
int log_deinit(void);

void log(int type, const char* fmt, ...);
void log_as_id(int srcId, int type, const char* fmt, ...);

int log_register(int srcId);
int log_deregister(int srcId);

#endif/*LOG_H*/
