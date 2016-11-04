
#ifndef STRUCTS_DB_H
#define STRUCTS_DB_H

#include "define.h"
#include "eqp_atomic.h"
#include "eqp_thread.h"
#include "container.h"
#include "ringbuf.h"
#include <sqlite3.h>

#define STMT_CALC_LEN (-1)

typedef sqlite3_stmt PreparedStmt;

typedef struct DbThread {
    Thread      thread;
    RingBuf*    inputQueue;
    Array       activeQueries;
    Array       activeTransactions;
} DbThread;

typedef struct Database {
    sqlite3*        sqlite;
    RingBuf*        callbackQueue;
    SimpleString*   dbPath;
    aint32_t        nextQueryId;
} Database;

struct Query;
typedef void(*QueryCB)(struct Query* query);

typedef struct Query {
    PreparedStmt*   stmt;
    Database*       db;
    
    union {
        int         state;
        int64_t     lastInsertId;
    };
    
    union {
        void*       userdata;
        int64_t     userInt;
    };
    
    QueryCB         callback;
    uint32_t        queryId;
    int             affectedRows;
    uint64_t        timestamp;
} Query;

struct Transaction;
typedef void(*TransactCB)(struct Transaction* transact);

typedef struct Transaction {
    Database*   db;
    void*       userdata;
    TransactCB  transactCallback;
    int         luaCallback;
    QueryCB     queryCallback;
} Transaction;

#endif/*STRUCTS_DB_H*/
