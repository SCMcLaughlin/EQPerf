
#ifndef DEFINE_H
#define DEFINE_H

#if defined(_WIN32) || defined(WIN32)
# define EQP_WINDOWS
#else
# define EQP_UNIX
#endif

#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>
#include <math.h>

#ifdef EQP_WINDOWS
# include <windows.h>
# include "win32_stdint.h"
#else
# include <stdint.h>
# include <stdatomic.h>
# include <errno.h>
# include <inttypes.h>
# include <unistd.h>
#endif

#ifdef EQP_WINDOWS
# define EQP_API __declspec(dllexport)
#else
# define EQP_API extern
#endif

typedef uint8_t byte;
typedef int8_t bool;

#define true 1
#define false 0

#define sizefield(type, name) sizeof(((type*)0)->name)

#endif/*DEFINE_H*/
