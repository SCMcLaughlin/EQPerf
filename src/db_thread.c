
#include "db_thread.h"

static DbThread* gDbThread;

static void db_thread_proc(Thread* thread, void* ptr);

int db_thread_init(void)
{
    gDbThread = alloc_type(DbThread);
    
    if (!gDbThread)
        return ERR_OutOfMemory;
    
    memset(gDbThread, 0, sizeof(DbThread));
    
    gDbThread->inputQueue = ringbuf_create();
    
    if (!gDbThread->inputQueue)
        return ERR_OutOfMemory;
    
    array_init(&gDbThread->activeQueries, Query*);
    array_init(&gDbThread->activeTransactions, Transaction*);
    
    return thread_start(EQPID_DbThread, db_thread_proc, &gDbThread->thread, NULL);
}

void db_thread_deinit(void)
{
    if (!gDbThread) return;
    
    /* Gracefully stop the thead */
    thread_stop_all_in_one(&gDbThread->thread);
    
    array_deinit(&gDbThread->activeQueries, NULL);
    array_deinit(&gDbThread->activeTransactions, NULL);
    
    if (gDbThread->inputQueue)
    {
        ringbuf_destroy(gDbThread->inputQueue);
        gDbThread->inputQueue = NULL;
    }
    
    free(gDbThread);
    gDbThread = NULL;
}

int db_thread_sched(void* ptr, int isTransaction)
{
    RingPacket rp;
    int rc;
    
    rc = ring_packet_init(&rp, EQPID_DbThread, isTransaction ? RingOp_DbTransaction : RingOp_DbQuery, 0, ptr);
    
    if (rc)
    {
    err:
        return rc;
    }
    
    rc = ringbuf_push(gDbThread->inputQueue, &rp);
    
    if (rc) goto err;
    
    return thread_trigger(&gDbThread->thread);
}

static void db_thread_push(Array* array, void* data, int isTransaction)
{
    if (array_push_back(array, (void*)&data))
        return;
    
    /*
        Push failed. Not likely to happen. In the unlikely event that it does,
        we will just abort the query or transaction to avoid the memory leak.
    */
    
    log_msg(Log_Error, "[%s] Failed to enqueue object %p (type: %s) for execution, aborting it", FUNC, data, isTransaction ? "Transaction" : "Query");
    
    /*if (isTransaction)
        transaction_destroy((Transaction*)data);
    else
        query_destroy((Query*)data);
    */
}

void db_thread_proc(Thread* thread, void* unused)
{
    RingBuf* inputQueue = gDbThread->inputQueue;
    Array* queries      = &gDbThread->activeQueries;
    Array* transactions = &gDbThread->activeTransactions;
    
    (void)unused;
    
    log_register(EQPID_DbThread);
    
    for (;;)
    {
        RingPacket rp;
        
        if (thread_wait(thread))
            break;
        
        while (!ringbuf_pop(inputQueue, &rp))
        {
            switch (rp.opcode)
            {
            case RingOp_DbQuery:
                db_thread_push(queries, rp.data, false);
                break;
            
            case RingOp_DbTransaction:
                db_thread_push(transactions, rp.data, true);
                break;
            
            default:
                break;
            }
        }
        
        /*fixme: execute queries here*/
        
        if (thread_should_stop(thread))
            break;
    }
    
    log_deregister(EQPID_DbThread);
}
