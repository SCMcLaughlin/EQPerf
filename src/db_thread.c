
#include "db_thread.h"
#include "db_query.h"

static DbThread* sDbThread;

static void db_thread_proc(Thread* thread, void* ptr);

int db_thread_init(void)
{
    sDbThread = alloc_type(DbThread);
    
    if (!sDbThread)
        return ERR_OutOfMemory;
    
    memset(sDbThread, 0, sizeof(DbThread));
    
    sDbThread->inputQueue = ringbuf_create();
    
    if (!sDbThread->inputQueue)
        return ERR_OutOfMemory;
    
    array_init(&sDbThread->activeQueries, Query*);
    array_init(&sDbThread->activeTransactions, Transaction*);
    
    return thread_start(EQPID_DbThread, db_thread_proc, &sDbThread->thread, NULL);
}

void db_thread_deinit(void)
{
    if (!sDbThread) return;
    
    /* Gracefully stop the thead */
    thread_stop_all_in_one(&sDbThread->thread);
    
    array_deinit(&sDbThread->activeQueries, NULL);
    array_deinit(&sDbThread->activeTransactions, NULL);
    
    if (sDbThread->inputQueue)
    {
        ringbuf_destroy(sDbThread->inputQueue);
        sDbThread->inputQueue = NULL;
    }
    
    free(sDbThread);
    sDbThread = NULL;
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
    
    rc = ringbuf_push(sDbThread->inputQueue, &rp);
    
    if (rc) goto err;
    
    return thread_trigger(&sDbThread->thread);
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
    else*/
        query_destroy((Query*)data);
}

static void db_thread_exec_transactions(Array* transactions)
{
    (void)transactions;
}

static void db_thread_exec_queries(Array* queries)
{
    uint32_t n = array_count(queries);
    uint32_t i = 0;
    
    while (i < n)
    {
        Query* query  = *array_get(queries, i, Query*);
        int rc;
        
        if (!query)
        {
        swap_and_pop:
            array_swap_and_pop(queries, i);
            n--;
            continue;
        }
        
        rc = query_exec_background(query);
        
        switch (rc)
        {
        case ERR_True: /* Execution completed */
            rc = query_queue_callback(query);
        
            if (rc)
            {
                log_msg(Log_Error, "[%s] Failed to queue successfully completed query for callback execution, err code: %i", rc);
                goto destroy;
            }
        
            goto swap_and_pop;
            
        case ERR_False: /* Could not complete query due to lock contention, keep trying */
            i++;
            break;
        
        default:
        destroy:
            query_destroy(query);
            goto swap_and_pop;
        }
    }
}

void db_thread_proc(Thread* thread, void* unused)
{
    RingBuf* inputQueue = sDbThread->inputQueue;
    Array* queries      = &sDbThread->activeQueries;
    Array* transactions = &sDbThread->activeTransactions;
    
    (void)unused;
    
    log_register(EQPID_DbThread);
    
    for (;;)
    {
        RingPacket rp;
        
        if (thread_wait(thread))
            break;
        
        for (;;)
        {
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
                    log_msg(Log_Warning, "[%s] Unexpected RingOp: %i", FUNC, rp.opcode);
                    break;
                }
            }
            
            db_thread_exec_transactions(transactions);
            db_thread_exec_queries(queries);
            
            if (array_empty(queries) && array_empty(transactions))
                break;
            
            clock_sleep(2);
        }
        
        if (thread_should_stop(thread))
            break;
    }
    
    log_deregister(EQPID_DbThread);
}
