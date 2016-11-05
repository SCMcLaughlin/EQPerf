
#include "db_query.h"
#include "db.h"

Query* query_create(Database* db, PreparedStmt* stmt, QueryCB callback, void* userdata)
{
    Query* query = alloc_type(Query);
    
    if (!query)
        return NULL;
    
    query->stmt         = stmt;
    query->db           = db;
    query->hasResults   = false;
    query->userdata     = userdata;
    query->callback     = callback;
    query->queryId      = db_next_query_id(db);
    query->affectedRows = 0;
    query->lastInsertId = 0;
    perf_init(&query->perfTimer);
    
    db_grab(db);
    
    return query;
}

void query_destroy(Query* query)
{
    Database* db = query->db;
    
    sqlite3_finalize(query->stmt);
    query->stmt = NULL;
    
    if (db)
        db_drop(db);
    
    free(query);
}

int query_exec_background(Query* query)
{
    int rc = sqlite3_step(query->stmt);
    sqlite3* sqlite;
    
    switch (rc)
    {
    case SQLITE_BUSY:
        return ERR_False;
    
    case SQLITE_ROW:
        query->hasResults = true;
        log_msg(Log_SQL, "[%s] Executed query %i for database '%s' in %lu microseconds; results were returned; SQL: '%s'",
            FUNC, query_id(query), query_db_path(query), perf_microseconds(&query->perfTimer), sqlite3_sql(query->stmt));
        break;
    
    case SQLITE_DONE:
        sqlite              = query->db->sqlite;
        query->affectedRows = sqlite3_changes(sqlite);
        query->lastInsertId = sqlite3_last_insert_rowid(sqlite);
        query->hasResults   = false;
        log_msg(Log_SQL, "[%s] Executed query %i for database '%s' in %lu microseconds; affected rows: %i, last insert rowid: %ld; SQL: '%s'",
            FUNC, query_id(query), query_db_path(query), perf_microseconds(&query->perfTimer), query->affectedRows, query->lastInsertId, sqlite3_sql(query->stmt));
        break;
    
    default:
        log_msg(Log_Error, "[%s] Error while running query, SQLite error (%i): '%s'", FUNC, rc, sqlite3_errstr(rc));
        return ERR_Invalid;
    }
    
    return ERR_True;
}

Row* query_select(Query* query)
{
    sqlite3_stmt* stmt = query->stmt;
    int rc;
    
    /*
        The way we have things set up (to execute queries in the background) means that
        when we call this the first time, we will already have the first row of results
        queued up; need to special-case it.
    */
    if (query->hasResults)
    {
        query->hasResults = false;
        return (Row*)stmt;
    }
    
    do
    {
        rc = sqlite3_step(stmt);
    }
    while (rc == SQLITE_BUSY); /* Shouldn't happen now, as far as I know */
    
    switch (rc)
    {
    case SQLITE_ROW:
        return (Row*)stmt;
    
    default:
        log_msg(Log_Error, "[%s] Error while retrieving query results, SQLite error (%i): '%s'", FUNC, rc, sqlite3_errstr(rc));
        /* fallthrough */
    case SQLITE_DONE:
        sqlite3_reset(stmt);
        break;
    }
    
    return NULL;
}

void query_reset(Query* query)
{
    sqlite3_reset(query->stmt);
}

int query_queue_callback(Query* query)
{
    RingPacket rp;
    
    ring_packet_init_src(&rp, EQPID_DbThread, 0, RingOp_DbQuery, 0, query);
    
    return db_queue_callback(query->db, &rp);
}

void query_exec_callback(Query* query)
{
    QueryCB callback = query->callback;
    
    if (callback)
        callback(query);
}

const char* query_db_path(Query* query)
{
    return db_path(query->db);
}
