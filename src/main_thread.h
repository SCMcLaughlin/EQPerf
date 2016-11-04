
#ifndef MAIN_THREAD_H
#define MAIN_THREAD_H

#include "define.h"
#include "log.h"
#include "db_thread.h"
#include "structs_main_thread.h"

int mt_global_init(void);
void mt_global_deinit(void);

#endif/*MAIN_THREAD_H*/
