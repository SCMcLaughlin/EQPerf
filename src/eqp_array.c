
#include "eqp_array.h"

#define MIN_CAPACITY 8

void array_init_size(Array* ar, uint32_t elemSize)
{
    ar->count       = 0;
    ar->capacity    = 0;
    ar->elemSize    = elemSize;
    ar->data        = NULL;
}

void array_deinit(Array* ar, ArrayCallback dtor)
{
    if (ar->data)
    {
        uint32_t index = 0;
        void* elem;
        
        while ((elem = array_get_raw(ar, index++)))
        {
            dtor(elem);
        }
        
        free(ar->data);
        
        ar->count       = 0;
        ar->capacity    = 0;
        ar->data        = NULL;
    }
}

void* array_get_raw(Array* ar, uint32_t index)
{
    return (ar->count > index) ? &ar->data[index * ar->elemSize] : NULL;
}

void* array_back_raw(Array* ar)
{
    uint32_t c = ar->count;
    return (c > 0) ? &ar->data[(c - 1) * ar->elemSize] : NULL;
}

int array_set(Array* ar, uint32_t index, const void* value)
{
    uint32_t size;
    
    if (index >= ar->count)
        return ERR_Invalid;

    size = ar->elemSize;
    memcpy(&ar->data[index * size], value, size);
    
    return ERR_None;
}

static int array_alloc_default(Array* ar)
{
    byte* data = alloc_bytes(ar->elemSize * MIN_CAPACITY);
    
    if (!data) return false;
    
    ar->capacity    = MIN_CAPACITY;
    ar->data        = data;
    
    return true;
}

static int array_realloc(Array* ar)
{
    uint32_t cap    = ar->capacity;
    uint32_t size   = ar->elemSize;
    byte* data;
    
    cap *= 2;
    data = realloc_bytes(ar->data, cap * size);
    
    if (!data) return false;
    
    ar->capacity    = cap;
    ar->data        = data;
    
    return true;
}

void* array_push_back(Array* ar, const void* value)
{
    uint32_t index  = ar->count;
    uint32_t cap    = ar->capacity;
    uint32_t size   = ar->elemSize;
    void* ptr;
    
    if (cap == 0)
    {
        if (!array_alloc_default(ar))
            return NULL;
    }
    else if (index >= cap)
    {
        if (!array_realloc(ar))
            return NULL;
    }
    
    ar->count   = index + 1;
    ptr         = &ar->data[index * size];
    
    if (value)
        memcpy(ptr, value, size);
    
    return ptr;
}

void array_pop_back(Array* ar)
{
    if (ar->count > 0)
        ar->count--;
}

int array_swap_and_pop(Array* ar, uint32_t index)
{
    if (ar->count > 0 && index < ar->count)
    {
        uint32_t back = --ar->count;
        
        if (index != back)
        {
            uint32_t size   = ar->elemSize;
            byte* data      = ar->data;
            
            memcpy(&data[index * size], &data[back * size], size);
            return true;
        }
    }
    
    return false;
}

void array_shift_left(Array* ar, uint32_t numIndices)
{
    uint32_t count  = ar->count;
    uint32_t size   = ar->elemSize;
    byte* data      = ar->data;
    
    if (count <= numIndices)
    {
        array_clear(ar);
        return;
    }
    
    if (!data) return;
    
    count -= numIndices;
    
    memmove(data, &data[numIndices * size], count * size);
    
    ar->count = count;
}

int array_reserve(Array* ar, uint32_t count)
{
    if (ar->capacity < count)
    {
        byte* data = realloc_bytes(ar->data, ar->elemSize * count);
        
        if (!data) return ERR_OutOfMemory;
        
        ar->capacity    = count;
        ar->data        = data;
    }
    
    return ERR_None;
}

void array_clear(Array* ar)
{
    ar->count = 0;
}

void array_clear_index_and_above(Array* ar, uint32_t index)
{
    if (index < ar->count)
        ar->count = index;
}

#undef MIN_CAPACITY
