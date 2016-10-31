
#include "eqp_atomic.h"

void aint32_set(aint32_t* a, int32_t val)
{
    InterlockedExchange(a, val);
}

int32_t aint32_get(aint32_t* a)
{
    return InterlockedCompareExchange(a, 0, 0);
}

int aint32_cmp_xchg_weak(aint32_t* a, int32_t expected, int32_t desired)
{
    return aint32_cmp_xchg_strong(a, expected, desired);
}

int aint32_cmp_xchg_strong(aint32_t* a, int32_t expected, int32_t desired)
{
    return InterlockedCompareExchange(a, desired, expected) == expected;
}

void aint16_set(aint16_t* a, int16_t val)
{
    InterlockedExchange16(a, val);
}

int16_t aint16_get(aint16_t* a)
{
    return InterlockedCompareExchange16(a, 0, 0);
}

int aint16_cmp_xchg_weak(aint16_t* a, int16_t expected, int16_t desired)
{
    return aint16_cmp_xchg_strong(a, expected, desired);
}

int aint16_cmp_xchg_strong(aint16_t* a, int16_t expected, int16_t desired)
{
    return InterlockedCompareExchange16(a, desired, expected) == expected;
}

void amutex_unlock(amutex* mtx)
{
    aint32_set(mtx, 0);
}

void amutex_lock(amutex* mtx)
{
    for (;;)
    {
        if (amutex_try_lock(mtx))
            break;
    }
}

int amutex_try_lock(amutex* mtx)
{
    return aint32_cmp_xchg_strong(mtx, 0, 1);
}
