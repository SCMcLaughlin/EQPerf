
#ifndef DB_TRANSACTION_H
#define DB_TRANSACTION_H

#include "define.h"
#include "log.h"
#include "eqp_alloc.h"
#include "structs_db.h"

Transaction* transact_create(Database* db, TransactCB transactCallback, QueryCB queryCallback, void* userdata);
void transact_destroy(Transaction* trans);

void transact_exec_callback(Transaction* trans);

#define transact_id(trans) ((trans)->transactId)
#define transact_db(trans) ((trans)->db)
#define transact_userdata(trans) ((trans)->userdata)
#define transact_userdata_type(trans, type) (type*)transact_userdata((trans))

#endif/*DB_TRANSACTION_H*/
