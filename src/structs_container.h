
#ifndef STRUCTS_CONTAINER_H
#define STRUCTS_CONTAINER_H

#include "define.h"

typedef void(*ArrayCallback)(void* elem);

typedef struct Array {
    uint32_t    count;
    uint32_t    capacity;
    uint32_t    elemSize;
    byte*       data;
} Array;

typedef struct SimpleString SimpleString;

typedef struct String {
    uint32_t    length;
    uint32_t    capacity;
    char*       data;
} String;

typedef void(*HashTblCallback)(void* elem);

typedef struct HashTblEnt {
    union {
        SimpleString*   keyStr; /* The hash table makes private copies of all keys */
        int64_t         keyInt;
    };
    uint32_t    hash;
    uint32_t    next;
    byte        data[0];
} HashTblEnt;

typedef struct HashTbl {
    uint32_t    capacity;
    uint32_t    elemSize;
    uint32_t    freeIndex;
    uint32_t    entSize;    /* Make sure each HashTblEnt will be a multiple of 8 bytes */
    byte*       data;
} HashTbl;

#endif/*STRUCTS_CONTAINER_H*/
