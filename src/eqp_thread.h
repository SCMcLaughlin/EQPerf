
#ifndef EQP_THREAD_H
#define EQP_THREAD_H

#include "define.h"
#include "eqp_alloc.h"
#include "eqp_tls.h"
#include "structs_thread.h"

int thread_start(int srcId, ThreadProc func, Thread* thread, void* userdata);
int thread_send_stop_signal(Thread* thread);
void thread_wait_until_stopped(Thread* thread);

int thread_wait(Thread* thread);
int thread_try_wait(Thread* thread);
int thread_trigger(Thread* thread);

int thread_is_running(Thread* thread);
int thread_should_stop(Thread* thread);

#endif/*EQP_THREAD_H*/
