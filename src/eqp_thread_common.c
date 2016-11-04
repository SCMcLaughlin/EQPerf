
#include "eqp_thread.h"

int thread_send_stop_signal(Thread* thread)
{
    amutex_unlock(&thread->mtxShouldContinue);
    return semaphore_trigger(&thread->semaphore);
}

void thread_wait_until_stopped(Thread* thread)
{
    amutex* mtx = &thread->mtxLifetime;
    
    for (;;)
    {
        if (amutex_try_lock(mtx))
            break;
        
        clock_sleep(10);
    }
    
    amutex_unlock(mtx);
}

void thread_stop_all_in_one(Thread* thread)
{
    if (thread_is_running(thread) && !thread_send_stop_signal(thread))
        thread_wait_until_stopped(thread);
}

int thread_wait(Thread* thread)
{
    return semaphore_wait(&thread->semaphore);
}

int thread_try_wait(Thread* thread)
{
    return semaphore_try_wait(&thread->semaphore);
}

int thread_trigger(Thread* thread)
{
    return semaphore_trigger(&thread->semaphore);
}

int thread_is_running(Thread* thread)
{
    amutex* mtx = &thread->mtxLifetime;
    
    if (amutex_try_lock(mtx))
    {
        amutex_unlock(mtx);
        return false;
    }
    
    return true;
}

int thread_should_stop(Thread* thread)
{
    amutex* mtx = &thread->mtxShouldContinue;
    
    if (amutex_try_lock(mtx))
    {
        amutex_unlock(mtx);
        return true;
    }
    
    return false;
}
