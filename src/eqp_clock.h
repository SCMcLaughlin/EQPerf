
#ifndef EQP_CLOCK_H
#define EQP_CLOCK_H

#include "define.h"
#include <time.h>

EQP_API uint64_t clock_milliseconds();
EQP_API uint64_t clock_microseconds();
EQP_API uint64_t clock_unix_seconds();
void clock_sleep_milliseconds(uint32_t ms);

#endif/*EQP_CLOCK_H*/
