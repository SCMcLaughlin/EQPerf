
#include "eqp_atomic.h"
#include "eqp_semaphore.h"
#include "ringbuf.h"
#include "eqp_tls.h"
#include "eqp_thread.h"
#include "eqp_clock.h"
#include "container.h"
#include "log.h"
#include "main_thread.h"
#include "database.h"

static void test_cb(Query* query)
{
    uint32_t i = 0;
    Row* row;
    
    while ( (row = query_select(query)) )
    {
        printf("row %i: %i %i %i\n", i++, row_int(row, 0), row_int(row, 1), row_int(row, 2));
    }
}

static void test_trans(Transaction* trans)
{
    Database* db = transact_db(trans);
    PreparedStmt* stmt;
    
    stmt = db_prep_literal(db, "INSERT INTO x (b) VALUES (99), (66), (33), (11), (19548)");
    if (stmt)
        stmt_exec_transaction(stmt);
}

static void test_commit(Query* query)
{
    printf("COMMIT: %i, %ld\n", query_affected_rows(query), query_last_insert_id(query));
}

int main()
{
    PerfTimer pt;
    aint32_t a;
    
    perf_init(&pt);
    
    if (mt_global_init())
    {
        fputs("Initialization failed, aborting\n", stderr);
        return 1;
    }
    
    log_msg(Log_Info, "Test log message!");
    
    aint32_set(&a, 5);
    
    printf("hello: %i\n", aint32_get(&a));
    
    Semaphore sem;
    semaphore_init(&sem);
    
    semaphore_trigger(&sem);
    if (!semaphore_try_wait(&sem))
        printf("semaphore worked\n");
    
    HashTbl tbl;
    
    tbl_init(&tbl, int);
    
    int x = 99;
    tbl_set_int(&tbl, 7, &x);
    x = 66;
    tbl_set_str(&tbl, "hi there", 0, &x);
    
    x = *tbl_get_str(&tbl, "hi there", 0, int);
    printf("hash tbl: %i, ", x);
    x = *tbl_get_int(&tbl, 7, int);
    printf("%i\n", x);
    
    tbl_deinit(&tbl, NULL);
    
    
    Database* db = db_create();
    db_open(db, ":memory:", NULL);
    
    PreparedStmt* stmt = db_prep(db, "CREATE TABLE x(a, b, c)", STMT_CALC_LEN);
    db_sched(db, stmt, NULL);
    clock_sleep(500);
    stmt = db_prep(db, "INSERT INTO x VALUES (1, 2, 3), (99, 0, 97), (47, NULL, 7)", STMT_CALC_LEN);
    db_sched(db, stmt, NULL);
    clock_sleep(750);
    db_exec_callbacks(db);
    
    db_sched_transact(db, test_trans, test_commit);
    
    stmt = db_prep_literal(db, "SELECT a, b, c FROM x");
    db_sched(db, stmt, test_cb);
    
    clock_sleep(1000);
    db_exec_callbacks(db);
    
    db_drop(db);
    

    printf("total time: %lu\n", perf_microseconds(&pt));
    
    clock_sleep(1000);
    mt_global_deinit();
}
