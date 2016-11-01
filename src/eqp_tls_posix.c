
#include "eqp_tls.h"
#include <pthread.h>

static pthread_key_t sKeyId;

static void tls_destroy_id(void* ptr)
{
    (void)ptr;
    pthread_setspecific(sKeyId, NULL);
}

int tls_global_init_keys(void)
{
    return
        pthread_key_create(&sKeyId, tls_destroy_id)
        ? ERR_CouldNotInit : ERR_None;
}

int tls_init_thread(int srcId)
{
    return
        pthread_setspecific(sKeyId, (void*)(intptr_t)srcId)
        ? ERR_CouldNotInit : ERR_None;
}

void* tls_get_ptr(int key)
{
    pthread_key_t pkey;
    
    switch (key)
    {
    case TlsKey_Id:
        pkey = sKeyId;
        break;
    
    default:
        return NULL;
    }
    
    return pthread_getspecific(pkey);
}

int tls_get_int(int key, int* out)
{
    void* ptr = tls_get_ptr(key);
    
    if (!ptr)
        return ERR_Invalid;
    
    *out = (int)ptr;
    return ERR_None;
}
