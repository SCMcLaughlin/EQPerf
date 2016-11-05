
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

SimpleString* sstr_from_file(const char* path)
{
    FILE* fp = fopen(path, "rb");
    SimpleString* str;
    
    if (!fp) return NULL;
    
    str = sstr_from_file_ptr(fp);
    
    fclose(fp);
    return str;
}

SimpleString* sstr_from_file_ptr(FILE* fp)
{
    long len;
    uint32_t dlen;
    byte* ptr;
    uint32_t* plen;
    
    if (fseek(fp, 0, SEEK_END))
        return NULL;
    
    len = ftell(fp);
    
    if (len <= 0 || fseek(fp, 0, SEEK_SET))
        return NULL;
    
    dlen    = (uint32_t)len + sizeof(uint32_t) + 1;
    ptr     = alloc_bytes(dlen);
    plen    = (uint32_t*)ptr;
    
    if (!ptr)
        return NULL;
    
    *plen = (uint32_t)len;
    
    if (fread(ptr + sizeof(uint32_t), sizeof(byte), (size_t)len, fp) != (size_t)len)
    {
        free(ptr);
        return NULL;
    }
    
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
