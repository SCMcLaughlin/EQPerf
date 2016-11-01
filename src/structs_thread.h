
#ifndef STRUCTS_THREAD_H
#define STRUCTS_THREAD_H

#include "define.h"
#include "eqp_atomic.h"
#include "eqp_semaphore.h"

typedef void(*ThreadProc)(void* ud);

typedef struct Thread {
    amutex      mtxLifetime;
    amutex      mtxShouldContinue;
    Semaphore   semaphore;
} Thread;

typedef struct ThreadCreate {
    int         srcId;
    ThreadProc  func;
    void*       userdata;
    Thread*     thread;
} ThreadCreate;

#endif/*STRUCTS_THREAD_H*/
