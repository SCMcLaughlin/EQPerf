
#ifndef EQP_ALLOC_H
#define EQP_ALLOC_H

#include "define.h"

#define realloc_bytes(ptr, n) (byte*)realloc((ptr), (n))
#define realloc_type(ptr, type) (type*)realloc((ptr), sizeof(type))
#define realloc_bytes_type(ptr, n, type) (type*)realloc((ptr), (n))
#define realloc_array_type(ptr, n, type) (type*)realloc((ptr), sizeof(type) * (n))

#define alloc_bytes(n) (byte*)realloc(NULL, (n))
#define alloc_type(type) (type*)realloc(NULL, sizeof(type))
#define alloc_bytes_type(n, type) (type*)realloc(NULL, (n))
#define alloc_array_type(n, type) (type*)realloc(NULL, sizeof(type) * (n))

#endif/*EQP_ALLOC_H*/
