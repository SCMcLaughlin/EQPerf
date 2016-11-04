
#include "db.h"

Database* db_create(void)
{
    Database* db = alloc_type(Database);
    
    if (!db) return NULL;
    
    db_init(db);
    return db;
}

void db_destroy(Database* db)
{
    if (!db) return;
    
    db_deinit(db);
    free(db);
}

void db_init(Database* db)
{
    db->sqlite      = NULL;
    //db->dbThread    = NULL;//core_db_thread(core);
    db->dbPath      = NULL;
    aint32_set(&db->nextQueryId, 1);
}

void db_deinit(Database* db)
{
    sqlite3_close_v2(db->sqlite);
    
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
        //fixme: log err message
        return -1;
    }
    
    if (!schemaPath)
        return 0;
    
    schema = fopen(schemaPath, "rb");
    
    if (!schema)
    {
        //fixme: log err message
        return -1;
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
