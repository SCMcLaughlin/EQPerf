
CC= clang

CFLAGS= 
COPT= -O2 -fomit-frame-pointer -ffast-math -std=gnu11
CWARN= -Wall -Wextra -Wredundant-decls
CWARNIGNORE= -Wno-unused-result -Wno-strict-aliasing
CINCLUDE= -Isrc/
CDEF=

#ifdef debug
CFLAGS+= -O0 -g -Wno-format -fno-omit-frame-pointer
CDEF+= -DEQP_DEBUG -DDEBUG
#else
#CFLAGS+= -DNDEBUG
#endif

##############################################################################
# eqp-server
##############################################################################
_EQP_SERVER_OBJECTS=    \
 db                     \
 db_thread              \
 eqp_array              \
 eqp_atomic_posix       \
 eqp_clock_posix        \
 eqp_hash_tbl           \
 eqp_semaphore_posix    \
 eqp_server_main        \
 eqp_string             \
 eqp_thread_common      \
 eqp_thread_posix       \
 eqp_tls_posix          \
 hash                   \
 log                    \
 main_thread            \
 ringbuf

EQP_SERVER_OBJECTS= $(patsubst %,build/%.o,$(_EQP_SERVER_OBJECTS))

##############################################################################
# Core Linker flags
##############################################################################
LFLAGS= -rdynamic
#LDYNAMIC= -pthread -lrt -lsqlite3 -lz -lm -ldl
LDYNAMIC= -pthread -lrt -lsqlite3
LIBLUAJIT= -lluajit-5.1

#ifdef debug
#LSTATIC= $(DIRBIN)libluajit-valgrind.a
LDYNCORE= $(LDYNAMIC)
#else
LSTATIC= 
#LDYNCORE= -lluajit-5.1 $(LDYNAMIC)
#endif

##############################################################################
# Util
##############################################################################
Q= @
E= @echo
RM= rm -f 

##############################################################################
# Build rules
##############################################################################
.PHONY: default all clean

default all: eqp-server

eqp-server: bin/eqp-server

bin/eqp-server: $(EQP_SERVER_OBJECTS)
	$(E) "Linking $@"
	$(Q)$(CC) -o $@ $^ $(LSTATIC) $(LDYNCORE) $(LFLAGS)

build/%.o: src/%.c $($(CC) -M src/%.c)
	$(E) "\033[0;32mCC     $@\033[0m"
	$(Q)$(CC) -c -o $@ $< $(CDEF) $(CWARN) $(CWARNIGNORE) $(CFLAGS) $(CINCLUDE)

clean:
	$(Q)$(RM) build/*.o
	$(E) "Cleaned build directory"
