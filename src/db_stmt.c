
#include "db_stmt.h"

int stmt_int(PreparedStmt* stmt, int column, int value)
{
    int rc = sqlite3_bind_int(stmt, column + 1, value);
    
    if (rc != SQLITE_OK)
    {
        log_msg(Log_Error, "[%s] SQLite error (%i): %s", FUNC, rc, sqlite3_errstr(rc));
        return ERR_Invalid;
    }
    
    return ERR_None;
}

int stmt_int64(PreparedStmt* stmt, int column, int64_t value)
{
    int rc = sqlite3_bind_int64(stmt, column + 1, value);
    
    if (rc != SQLITE_OK)
    {
        log_msg(Log_Error, "[%s] SQLite error (%i): %s", FUNC, rc, sqlite3_errstr(rc));
        return ERR_Invalid;
    }
    
    return ERR_None;
}

int stmt_double(PreparedStmt* stmt, int column, double value)
{
    int rc = sqlite3_bind_double(stmt, column + 1, value);
    
    if (rc != SQLITE_OK)
    {
        log_msg(Log_Error, "[%s] SQLite error (%i): %s", FUNC, rc, sqlite3_errstr(rc));
        return ERR_Invalid;
    }
    
    return ERR_None;
}

static int stmt_str_impl(PreparedStmt* stmt, int column, const char* value, int len, void (*type)(void*))
{
    int rc = sqlite3_bind_text(stmt, column + 1, value, len, type);
    
    if (rc != SQLITE_OK)
    {
        log_msg(Log_Error, "[%s] SQLite error (%i): %s", FUNC, rc, sqlite3_errstr(rc));
        return ERR_Invalid;
    }
    
    return ERR_None;
}

int stmt_str(PreparedStmt* stmt, int column, const char* value, int len)
{
    return stmt_str_impl(stmt, column, value, len, SQLITE_TRANSIENT);
}

int stmt_str_no_copy(PreparedStmt* stmt, int column, const char* value, int len)
{
    return stmt_str_impl(stmt, column, value, len, SQLITE_STATIC);
}

static int stmt_blob_impl(PreparedStmt* stmt, int column, const void* value, uint32_t len, void (*type)(void*))
{
    int rc = sqlite3_bind_blob(stmt, column + 1, value, len, type);
    
    if (rc != SQLITE_OK)
    {
        log_msg(Log_Error, "[%s] SQLite error (%i): %s", FUNC, rc, sqlite3_errstr(rc));
        return ERR_Invalid;
    }
    
    return ERR_None;
}

int stmt_blob(PreparedStmt* stmt, int column, const void* value, uint32_t len)
{
    return stmt_blob_impl(stmt, column, value, len, SQLITE_TRANSIENT);
}

int stmt_blob_no_copy(PreparedStmt* stmt, int column, const void* value, uint32_t len)
{
    return stmt_blob_impl(stmt, column, value, len, SQLITE_STATIC);
}

void stmt_abort(PreparedStmt* stmt)
{
    sqlite3_finalize(stmt);
}

int stmt_exec_transaction(PreparedStmt* stmt)
{
    int rc;
    
    do
    {
        rc = sqlite3_step(stmt);
    }
    while (rc == SQLITE_BUSY || rc == SQLITE_LOCKED);
    
    sqlite3_finalize(stmt);
    
    return (rc != SQLITE_DONE && rc != SQLITE_ROW) ? ERR_Invalid : ERR_None;
}
