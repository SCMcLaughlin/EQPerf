
#ifndef EQP_CLOCK_H
#define EQP_CLOCK_H

#include "define.h"
#include <time.h>

EQP_API uint64_t clock_milliseconds();
EQP_API uint64_t clock_microseconds();
EQP_API uint64_t clock_unix_seconds();
void clock_sleep(uint32_t ms);

typedef uint64_t PerfTimer;

#define perf_init(p) ((*(p)) = clock_microseconds())
#define perf_microseconds(p) (clock_microseconds() - (*(p)))

#endif/*EQP_CLOCK_H*/
