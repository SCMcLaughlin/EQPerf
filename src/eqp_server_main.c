
#include "eqp_atomic.h"
#include "eqp_semaphore.h"
#include "ringbuf.h"

int main()
{
    aint32_t a;
    
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
}
