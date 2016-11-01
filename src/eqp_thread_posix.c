
#include "eqp_thread.h"
#include <pthread.h>

static void* thread_proc_wrapper(void* ptr)
{
    ThreadCreate* tc    = (ThreadCreate*)ptr;
    int srcId           = tc->srcId;
    ThreadProc func     = tc->func;
    void* userdata      = tc->userdata;
    Thread* thread      = tc->thread;
    
    free(tc);
    
    if (!tls_init_thread(srcId))
    {
        func(thread, userdata);
    }
    
    amutex_unlock(&thread->mtxLifetime);
    amutex_unlock(&thread->mtxShouldContinue);
    
    return NULL;
}

int thread_start(int srcId, ThreadProc func, Thread* thread, void* userdata)
{
    ThreadCreate* tc = alloc_type(ThreadCreate);
    pthread_t pthread;
    
    if (!tc) return ERR_OutOfMemory;
    
    tc->srcId       = srcId;
    tc->func        = func;
    tc->userdata    = userdata;
    tc->thread      = thread;
    
    amutex_init(&thread->mtxLifetime);
    amutex_lock(&thread->mtxLifetime);
    amutex_init(&thread->mtxShouldContinue);
    amutex_lock(&thread->mtxShouldContinue);
    
    if (pthread_create(&pthread, NULL, thread_proc_wrapper, tc) || pthread_detach(pthread))
    {
        free(tc);
        amutex_unlock(&thread->mtxLifetime);
        amutex_unlock(&thread->mtxShouldContinue);
        return ERR_CouldNotInit;
    }
    
    return ERR_None;
}
