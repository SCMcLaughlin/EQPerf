
#include "eqp_atomic.h"
#include "eqp_semaphore.h"
#include "ringbuf.h"
#include "eqp_tls.h"
#include "eqp_thread.h"
#include "eqp_clock.h"
#include "container.h"

static void threadmain(Thread* thread, void* userdata)
{
    (void)thread;
    (void)userdata;
    int id = -1;
    
    tls_get_int(TlsKey_Id, &id);
    
    printf("thread says hi, tls id: %i\n", id);
}

int main()
{
    PerfTimer pt;
    aint32_t a;
    
    perf_init(&pt);
    
    aint32_set(&a, 5);
    
    printf("hello: %i\n", aint32_get(&a));
    
    Semaphore sem;
    semaphore_init(&sem);
    
    semaphore_trigger(&sem);
    if (!semaphore_try_wait(&sem))
        printf("semaphore worked\n");
    
    RingBuf* rb = ringbuf_create();
    if (!rb)
    {
        printf("ringbuf alloc failed\n");
    }
    else
    {
        RingPacket inp;
        inp.srcId = 1;
        inp.dstId = 99;
        
        ringbuf_push(rb, &inp);
        
        inp.srcId = 7777;
        
        if (!ringbuf_push(rb, &inp) && !ringbuf_wait(rb))
        {
            RingPacket p;
            
            while (!ringbuf_pop(rb, &p))
            {
                printf("ringbuf: src %i dst %i\n", p.srcId, p.dstId);
            }
        }
        
        ringbuf_destroy(rb);
    }
    
    if (tls_global_init_keys())
        printf("tls_init_keys failed\n");
    else if (tls_init_thread(EQPID_MainThread))
        printf("tls_init_thread failed\n");
    else
    {
        int id;
        printf("tls_init succeeded\n");
        
        if (tls_get_int(TlsKey_Id, &id))
            printf("tls_get_int failed\n");
        else
            printf("tls id: %i\n", id);
    }
    
    Thread* thread = alloc_type(Thread);
    
    if (!thread)
        printf("thread alloc failed\n");
    else if (thread_start(2, threadmain, thread, NULL))
        printf("thread_start failed\n");
    else
    {
        thread_wait_until_stopped(thread);
        printf("thread finished\n");
        free(thread);
    }
    
    HashTbl tbl;
    
    tbl_init(&tbl, int);
    
    int x = 99;
    tbl_set_int(&tbl, 7, &x);
    x = 66;
    tbl_set_cstr(&tbl, "hi there", 0, &x);
    
    x = *tbl_get_cstr(&tbl, "hi there", 0, int);
    printf("hash tbl: %i, ", x);
    x = *tbl_get_int(&tbl, 7, int);
    printf("%i\n", x);
    
    tbl_deinit(&tbl, NULL);

    printf("total time: %lu\n", perf_microseconds(&pt));
}
