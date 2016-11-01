
#include "eqp_thread.h"

static DWORD WINAPI thread_proc_wrapper(void* ptr)
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
    
    return 0;
}

int thread_start(int srcId, ThreadProc func, Thread* thread, void* userdata)
{
    ThreadCreate* tc = alloc_type(ThreadCreate);
    HANDLE handle;
    
    if (!tc) return ERR_OutOfMemory;
    
    tc->srcId       = srcId;
    tc->func        = func;
    tc->userdata    = userdata;
    tc->thread      = thread;
    
    amutex_init(&thread->mtxLifetime);
    amutex_lock(&thread->mtxLifetime);
    amutex_init(&thread->mtxShouldContinue);
    amutex_lock(&thread->mtxShouldContinue);
    
    handle = CreateThread(NULL, 0, thread_proc_wrapper, tc, 0, NULL);
    
    if (!handle)
    {
        free(tc);
        amutex_unlock(&thread->mtxLifetime);
        amutex_unlock(&thread->mtxShouldContinue);
        return ERR_CouldNotInit;
    }
    
    CloseHandle(handle);
    
    return ERR_None;
}
