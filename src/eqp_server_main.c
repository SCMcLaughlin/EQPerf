
#include "eqp_atomic.h"
#include "eqp_semaphore.h"
#include "ringbuf.h"
#include "eqp_tls.h"
#include "eqp_thread.h"
#include "eqp_clock.h"
#include "container.h"
#include "log.h"
#include "main_thread.h"

int main()
{
    PerfTimer pt;
    aint32_t a;
    
    perf_init(&pt);
    
    if (mt_global_init())
    {
        fputs("Initialization failed, aborting\n", stderr);
        return 1;
    }
    
    log_msg(Log_Info, "Test log message!");
    
    aint32_set(&a, 5);
    
    printf("hello: %i\n", aint32_get(&a));
    
    Semaphore sem;
    semaphore_init(&sem);
    
    semaphore_trigger(&sem);
    if (!semaphore_try_wait(&sem))
        printf("semaphore worked\n");
    
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
    
    clock_sleep(1000);
    mt_global_deinit();
}
