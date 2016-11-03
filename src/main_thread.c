
#include "main_thread.h"

static void mt_global_init_err(const char* str, int rc)
{
    fprintf(stderr, "ERROR: [mt_global_init] %s, err code: %i\n", str, rc);
}

int mt_global_init(void)
{
    int rc = ERR_None;
    
#define check(err) if (rc) do { mt_global_init_err((err), rc); goto fail; } while(0)
    
    /* Init the thread local storage system, and set the id for the main thread */
    rc = tls_global_init_keys();
    check("tls_global_init_keys() failed");
    
    rc = tls_init_thread(EQPID_MainThread);
    check("tls_init_thread() failed");
    
    /* Start the logging thread, and open a log for the main thread */
    rc = log_init();
    check("log_init() failed");
    
    rc = log_register(EQPID_MainThread);
    check("log_register() failed");
    
#undef check
    
    log_msg(Log_Init, "[%s] Essential systems initialized", FUNC);
    
    return ERR_None;
    
fail:
    return rc;
}

void mt_global_deinit(void)
{
    log_msg(Log_Init, "[%s] Shutting down essential systems", FUNC);
    log_deinit();
    tls_global_deinit_keys();
}
