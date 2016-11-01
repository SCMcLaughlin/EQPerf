
#include "eqp_semaphore.h"

int semaphore_init(Semaphore* ptr)
{
    if (sem_init(&ptr->semaphore, 0, 0))
    {
        //log error
        return ERR_CouldNotInit;
    }
    
    return ERR_None;
}

int semaphore_deinit(Semaphore* ptr)
{
    if (sem_destroy(&ptr->semaphore))
    {
        //log error
        return ERR_Generic;
    }
    
    return ERR_None;
}

int semaphore_wait(Semaphore* ptr)
{
    if (sem_wait(&ptr->semaphore))
    {
        //log error
        return ERR_Generic;
    }
    
    return ERR_None;
}

int semaphore_try_wait(Semaphore* ptr)
{
    if (sem_trywait(&ptr->semaphore))
    {
        int err = errno;
        return (err == EAGAIN) ? 1 : ERR_Generic;
    }
    
    return ERR_None;
}

int semaphore_trigger(Semaphore* ptr)
{
    if (sem_post(&ptr->semaphore))
    {
        return ERR_Generic;
    }
    
    return ERR_None;
}
