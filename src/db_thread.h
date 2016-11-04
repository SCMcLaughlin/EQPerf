
#ifndef DB_THREAD_H
#define DB_THREAD_H

#include "define.h"
#include "log.h"
#include "structs_db.h"

int db_thread_init(void);
void db_thread_deinit(void);

int db_thread_sched(void* ptr, int isTransaction);

#endif/*DB_THREAD_H*/
