
#include "eqp_string.h"

SimpleString* sstr_create(const char* str, uint32_t len)
{
    uint32_t dlen;
    byte* ptr;
    uint32_t* plen;
    
    if (len == 0 && str)
        len = strlen(str);
    
    dlen    = len + sizeof(uint32_t) + 1; /* Include null terminator for easy compatibility */
    ptr     = alloc_bytes(dlen);
    plen    = (uint32_t*)ptr;
    
    if (!ptr) return NULL;
    
    *plen = len;
    
    if (str)
        memcpy(ptr + sizeof(uint32_t), str, len);
    
    ptr[dlen - 1] = 0; /* Explicit null terminator, in case the source string did not have one */
    
    return (SimpleString*)ptr;
}

uint32_t sstr_length(SimpleString* ss)
{
    uint32_t* len = (uint32_t*)ss;
    return *len;
}

const char* sstr_data(SimpleString* ss)
{
    const char* str = (const char*)ss;
    return str + sizeof(uint32_t);
}
