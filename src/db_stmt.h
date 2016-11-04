
#ifndef DB_STMT_H
#define DB_STMT_H

#include "define.h"
#include "log.h"
#include "structs_db.h"

int stmt_int(PreparedStmt* stmt, int column, int value);
int stmt_int64(PreparedStmt* stmt, int column, int64_t value);
int stmt_double(PreparedStmt* stmt, int column, double value);
int stmt_cstr(PreparedStmt* stmt, int column, const char* value, int len);
int stmt_cstr_no_copy(PreparedStmt* stmt, int column, const char* value, int len);
int stmt_blob(PreparedStmt* stmt, int column, const void* value, uint32_t len);
int stmt_blob_no_copy(PreparedStmt* stmt, int column, const void* value, uint32_t len);

#endif/*DB_STMT_H*/
