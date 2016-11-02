
#ifndef EQP_HASH_TBL_H
#define EQP_HASH_TBL_H

#include "define.h"
#include "hash.h"
#include "eqp_alloc.h"
#include "eqp_string.h"
#include "structs_container.h"

void tbl_init_size(HashTbl* tbl, uint32_t elemSize);
#define tbl_init(tbl, type) tbl_init_size((tbl), sizeof(type))
void tbl_deinit(HashTbl* tbl, HashTblCallback dtor);

int tbl_set_cstr(HashTbl* tbl, const char* key, uint32_t len, const void* value);
int tbl_set_int(HashTbl* tbl, int64_t key, const void* value);
#define tbl_set_ptr(tbl, ptr, val) tbl_set_int((tbl), (intptr_t)(ptr), (val))

void* tbl_get_cstr_raw(HashTbl* tbl, const char* key, uint32_t len);
#define tbl_get_cstr(tbl, key, len, type) (type*)tbl_get_cstr_raw((tbl), (key), (len))
void* tbl_get_int_raw(HashTbl* tbl, int64_t key);
#define tbl_get_int(tbl, key, type) (type*)tbl_get_int_raw((tbl), (key))
#define tbl_get_ptr(tbl, ptr, type) tbl_get_int(tbl, (intptr_t)(ptr), type)

int tbl_remove_cstr(HashTbl* tbl, const char* key, uint32_t len);
int tbl_remove_int(HashTbl* ptr, int64_t key);
#define tbl_remove_ptr(tbl, ptr) tbl_remove_int((tbl), (intptr_t)(ptr))

void tbl_for_each(HashTbl* tbl, HashTblCallback func);

#endif/*EQP_HASH_TBL_H*/
