
#include "db_row.h"

int row_int(Row* row, int column)
{
    return sqlite3_column_int(row, column);
}

int64_t row_int64(Row* row, int column)
{
    return sqlite3_column_int64(row, column);
}

double row_double(Row* row, int column)
{
    return sqlite3_column_double(row, column);
}

const char* row_str(Row* row, int column, uint32_t* len)
{
    if (len)
        *len = sqlite3_column_bytes(row, column);
    
    return (const char*)sqlite3_column_text(row, column);
}

const byte* row_blob(Row* row, int column, uint32_t* len)
{
    if (len)
        *len = sqlite3_column_bytes(row, column);
    
    return (const byte*)sqlite3_column_blob(row, column);
}

int row_is_null(Row* row, int column)
{
    return (sqlite3_column_type(row, column) == SQLITE_NULL);
}
