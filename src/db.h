
#ifndef DB_H
#define DB_H

#include "define.h"
#include "eqp_alloc.h"
#include "structs_db.h"

EQP_API Database* db_create(void);
EQP_API void db_destroy(Database* db);

void db_init(Database* db);
void db_deinit(Database* db);

EQP_API int db_open(Database* db, const char* dbPath, const char* schemaPath);

int db_exec(Database* db, const char* sql);

EQP_API PreparedStmt* db_prep(Database* db, const char* sql, int len);
#define db_prep_cstr(db, sql) db_prep((db), (sql), -1)
#define db_prep_literal(db, sql) db_prep((db), (sql), sizeof(sql) - 1)

int db_sched_ud(Database* db, PreparedStmt* stmt, QueryCB callback, void* userdata);
#define db_sched(db, stmt, cb) db_sched_ud((db), (stmt), (db), NULL)
int db_sched_transact_ud(Database* db, TransactCB transCB, QueryCB queryCB, void* userdata);
#define db_sched_transact(db, tcb, qcb) db_sched_transact_ud((db), (tcb), (qcb), NULL)

#endif/*DB_H*/
