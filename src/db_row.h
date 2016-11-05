
#ifndef DB_ROW_H
#define DB_ROW_H

#include "define.h"
#include "log.h"
#include "structs_db.h"

int row_int(Row* row, int column);
int64_t row_int64(Row* row, int column);
double row_double(Row* row, int column);
const char* row_str(Row* row, int column, uint32_t* len);
const byte* row_blob(Row* row, int column, uint32_t* len);
int row_is_null(Row* row, int column);

#endif/*DB_ROW_H*/
