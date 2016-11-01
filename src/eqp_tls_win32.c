
#include "eqp_tls.h"

static DWORD sKeyId;

int tls_global_init_keys(void)
{
    return
        ((sKeyId = TlsAlloc()) == TLS_OUT_OF_INDEXES)
        ? ERR_CouldNotInit : ERR_None;
}

int tls_init_thread(int srcId)
{
    return
        TlsSetValue(sKeyId, (void*)(intptr_t)srcId)
        ? ERR_None : ERR_CouldNotInit;
}

void* tls_get_ptr(int key)
{
    DWORD tkey;
    
    switch (key)
    {
    case TlsKey_Id:
        tkey = sKeyId;
        break;
    
    default:
        return NULL;
    }
    
    return TlsGetValue(tkey);
}

int tls_get_int(int key, int* out)
{
    void* ptr = tls_get_ptr(key);
    
    if (!ptr)
        return ERR_Invalid;
    
    *out = (int)ptr;
    return ERR_None;
}
