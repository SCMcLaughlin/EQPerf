
#ifndef ENUM_RINGBUF_H
#define ENUM_RINGBUF_H

enum RingBufOp
{
    RingOp_None,
    /* Logging */
    RingOp_LogMessage,
    RingOp_LogRegister,
    RingOp_LogDeregister,
    /* Database */
    RingOp_DbQuery,
    RingOp_DbTransaction,
    /* End */
    RingOp_COUNT
};

#endif/*ENUM_RINGBUF_H*/
