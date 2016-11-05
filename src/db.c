
#include "db.h"
#include "db_thread.h"
#include "db_query.h"

Database* db_create(void)
{
    Database* db = alloc_type(Database);
    int rc;
    
    if (!db) return NULL;
    
    rc = db_init(db);
    
    if (rc)
    {
        free(db);
        return NULL;
    }
    
    return db;
}

void db_grab(Database* db)
{
    if (db)
        aint32_add(&db->refCount, 1);
}

void db_drop(Database* db)
{
    if (!db || aint32_sub(&db->refCount, 1) > 1)
        return;
    
    db_deinit(db);
    free(db);
}

int db_init(Database* db)
{
    db->sqlite          = NULL;
    db->callbackQueue   = ringbuf_create();
    db->dbPath          = NULL;
    aint32_set(&db->refCount, 1);
    aint32_set(&db->nextQueryId, 1);
    
    return (!db->callbackQueue) ? ERR_OutOfMemory : ERR_None;
}

void db_deinit(Database* db)
{
    sqlite3_close_v2(db->sqlite);
    
    if (db->callbackQueue)
    {
        ringbuf_destroy(db->callbackQueue);
        db->callbackQueue = NULL;
    }
    
    if (db->dbPath)
    {
        free(db->dbPath);
        db->dbPath = NULL;
    }
}

static int db_open_create(Database* db, const char* dbPath, const char* schemaPath)
{
    FILE* schema;
    SimpleString* sql;
    int rc;

    rc = sqlite3_open_v2(
        dbPath,
        &db->sqlite,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_CREATE,
        NULL
    );
    
    if (rc != SQLITE_OK)
    {
        log_msg(Log_Error, "[%s] Could not create database '%s', SQLite error (%i): '%s'", FUNC, dbPath, rc, sqlite3_errstr(rc));
        return ERR_CouldNotCreate;
    }
    
    if (!schemaPath)
        return ERR_None;
    
    schema = fopen(schemaPath, "rb");
    
    if (!schema)
    {
        log_msg(Log_Error, "[%s] Could not read schema file '%s', SQLite error (%i): '%s'", FUNC, schemaPath, rc, sqlite3_errstr(rc));
        return ERR_CouldNotOpen;
    }

    sql = sstr_from_file_ptr(schema);
    fclose(schema);
    
    if (!sql)
        return ERR_CouldNotOpen;
    
    rc = db_exec(db, sstr_data(sql));
    sstr_destroy(sql);
    
    return rc;
}

int db_open(Database* db, const char* dbPath, const char* schemaPath)
{
    int rc = sqlite3_open_v2(
        dbPath,
        &db->sqlite,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX,
        NULL
    );
    
    if (rc == SQLITE_CANTOPEN)
        rc = db_open_create(db, dbPath, schemaPath);
    
    if (rc == SQLITE_OK)
    {
        /* Record dbPath, log success */
        db->dbPath = sstr_create(dbPath, 0);
        log_msg(Log_Info, "[%s] Opened database '%s'", FUNC, dbPath);
    }
    
    return rc;
}

int db_exec(Database* db, const char* sql)
{
    char* err   = NULL;
    int rc      = sqlite3_exec(db->sqlite, sql, NULL, NULL, &err);
    
    if (err)
    {
        log_msg(Log_Error, "[%s] Error running SQL '%s'; SQLite error: '%s'", FUNC, sql, err);
        sqlite3_free(err);
    }
    
    return (rc == SQLITE_OK) ? 0 : -1;
}

PreparedStmt* db_prep(Database* db, const char* sql, int len)
{
    PreparedStmt* stmt = NULL;
    int rc;
    
    rc = sqlite3_prepare_v2(db->sqlite, sql, len, &stmt, NULL);
    
    if (rc != SQLITE_OK)
    {
        log_msg(Log_Error, "[%s] Prepared statement creation failed, SQLite error (%i): '%s'", FUNC, rc, sqlite3_errstr(rc));
    }
    
    return stmt;
}

int db_sched_ud(Database* db, PreparedStmt* stmt, QueryCB callback, void* userdata)
{
    Query* query = query_create(db, stmt, callback, userdata);
    int rc;
    
    if (!query) return ERR_OutOfMemory;
    
    rc = db_thread_sched(query, false);
    
    if (rc && rc != ERR_Semaphore)
    {
        log_msg(Log_Error, "[%s] Failed to schedule query, err code: %i", FUNC, rc);
        query_destroy(query);
    }
    
    return rc;
}

int db_sched_transact_ud(Database* db, TransactCB transCB, QueryCB queryCB, void* userdata)
{
    return 0;
}

int db_next_query_id(Database* db)
{
    return aint32_add(&db->nextQueryId, 1);
}
