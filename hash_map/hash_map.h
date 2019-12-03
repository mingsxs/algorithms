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


/* hash map capacity const*/
#define HASHMAP_MAX_CAPACITY (1 << 16)      // maximum
#define HASHMAP_INIT_CAPACITY (1 << 2)      // initialize

/* hash map entry */
typedef struct entry {
    void *key;              /* key */
    unsigned short ksize;   /* key size */
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
typedef unsigned long (*HashFunc)(void *key, size_t ksize);
typedef unsigned int (*Index)(HashMap hashmap, void *key, size_t ksize);
typedef void (*Resize)(HashMap hashmap);
typedef bool (*Compare)(void *key1, size_t ksize1, void *key2, size_t ksize2);
typedef void (*Put)(HashMap hashmap, void *key, size_t ksize, void *value);
typedef void *(*Get)(HashMap hashmap, void *key, size_t ksize);
typedef bool (*Remove)(HashMap hashmap, void *key, size_t ksize);
typedef bool (*Exists)(HashMap hashmap, void *key, size_t ksize);
typedef void (*Free)(HashMap *hashmap_p);

/* hash map structure */
typedef struct hashMap {
    unsigned int map_size;              /* key counts */
    unsigned int list_size;             /* entry list length */
    unsigned int conflicts;             /* conflict count */
    Entry *list;                        /* entry of hash map */
    HashFunc hashfunc;                  /* hash function */
    Index index;                        /* index keys in hash map */
    Compare equal;                      /* equal function for comparing keys */
    Put put;                            /* put new pair to hash map */
    Get get;                            /* get pairs by key */
    Remove remove;                      /* remove pairs by key */
    Resize resize;                      /* resize hash map */
    Exists exists;                      /* exists function */
    Free free;                          /* free hash map and release space */
    HashMapIterator iterator;           /* iterator for walk */
} *HashMap;                             /* hash map pointer */

/* create new hash map */
HashMap create_hashmap(HashFunc hash_func, Compare equal);
/* other macros */
#define PUT(hashmap, key, ksize, value) hashmap->put(hashmap, key, ksize, value)
#define GET(hashmap, key, ksize) hashmap->get(hashmap, key, ksize)
#define REMOVE(hashmap, key, ksize) hashmap->remove(hashmap, key, ksize)
#define EXISTS(hashmap, key, ksize) hashmap->exists(hashmap, key, ksize)

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
