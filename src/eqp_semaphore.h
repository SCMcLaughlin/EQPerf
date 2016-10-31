
#ifndef EQP_SEMAPHORE_H
#define EQP_SEMAPHORE_H

#include "define.h"
#include "structs_sync.h"

/*
    "Unnamed" Semaphore, no filesystem representation.
*/

int semaphore_init(Semaphore* ptr);
int semaphore_deinit(Semaphore* ptr);

int semaphore_wait(Semaphore* ptr);
int semaphore_try_wait(Semaphore* ptr);
int semaphore_trigger(Semaphore* ptr);

#endif/*EQP_SEMAPHORE_H*/
