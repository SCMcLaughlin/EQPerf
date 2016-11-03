
#include "main_thread.h"

static void mt_global_init_err(const char* str, int rc)
{
    fprintf(stderr, "ERROR: [mt_global_init] %s, err code: %i\n", str, rc);
}

int mt_global_init(void)
{
    int rc = ERR_None;
    
#define check(err) if (rc) do { mt_global_init_err((err), rc); goto fail; } while(0)
    
    rc = tls_global_init_keys();
    check("tls_global_init_keys() failed");
    
    rc = tls_init_thread(EQPID_MainThread);
    check("tls_init_thread() failed");
    
    rc = log_init();
    check("log_init() failed");
    
    rc = log_register(EQPID_MainThread);
    check("log_register() failed");
    
#undef check
    
    return ERR_None;
    
fail:
    return rc;
}

void mt_global_deinit(void)
{
    log_deinit();
    tls_global_deinit_keys();
}
