
#ifndef EQP_STRING_H
#define EQP_STRING_H

#include "define.h"
#include "eqp_alloc.h"
#include "structs_container.h"

SimpleString* sstr_create(const char* str, uint32_t len);
#define sstr_destroy(ss) free(ss)

uint32_t sstr_length(SimpleString* ss);
const char* sstr_data(SimpleString* ss);

#endif/*EQP_STRING_H*/
