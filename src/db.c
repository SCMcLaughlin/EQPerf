
#include "db.h"
#include "db_thread.h"
#include "db_query.h"
#include "db_transaction.h"

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
    aint32_set(&db->nextTransactId, 1);
    
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
        log_msg(Log_Info, "[%s] Shutting down database '%s'", FUNC, sstr_data(db->dbPath));
        sstr_destroy(db->dbPath);
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
    
    return (rc == SQLITE_OK) ? ERR_None : ERR_Invalid;
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
        log_msg(Log_Error, "[%s] Failed to schedule query %i for database '%s', err code: %i", FUNC, query_id(query), db_path(db), rc);
        query_destroy(query);
    }
    else
    {
        log_msg(Log_SQL, "[%s] Scheduled query %i for database '%s': '%s'", FUNC, query_id(query), db_path(db), sqlite3_sql(stmt));
    }
    
    return rc;
}

int db_sched_transact_ud(Database* db, TransactCB transCB, QueryCB queryCB, void* userdata)
{
    Transaction* trans = transact_create(db, transCB, queryCB, userdata);
    int rc;
    
    if (!trans) return ERR_OutOfMemory;
    
    rc = db_thread_sched(trans, true);
    
    if (rc && rc != ERR_Semaphore)
    {
        log_msg(Log_Error, "[%s] Failed to schedule transaction %i for database '%s', err code: %i", FUNC, transact_id(trans), db_path(db), rc);
        transact_destroy(trans);
    }
    else
    {
        log_msg(Log_SQL, "[%s] Scheduled transaction %i for database '%s'", FUNC, transact_id(trans), db_path(db));
    }
    
    return rc;
}

int db_next_query_id(Database* db)
{
    return aint32_add(&db->nextQueryId, 1);
}

int db_next_transact_id(Database* db)
{
    return aint32_add(&db->nextTransactId, 1);
}

const char* db_path(Database* db)
{
    SimpleString* path = db->dbPath;
    return path ? sstr_data(path) : "<unknown>";
}

int db_queue_callback(Database* db, RingPacket* rp)
{
    return ringbuf_push(db->callbackQueue, rp);
}

void db_exec_callbacks(Database* db)
{
    RingBuf* queue = db->callbackQueue;
    RingPacket rp;
    Query* query;
    
    while (!ringbuf_pop(queue, &rp))
    {
        switch (rp.opcode)
        {
        case RingOp_DbQuery:
            query = (Query*)rp.data;
            
            query_exec_callback(query);
            query_destroy(query);
            break;
        
        default:
            log_msg(Log_Warning, "[%s] Unexpected RingOp: %i", FUNC, rp.opcode);
            break;
        }
    }
}
