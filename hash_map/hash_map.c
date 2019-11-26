#include <stdio.h>

#include "hash_map.h"

#define ENLARGE_SIZE 8

/* default hash map functions */
extern unsigned int string_hash_func(HashMap hashmap, void *key, size_t size, unsigned long *code); // built-in hash function for strings
extern unsigned int common_hash_func(HashMap hashmap, void *key, size_t size, unsigned long *code); // built-in hash function for commons
static bool default_equal(void *key1, size_t size1, void *key2, size_t size2);
static void default_put(HashMap hashmap, void *key, size_t size, void *value);
static void *default_get(HashMap hashmap, void *key, size_t size);
static bool default_remove(HashMap hashmap, void *key, size_t size);
static bool default_exists(HashMap hashmap, void *key, size_t size);
static void default_destroy(HashMap *hashmap_p);

/* hash map iterator functions */
static void next_entry(HashMapIterator iterator);
static bool has_next_entry(HashMapIterator iterator);
static void free_iterator(HashMapIterator *iterator_p);

/* djb2 hash function works well with strings */
unsigned int
string_hash_func(HashMap hashmap, void *key, size_t size, unsigned long *code)
{
    if(!(hashmap && key))
        return ~0;

    unsigned long hash = 5381;
    int8_t *start = key;

    while(*start)   // last byte '\0' for string is excepted
        hash = (hash << 5) + hash + *start++;

    if(code != NULL)
        *code = hash;

    return hash % hashmap->list_len;
}

/* sdbm hash function for common scenario */
unsigned int
common_hash_func(HashMap hashmap, void *key, size_t size, unsigned long *code)
{
    if(!(hashmap && key))
        return ~0;

    unsigned long hash = 0;
    int8_t *start = key;

    while(size--)
        hash = *start++ + (hash << 6) + (hash << 16) - hash;

    if(code != NULL)
        *code = hash;

    return hash % hashmap->list_len;
}

static bool
default_equal(void *key1, size_t size1, void *key2, size_t size2)
{
    if(!(key1 && key2) || size1!=size2)
        return false;

    int8_t *p = key1, *q = key2;
    while(size1>0 && *p++==*q++) size1--;

    return size1==0 ? true : false;
}

static void
default_put(HashMap hashmap, void *key, size_t size, void *value)
{
    if(!(hashmap && key && value))
        return ;

    if(hashmap->map_size>=hashmap->list_len && !hashmap->auto_assign) {
        printf("Error: Full Hash Map.\n");
        return ;
    }
    /* enlarge space */
    if(hashmap->map_size >= hashmap->list_len) {
        // realloc will ensure all data is copied to new space if the ptr is changed to new ptr
        // we don't need to care about restore
        Entry *list = (Entry *)realloc(hashmap->list, (hashmap->list_len+ENLARGE_SIZE) * sizeof(Entry));
        MEMZERO(&list[hashmap->list_len], ENLARGE_SIZE * sizeof(Entry));
        hashmap->list = list;
        hashmap->list_len += ENLARGE_SIZE;
    }

    unsigned long hashcode;
    unsigned int lsidx = hashmap->hash_func(hashmap, key, size, &hashcode);
    Entry entry = newEntry();
    entry->key = key;
    entry->size = size;
    entry->value = value;
    entry->hashcode = hashcode;
    entry->next = NULL;

    if(hashmap->list[lsidx] == NULL) {
        hashmap->list[lsidx] = entry;
    } else {
        hashmap->conflicts ++;
        Entry walk = hashmap->list[lsidx];
        while(walk->next) walk = walk->next;
        walk->next = entry;
    }

    hashmap->map_size ++;
}

static void *
default_get(HashMap hashmap, void *key, size_t size)
{
    if(!(hashmap && key))
        return NULL;

    unsigned int lsidx = hashmap->hash_func(hashmap, key, size, NULL);
    Entry walk = hashmap->list[lsidx];
    while(walk && !hashmap->equal(key, size, walk->key, walk->size))
        walk = walk->next;

    return (void *)(walk!=NULL? walk->value:NULL);
}

static bool
default_remove(HashMap hashmap, void *key, size_t size)
{
    if(!(hashmap && key))
        return false;

    unsigned int lsidx = hashmap->hash_func(hashmap, key, size, NULL);
    /* This is inspired by Linus from TED's course, to remove one item from linkedlist */
    // The "indirect" pointer points to the *address* of the entry
    // which points to the item in linkedlist that we'll remove
    Entry *indirect = &hashmap->list[lsidx];
    // Walk the list, looking for the thing that points to the entry we want to remove
    while(*indirect && !hashmap->equal(key, size, (*indirect)->key, (*indirect)->size))
        indirect = &(*indirect)->next;
    // If the entry is found
    if(*indirect) {
        if(*indirect != hashmap->list[lsidx])
            hashmap->conflicts --;
        free(*indirect);
        hashmap->map_size --;
        // just remove it
        *indirect = (*indirect)->next;

        return true;
    }

    return false;
}

