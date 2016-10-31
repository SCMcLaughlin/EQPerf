
#include "eqp_semaphore.h"

int semaphore_init(Semaphore* ptr)
{
    if (sem_init(&ptr->semaphore, 0, 0))
    {
        //log error
        return -1;
    }
    
    return 0;
}

int semaphore_deinit(Semaphore* ptr)
{
    if (sem_destroy(&ptr->semaphore))
    {
        //log error
        return -1;
    }
    
    return 0;
}

int semaphore_wait(Semaphore* ptr)
{
    if (sem_wait(&ptr->semaphore))
    {
        //log error
        return -1;
    }
    
    return 0;
}

int semaphore_try_wait(Semaphore* ptr)
{
    if (sem_trywait(&ptr->semaphore))
    {
        int err = errno;
        return (err == EAGAIN) ? 1 : -1;
    }
    
    return 0;
}

int semaphore_trigger(Semaphore* ptr)
{
    if (sem_post(&ptr->semaphore))
    {
        return -1;
    }
    
    return 0;
}
