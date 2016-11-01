
#include "eqp_clock.h"

static LARGE_INTEGER sFrequency;

static LARGE_INTEGER clock_win32()
{
    static BOOL sUseQpc = QueryPerformanceFrequency(&sFrequency);
    
    LARGE_INTEGER now;
    
    HANDLE thread       = GetCurrentThread();
    DWORD_PTR oldMask   = SetThreadAffinityMask(thread, 0);
    
    QueryPerformanceCounter(&now);
    SetThreadAffinityMask(thread, oldMask);
    
    return now;
}

uint64_t clock_milliseconds()
{
    LARGE_INTEGER li = clock_win32();
    return (uint64_t)((1000LL * li.QuadPart) / sFrequency.QuadPart);
}

uint64_t clock_microseconds()
{
    LARGE_INTEGER li = clock_win32();
    return (uint64_t)((1000000LL * li.QuadPart) / sFrequency.QuadPart);
}

uint64_t clock_unix_seconds()
{
    return _time64(NULL);
}

void clock_sleep_milliseconds(uint32_t ms)
{
    Sleep(ms);
}
