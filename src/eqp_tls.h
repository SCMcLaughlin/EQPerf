
#ifndef TLS_H
#define TLS_H

#include "define.h"

int tls_global_init_keys(void);
void tls_global_deinit_keys(void);
int tls_init_thread(int srcId);
void* tls_get_ptr(int key);
int tls_get_int(int key, int* out);

enum TlsKey
{
    TlsKey_Id
};

#endif/*TLS_H*/
