
#ifndef STRUCTS_LOG_H
#define STRUCTS_LOG_H

#include "define.h"
#include "eqp_thread.h"
#include "ringbuf.h"
#include "container.h"

typedef struct LogFile {
    FILE*       fp;
    uint32_t    size;
} LogFile;

typedef struct LogThread {
    Thread      thread;
    RingBuf*    ringBuf;
    HashTbl     logFiles;
    Array       activeCompressThreads;
} LogThread;

#endif/*STRUCTS_LOG_H*/
