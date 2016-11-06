
#include "db_transaction.h"
#include "db.h"

Transaction* transact_create(Database* db, TransactCB transactCallback, QueryCB queryCallback, void* userdata)
{
    Transaction* trans = alloc_type(Transaction);
    
    if (!trans)
        return NULL;
    
    trans->db               = db;
    trans->userdata         = userdata;
    trans->transactCallback = transactCallback;
    trans->queryCallback    = queryCallback;
    trans->transactId       = db_next_transact_id(db);
    perf_init(&trans->perfTimer);
    
    db_grab(db);
    
    return trans;
}

void transact_destroy(Transaction* trans)
{
    Database* db = trans->db;
    
    if (db)
        db_drop(db);
    
    free(trans);
}

void transact_exec_callback(Transaction* trans)
{
    TransactCB callback = trans->transactCallback;
    
    if (callback)
        callback(trans);
}
