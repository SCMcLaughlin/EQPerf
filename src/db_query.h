
#ifndef DB_QUERY_H
#define DB_QUERY_H

#include "define.h"
#include "log.h"
#include "eqp_clock.h"
#include "structs_db.h"

Query* query_create(Database* db, PreparedStmt* stmt, QueryCB callback, void* userdata);
void query_destroy(Query* query);

int query_exec_background(Query* query);
Row* query_select(Query* query);
void query_reset(Query* query);
int query_queue_callback(Query* query);
void query_exec_callback(Query* query);

#define query_id(q) ((q)->queryId)
const char* query_db_path(Query* query);

#endif/*DB_QUERY_H*/
