
#ifndef EQP_STRING_H
#define EQP_STRING_H

#include "define.h"
#include "bit.h"
#include "eqp_alloc.h"
#include "structs_container.h"

/* SimpleString */

SimpleString* sstr_create(const char* str, uint32_t len);
#define sstr_destroy(ss) free(ss)
SimpleString* sstr_from_file(const char* path);
SimpleString* sstr_from_file_ptr(FILE* fp);

uint32_t sstr_length(SimpleString* ss);
const char* sstr_data(SimpleString* ss);

/* String */

void str_init(String* str);
void str_deinit(String* str);

uint32_t str_length(String* str);
const char* str_data(String* str);

int str_set(String* str, const char* cstr, uint32_t len);
int str_set_from_file(String* str, const char* path);
int str_set_from_file_ptr(String* str, FILE* fp);

int str_append(String* str, const char* input, uint32_t len);
int str_append_file(String* str, const char* path);
int str_append_file_ptr(String* str, FILE* fp);

int str_reserve(String* str, uint32_t bytes);
void str_clear(String* str);

#endif/*EQP_STRING_H*/
