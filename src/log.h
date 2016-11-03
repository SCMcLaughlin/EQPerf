
#ifndef LOG_H
#define LOG_H

#include "define.h"
#include "enum_log.h"
#include "eqp_tls.h"
#include "eqp_alloc.h"
#include "structs_log.h"

#define EQP_LOG_COMPRESS_THRESHOLD 1024

int log_init(void);
void log_deinit(void);

void log_msg(int type, const char* fmt, ...);
void log_as_id(int srcId, int type, const char* fmt, ...);

int log_register(int srcId);
int log_deregister(int srcId);

#endif/*LOG_H*/