static void
default_destroy(HashMap *hashmap_p)
{
    if(!(hashmap_p && *hashmap_p))
        return ;

    HashMap hashmap = *hashmap_p;
    int increase = 0;
    Entry current = NULL, next;

    while(hashmap->map_size > 0 && increase < hashmap->list_len)
        if(current) {
            next = current->next;
            free(current);
            hashmap->map_size --;
            current = next;
        } else {
            current = hashmap->list[increase++];
        }
    free(hashmap->list);

    if(hashmap->iterator)
        hashmap->iterator->free_iterator(&hashmap->iterator);

    free(*hashmap_p);
}

static bool
default_exists(HashMap hashmap, void *key, size_t size)
{
    if(!(hashmap && key))
        return false;

    unsigned int lsidx = hashmap->hash_func(hashmap, key, size, NULL);
    Entry walk = hashmap->list[lsidx];

    while(walk && !hashmap->equal(key, size, walk->key, walk->size))
        walk = walk->next;

    return walk? true:false;
}

static void
next_entry(HashMapIterator iterator)
{
    if(!iterator)
        return ;

    if(iterator->current->next == NULL) {
        unsigned int lsidx = iterator->lsidx;
        while(++lsidx < iterator->hashmap->list_len && iterator->hashmap->list[lsidx] == NULL) ;
        // there is no next entry, reset iterator
        if(lsidx == iterator->hashmap->list_len) {
            hashmap_reset_iterator(iterator->hashmap);
            return ;
        }

        iterator->lsidx = lsidx;
        iterator->current = iterator->hashmap->list[lsidx];
    } else {
        iterator->current = iterator->current->next;
    }
    iterator->iterations ++;
}

static bool
has_next_entry(HashMapIterator iterator)
{
    if(!iterator)
        return false;

    if(iterator->current->next == NULL) {
        unsigned int lsidx = iterator->lsidx;
        while(++lsidx < iterator->hashmap->list_len && iterator->hashmap->list[lsidx] == NULL) ;
        if(lsidx == iterator->hashmap->list_len)
            return false;
    }
    return true;
}

static void
free_iterator(HashMapIterator *iterator_p)
{
    if(!(iterator_p && *iterator_p))
        return ;

    MEMZERO(*iterator_p, sizeof(struct hashMapIterator));
    free(*iterator_p);
}

// create new hash map
HashMap
create_hashmap(HashFunc hash_func, Compare equal, bool auto_assign)
{
    // allocate memory for new hash map
    HashMap hashmap = newHashMap();
    // initialize memory with size 8 for first entry list
    Entry *list = newEntryList(ENLARGE_SIZE);
    MEMZERO(list, ENLARGE_SIZE * sizeof(Entry));

    //initialize hash map
    hashmap->map_size = 0;
    hashmap->conflicts = 0;
    hashmap->list_len = ENLARGE_SIZE;
    hashmap->list = list;
    hashmap->auto_assign = auto_assign;
    // bind hash map functions
    hashmap->hash_func = hash_func==NULL ? string_hash_func: hash_func;
    hashmap->equal = equal==NULL ? default_equal : equal;
    hashmap->put = default_put;
    hashmap->get = default_get;
    hashmap->remove = default_remove;
    hashmap->exists = default_exists;
    hashmap->destroy = default_destroy;
    hashmap->iterator = NULL;

    return hashmap;
}

// reset iterator for current hash map
bool
hashmap_reset_iterator(HashMap hashmap)
{
    if(!hashmap)
        return false;

    if(hashmap->iterator)
        hashmap->iterator->free_iterator(&hashmap->iterator);

    HashMapIterator iterator = newHashMapIterator();
    iterator->iterations = 1;
    iterator->lsidx = -1;
    iterator->hashmap = hashmap;

    //walk to first entry in hash map list
    while(hashmap->list[++iterator->lsidx] == NULL) ;
    iterator->current = hashmap->list[iterator->lsidx];

    iterator->next_one = next_entry;
    iterator->has_next_one = has_next_entry;
    iterator->free_iterator = free_iterator;

    hashmap->iterator = iterator;

    return true;
}
