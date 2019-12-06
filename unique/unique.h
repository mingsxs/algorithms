#ifndef __UNIQUE_H__
#define __UNIQUE_H__

#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>
#include <ctype.h>
#include <string.h>
#include <stdio.h>

/* macros memzero */
#define MEMZERO(__dest, __size) do { \
unsigned int i; char *start; \
for(i=0, start=(char *)(__dest); i<(__size); i++) \
    *(start + i) = 0x0; } while(0)
/* macros memcpy */
#define MEMCPY(__dest, __src, __size) do { \
unsigned int i; char *start; \
for(i=0, start=(char *)(__dest); i<(__size); i++) \
    *(start + i) = *((char *)(__src) + i); } while(0)


typedef struct uniqList *UniqList;

typedef void (*Feed)(UniqList list);
typedef void (*Unique)(UniqList list);
typedef void (*Destroy)(UniqList list);
typedef void (*WriteBack)(UniqList list);
typedef void (*Flush)(UniqList list);

/* unique structure */
typedef struct uniqList {
    unsigned int words;
    char **list;
    unsigned int list_len;
    unsigned int removed;
    void *mapblock;
    unsigned long blocksize;
    Feed feedtxt;
    Unique unique_func;
    Flush flushspace;
    Destroy destroy;
    WriteBack writebacktxt;
} *UniqList;
#define newUniqStruct() (UniqList)malloc(sizeof(struct uniqList))
#define newWordList(length) (char **)malloc(length * sizeof(char *))

UniqList create_uniq_list(const char *path);
#endif /* __UNIQUE_H__ */
