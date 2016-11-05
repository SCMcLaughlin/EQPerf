
#ifndef DB_H
#define DB_H

#include "define.h"
#include "log.h"
#include "eqp_alloc.h"
#include "structs_db.h"

Database* db_create(void);
void db_grab(Database* db);
void db_drop(Database* db);

int db_init(Database* db);
void db_deinit(Database* db);

int db_open(Database* db, const char* dbPath, const char* schemaPath);

int db_exec(Database* db, const char* sql);

PreparedStmt* db_prep(Database* db, const char* sql, int len);
#define db_prep_cstr(db, sql) db_prep((db), (sql), STMT_CALC_LEN)
#define db_prep_literal(db, sql) db_prep((db), (sql), sizeof(sql) - 1)

int db_sched_ud(Database* db, PreparedStmt* stmt, QueryCB callback, void* userdata);
#define db_sched(db, stmt, cb) db_sched_ud((db), (stmt), (cb), NULL)
int db_sched_transact_ud(Database* db, TransactCB transCB, QueryCB queryCB, void* userdata);
#define db_sched_transact(db, tcb, qcb) db_sched_transact_ud((db), (tcb), (qcb), NULL)

int db_next_query_id(Database* db);
const char* db_path(Database* db);

int db_queue_callback(Database* db, RingPacket* rp);
void db_exec_callbacks(Database* db);

#endif/*DB_H*/
