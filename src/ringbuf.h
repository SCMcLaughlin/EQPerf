
#ifndef RINGBUF_H
#define RINGBUF_H

#include "define.h"
#include "eqp_alloc.h"
#include "eqp_semaphore.h"
#include "structs_sync.h"

RingBuf* ringbuf_create(void);
int ringbuf_destroy(RingBuf* rb);

int ringbuf_init(RingBuf* rb);
int ringbuf_deinit(RingBuf* rb);

int ringbuf_wait(RingBuf* rb);
int ringbuf_try_wait(RingBuf* rb);
int ringbuf_trigger(RingBuf* rb);

int ringbuf_push(RingBuf* rb, const RingPacket* p);
int ringbuf_pop(RingBuf* rb, RingPacket* p);

#endif/*RINGBUF_H*/
