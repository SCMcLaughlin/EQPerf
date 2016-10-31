
#ifndef STRUCTS_RINGBUF_H
#define STRUCTS_RINGBUF_H

#include "define.h"
#include "eqp_atomic.h"

#ifdef EQP_UNIX
# include <semaphore.h>
#endif

#define EQP_RINGBUF_MAX_PACKETS 2048

typedef struct Semaphore {
#ifdef EQP_WINDOWS
    HANDLE  handle;
#else
    sem_t   semaphore;
#endif
} Semaphore;

typedef struct RingPacket {
    int         dstId;
    int         srcId;
    uint32_t    opcode;
    uint32_t    length;
    void*       data;
} RingPacket;

typedef struct RingBlock {
    int32_t     nextIndex;
    aint16_t    hasBeenRead;
    aint16_t    hasBeenWritten;
    RingPacket  packet;
} RingBlock;

typedef struct RingBuf {
    RingBlock   blocks[EQP_RINGBUF_MAX_PACKETS];
    aint16_t    readStart;
    aint16_t    readEnd;
    aint16_t    writeStart;
    aint16_t    writeEnd;
    Semaphore   semaphore;
} RingBuf;

#endif/*STRUCTS_RINGBUF_H*/
