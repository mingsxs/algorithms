#ifndef __HASH_MAP_H__
#define __HASH_MAP_H__

#include <sys/types.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>

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


/* hash map entry */
typedef struct entry {
    void *key;              /* key */
    unsigned short size;    /* key size */
    void *value;            /* value */
    unsigned long hashcode; /* hash code for key */
    struct entry *next;     /* conflict list */
} *Entry;                   /* entry pointer */
#define newEntry() (Entry)malloc(sizeof(struct entry))
#define newEntryList(length) (Entry *)malloc(length * sizeof(Entry))

/* hash map function prototype forward declarations */
typedef struct hashMap *HashMap;
typedef struct hashMapIterator *HashMapIterator;
#define newHashMap() (HashMap)malloc(sizeof(struct hashMap))
typedef unsigned int (*HashFunc)(HashMap hashmap, void *key, size_t size, unsigned long *code);
typedef bool (*Compare)(void *key1, size_t size1, void *key2, size_t size2);
typedef void (*Put)(HashMap hashmap, void *key, size_t size, void *value);
typedef void *(*Get)(HashMap hashmap, void *key, size_t size);
typedef bool (*Remove)(HashMap hashmap, void *key, size_t size);
typedef bool (*Exists)(HashMap hashmap, void *key, size_t size);
typedef void (*Destroy)(HashMap *hashmap_p);

/* hash map structure */
typedef struct hashMap {
    unsigned int map_size;              /* key counts */
    unsigned int list_len;              /* entry list length */
    Entry *list;                        /* entry of hash map */
    unsigned int conflicts;             /* conflict count */
    bool auto_assign;                   /* dynamically reassign map */
    HashFunc hash_func;                 /* hash function */
    Compare equal;                      /* equal function for comparing keys */
    Put put;                            /* put new pair to hash map */
    Get get;                            /* get pairs by key */
    Remove remove;                      /* remove pairs by key */
    Destroy destroy;                    /* destroy hash map and release space */
    Exists exists;                      /* exists function */
    HashMapIterator iterator;           /* iterator for walk */
} *HashMap;                             /* hash map pointer */

/* create new hash map */
HashMap create_hashmap(HashFunc hash_func, Compare equal, bool auto_assign);
/* other macros */
#define PUT(hashmap, key, size, value) hashmap->put(hashmap, key, size, value)
#define GET(hashmap, key, size) hashmap->get(hashmap, key, size)
#define REMOVE(hashmap, key, size) hashmap->remove(hashmap, key, size)
#define EXISTS(hashmap, key, size) hashmap->exists(hashmap, key, size)

// Hash Map Iterator
typedef void (*NextEntry)(HashMapIterator iterator);
typedef bool (*HasNextEntry)(HashMapIterator iterator);
typedef void (*FreeIterator)(HashMapIterator *iterator_p);

/* iterator structure */
typedef struct hashMapIterator {
    HashMap hashmap;                    /* iterated hash map */
    unsigned int iterations;            /* iterations count */
    Entry current;                      /* current pair */
    int lsidx;                          /* current list index */
    NextEntry next_one;                 /* walk function to next entry */
    HasNextEntry has_next_one;          /* has next pair entry in hash map */
    FreeIterator free_iterator;         /* free iterator space */
} *HashMapIterator;                     /* hash map iterator pointer */
#define newHashMapIterator() (HashMapIterator)malloc(sizeof(struct hashMapIterator))

/* create iterator for hash map */
bool hashmap_reset_iterator(HashMap hashmap);

#endif  /* __HASH_MAP_H__ */
