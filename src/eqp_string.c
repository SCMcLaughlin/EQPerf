
#include "eqp_string.h"

/* SimpleString */

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

/* String */

void str_init(String* str)
{
    str->length     = 0;
    str->capacity   = 0;
    str->data       = NULL;
}

void str_deinit(String* str)
{
    if (str->data)
    {
        free(str->data);
        str->data = NULL;
        
        str->length     = 0;
        str->capacity   = 0;
    }
}

uint32_t str_length(String* str)
{
    return str->length;
}

const char* str_data(String* str)
{
    return str->data;
}

int str_set(String* str, const char* cstr, uint32_t len)
{
    str_clear(str);
    return str_append(str, cstr, len);
}

int str_set_from_file(String* str, const char* path)
{
    str_clear(str);
    return str_append_file(str, path);
}

int str_set_from_file_ptr(String* str, FILE* fp)
{
    str_clear(str);
    return str_append_file_ptr(str, fp);
}

static int str_realloc(String* str, uint32_t newlen)
{
    uint32_t n = bit_pow2_greater_than_u32(newlen);
    char* data = realloc_bytes_type(str->data, n, char);
    
    if (!data) return false;
    
    str->capacity   = n;
    str->data       = data;
    
    return true;
}

int str_append(String* str, const char* input, uint32_t len)
{
    uint32_t cap;
    uint32_t newlen;
    uint32_t curlen;
    
    if (len == 0)
    {
        len = strlen(input);
        
        if (len == 0)
            return ERR_Invalid;
    }
    
    cap     = str->capacity;
    curlen  = str->length;
    newlen  = curlen + len;
    
    if (newlen >= cap && !str_realloc(str, newlen))
        return ERR_OutOfMemory;
    
    memcpy(str->data + curlen, input, len);
    str->data[newlen] = 0; /* Ensure the string is null terminated */
    
    str->length = newlen;
    
    return ERR_None;
}

int str_append_file(String* str, const char* path)
{
    FILE* fp = fopen(path, "rb");
    int rc;
    
    if (!fp) return ERR_CouldNotOpen;
    
    rc = str_append_file_ptr(str, fp);
    
    fclose(fp);
    
    return rc;
}

int str_append_file_ptr(String* str, FILE* fp)
{
    long len;
    uint32_t curlen;
    int rc;
    
    if (fseek(fp, 0, SEEK_END))
        return ERR_FileOperation;
    
    len = ftell(fp);
    
    if (len <= 0 || fseek(fp, 0, SEEK_SET))
        return ERR_FileOperation;
    
    curlen  = str->length;
    rc      = str_reserve(str, curlen + (uint32_t)len);
    
    if (rc) return rc;
    
    if (fread(str->data + curlen, sizeof(char), (size_t)len, fp) != (size_t)len)
    {
        rc = ERR_FileOperation;
    }
    else
    {
        curlen += len;
        str->length = curlen;
        rc          = ERR_None;
    }
    
    str->data[curlen] = 0; /* New null terminator, or ensure we still have our original null terminator on failure */
    
    return rc;
}

int str_reserve(String* str, uint32_t count)
{
    if (str->capacity <= count && !str_realloc(str, count))
        return ERR_OutOfMemory;
    
    return ERR_None;
}

void str_clear(String* str)
{
    str->length = 0;
}
