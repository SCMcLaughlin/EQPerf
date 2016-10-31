
#ifndef STRUCTS_DB_H
#define STRUCTS_DB_H

#include "define.h"
#include "eqp_atomic.h"
#include "structs_container.h"
#include <sqlite3.h>

typedef sqlite3_stmt Query;

typedef struct DbThread {
    
} DbThread;

typedef struct Database {
    sqlite3*    sqlite;
    DbThread*   dbThread;
    String*     dbPath;
    aint32_t    nextQueryId;
} Database;

struct QueryWrapper;
typedef void(*QueryCB)(struct QueryWrapper* qw);

typedef struct QueryWrapper {
    Query*      query;
    Database*   db;
    
    union {
        int     state;
        int64_t lastInsertId;
    };
    
    union {
        void*   userdata;
        int64_t userInt;
    };
    
    QueryCB     callback;
    uint32_t    queryId;
    int         affectedRows;
    uint64_t    timestamp;
} QueryWrapper;

struct TransactWrapper;
typedef void(*TransactCB)(struct TransactWrapper* tw);

typedef struct TransactWrapper {
    Database*   db;
    void*       userdata;
    TransactCB  callback;
    int         luaCallback;
    QueryCB     queryCallback;
} TransactWrapper;

#endif/*STRUCTS_DB_H*/
