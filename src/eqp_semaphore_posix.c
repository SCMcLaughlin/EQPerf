
#include "eqp_semaphore.h"

int semaphore_init(Semaphore* ptr)
{
    return sem_init(&ptr->semaphore, 0, 0) ? ERR_CouldNotInit : ERR_None;
}

int semaphore_deinit(Semaphore* ptr)
{
    return sem_destroy(&ptr->semaphore) ? ERR_Generic : ERR_None;
}

int semaphore_wait(Semaphore* ptr)
{
    return sem_wait(&ptr->semaphore) ? ERR_Generic : ERR_None;
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
    return sem_post(&ptr->semaphore) ? ERR_Generic : ERR_None;
}
