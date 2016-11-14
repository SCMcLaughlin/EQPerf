
#ifndef HASH_H
#define HASH_H

#include "define.h"
#include "bit.h"

uint32_t hash_int64(int64_t val);
uint32_t hash_str(const char* str, uint32_t len);

#endif/*HASH_H*/
