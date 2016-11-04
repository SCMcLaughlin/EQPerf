
#include "db.h"
#include "db_thread.h"

extern DbThread* gDbThread;

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

void db_destroy(Database* db)
{
    if (!db) return;
    
    db_deinit(db);
    free(db);
}

int db_init(Database* db)
{
    db->sqlite          = NULL;
    db->callbackQueue   = ringbuf_create();
    db->dbPath          = NULL;
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
    char* sql;
    int len;
    int rc;

    rc = sqlite3_open_v2(
        dbPath,
        &db->sqlite,
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_FULLMUTEX | SQLITE_OPEN_CREATE,
        NULL
    );
    
    if (rc != SQLITE_OK)
    {
        log_msg(Log_Error, "[%s] Could not create database '%s', SQLite error (%i): %s", FUNC, dbPath, rc, sqlite3_errstr(rc));
        return ERR_CouldNotCreate;
    }
    
    if (!schemaPath)
        return ERR_None;
    
    schema = fopen(schemaPath, "rb");
    
    if (!schema)
    {
        log_msg(Log_Error, "[%s] Could not read schema file '%s', SQLite error (%i): %s", FUNC, schemaPath, rc, sqlite3_errstr(rc));
        return ERR_CouldNotOpen;
    }

    //fixme: replace with String funcs later
    fseek(schema, 0, SEEK_END);
    len = ftell(schema);
    fseek(schema, 0, SEEK_SET);
    
    sql = alloc_array_type(len + 1, char);
    
    if (!sql)
    {
        fclose(schema);
        return -2;
    }
    
    fread(sql, sizeof(char), len, schema);
    fclose(schema);
    
    rc = db_exec(db, sql);
    
    free(sql);
    
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
        //record dbPath, log success
    }
    
    return rc;
}

int db_exec(Database* db, const char* sql)
{
    char* err   = NULL;
    int rc      = sqlite3_exec(db->sqlite, sql, NULL, NULL, &err);
    
    if (err)
    {
        //fixme: log error message
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
        //fixme: log error message
    }
    
    return stmt;
}

int db_sched_ud(Database* db, PreparedStmt* stmt, QueryCB callback, void* userdata)
{
    return 0;
}

int db_sched_transact_ud(Database* db, TransactCB transCB, QueryCB queryCB, void* userdata)
{
    return 0;
}
