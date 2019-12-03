#include <stdio.h>

#include "hash_map.h"


/* default hash map functions */
extern unsigned long string_hash(void *key, size_t ksize); // built-in hash function for strings
extern unsigned long common_hash(void *key, size_t ksize); // built-in hash function for commons

static unsigned int default_index(HashMap hashmap, void *key, size_t ksize);
static bool default_equal(void *key1, size_t ksize1, void *key2, size_t ksize2);
static void default_put(HashMap hashmap, void *key, size_t ksize, void *value);
static void *default_get(HashMap hashmap, void *key, size_t ksize);
static bool default_remove(HashMap hashmap, void *key, size_t ksize);
static bool default_exists(HashMap hashmap, void *key, size_t ksize);
static void default_resize(HashMap hashmap);
static void default_free(HashMap *hashmap_p);

/* hash map iterator functions */
static void next_entry(HashMapIterator iterator);
static bool has_next_entry(HashMapIterator iterator);
static void free_iterator(HashMapIterator *iterator_p);


/* djb2 hash function works well with strings */
unsigned long
string_hash(void *key, size_t ksize)
{
    if(key == NULL)
        return ~0;

    unsigned long hash = 5381;
    int8_t *start = key;

    while(*start)   // last byte '\0' for string is excepted
        hash = (hash << 5) + hash + *start++;

    return hash;
}

/* sdbm hash function for common scenario */
unsigned long
common_hash_func(void *key, size_t ksize)
{
    if(key == NULL)
        return ~0;

    unsigned long hash = 0;
    int8_t *start = key;

    while(ksize--)
        hash = *start++ + (hash << 6) + (hash << 16) - hash;

    return hash;
}

/* default function for indexing keys in hash map */
static unsigned int
default_index(HashMap hashmap, void *key, size_t ksize)
{
    if(!(hashmap && key))
        return (unsigned int) ~0;

    unsigned long hash = hashmap->hashfunc(key, ksize);

    return (unsigned int) hash % hashmap->list_size;
}

/* default function for checking keys equal */
static bool
default_equal(void *key1, size_t ksize1, void *key2, size_t ksize2)
{
    if(!(key1 && key2) || ksize1!=ksize2)
        return false;

    int8_t *p = key1, *q = key2;
    while(ksize1>0 && *p++==*q++) ksize1--;

    return ksize1==0 ? true : false;
}

/* default function for placing new entry */
static void
default_put(HashMap hashmap, void *key, size_t ksize, void *value)
{
    if(!(hashmap && key && value))
        return ;

    /* check load and capacity for resizing */
    if(hashmap->map_size>=hashmap->list_size)
        hashmap->resize(hashmap);

    Entry entry = newEntry();
    entry->key = key;
    entry->ksize = ksize;
    entry->value = value;
    entry->hashcode = hashmap->hashfunc(key, ksize);
    entry->next = NULL;

    unsigned int lsidx = entry->hashcode % hashmap->list_size;

    if(hashmap->list[lsidx] == NULL) {
        hashmap->list[lsidx] = entry;
    } else {
        Entry *indirect;
        for(indirect=&hashmap->list[lsidx]; *indirect; indirect=&(*indirect)->next)
            if(entry->hashcode == (*indirect)->hashcode) { // key already exists, update value
                (*indirect)->value = value;
                free(entry);
                return ;
            }
        hashmap->conflicts ++;
        *indirect = entry;
    }

    hashmap->map_size ++;
}

/* default function for fetching entry by key */
static void *
default_get(HashMap hashmap, void *key, size_t ksize)
{
    if(!(hashmap && key))
        return NULL;

    unsigned int lsidx = hashmap->index(hashmap, key, ksize);
    Entry walk = hashmap->list[lsidx];
    while(walk && !hashmap->equal(key, ksize, walk->key, walk->ksize))
        walk = walk->next;

    return (void *)(walk!=NULL? walk->value:NULL);
}

/* default function for removing entry from hash map */
static bool
default_remove(HashMap hashmap, void *key, size_t ksize)
{
    if(!(hashmap && key))
        return false;
    if(hashmap->map_size<(hashmap->list_size>>1) && hashmap->list_size>HASHMAP_INIT_CAPACITY)
        hashmap->resize(hashmap);

    unsigned int lsidx = hashmap->index(hashmap, key, ksize);
    /* This is inspired by Linus from TED's course, to remove one item from linkedlist */
    // The "indirect" pointer points to the *address* of the entry
    // which points to the item in linkedlist that we'll remove
    Entry remove, *indirect = &hashmap->list[lsidx];
    // Walk the list, looking for the thing that points to the entry we want to remove
    while(*indirect && !hashmap->equal(key, ksize, (*indirect)->key, (*indirect)->ksize))
        indirect = &(*indirect)->next;      // points to address of the next pointer of current entry, eg, walk to next one
    // If the entry is found
    if(*indirect) {
        // just remove it, by rewriting the next pointer of current entry
        remove = *(indirect);
        *indirect = (*indirect)->next;
        free(remove);
        if(hashmap->list[lsidx])
            hashmap->conflicts --;
        hashmap->map_size --;
        return true;
    }

    return false;
}


/* default function for resizing hash map to double/half size, for 2^n hash map capacity */
static void
default_resize(HashMap hashmap)
{
    /*
     * 2 tricks here need to be demonstrated.
     *  1. since we store hashcode for each entry, so we don't have to recaculate hashcode when
     *     walking through lists during doing resize.
     *  2. since list_size here is always 2^n(1 << n), so we don't have to do modulus for the
     *     hashcodes to get new inidce for entries, just compare the mask bit with hashcodes
     *     then we can know if entry needs to be moved to new place for both enlarging size and
     *     shrinking size. Like,
     *          hashcode = 0x65301de661e, old_size = 2^5, old_indice = hashcode & 0x1f = 0x1e
     *          we use last 5 bits from low bit side, now new_size = 2^6, we just need to check
     *          the 6th bit from low bit side, if it's 0, then modulus(hashcode & 3f) is still
     *          0x1e, lsit indice doesn't change, if it's 1, modulus changes to old_indice + 2^5
     *
     * this idea comes from Java.
     */
    unsigned int new_size, lsidx_mask;
    bool enlarge;
    if(hashmap->map_size >= hashmap->list_size && hashmap->list_size < HASHMAP_MAX_CAPACITY) {  // enlarge
        lsidx_mask = hashmap->list_size;
        new_size = hashmap->list_size << 1;
        enlarge = true;
    } else if(hashmap->map_size < (hashmap->list_size >> 1) && hashmap->list_size > HASHMAP_INIT_CAPACITY) {  // shrink
        lsidx_mask = hashmap->list_size >> 1;
        new_size = lsidx_mask;
        enlarge = false;
    } else {
        return ;  // no resize needed, return
    }

    /* malloc space */
    Entry *new_list, *indirect, move_entry;
    if(enlarge) { // enlarge
        // realloc will ensure all data is copied to new space if the ptr is changed to new ptr
        // we don't need to care about restore
        new_list = (Entry *)realloc(hashmap->list, new_size*sizeof(Entry));
        MEMZERO(&new_list[hashmap->list_size], hashmap->list_size*sizeof(Entry));
        hashmap->list = new_list;
    } else { // shrink
        new_list = (Entry *)malloc(new_size*sizeof(Entry));
        MEMCPY(new_list, hashmap->list, (hashmap->list_size >> 1)*sizeof(Entry));   // copy first half of the list which surely won't be moved during transfer
    }

    /* do entry transfer */
    unsigned int new_lsidx;
    for(unsigned int lsidx = (enlarge ? 0 : hashmap->list_size >> 1); lsidx < hashmap->list_size; lsidx++) {
        indirect = &hashmap->list[lsidx];  // walk current bucket

        while(*indirect) {
            if(enlarge && ((*indirect)->hashcode & lsidx_mask)==0) {    // no need to move current entry
                indirect = &(*indirect)->next;  // move to next entry by pointing to the next pointer of current entry
            } else {  // need to move current entry
                new_lsidx = enlarge ? (lsidx + lsidx_mask) : (lsidx - lsidx_mask);   // caculate new indice for current entry, the method is different between enlarging and shrinking.
                /* remove this entry from old bucket with `indirect` Linux way */
                move_entry = *indirect;         // restore current entry
                *indirect = (*indirect)->next;  // just remove current entry
                if(hashmap->list[lsidx] != NULL)
                    hashmap->conflicts --;      // if this entry is removed from a conflicts linked-list, it was a conflict before being removed.

                /* insert this entry to new bucket by inserting to head position */
                if(new_list[new_lsidx] != NULL)
                    hashmap->conflicts ++;      // if this entry is moved to a place that's not NULL, it is a new conflict after being transferred.
                move_entry->next = new_list[new_lsidx];
                new_list[new_lsidx] = move_entry;
            }
        }
    }

    /* update hash map */
    if(!enlarge) {
        free(hashmap->list);
        hashmap->list = new_list;
    }
    hashmap->list_size = new_size;
}

/* default function for releasing allocated space */
static void
default_free(HashMap *hashmap_p)
{
    if(!(hashmap_p && *hashmap_p))
        return ;

    HashMap hashmap = *hashmap_p;
    int increase = 0;
    Entry current = NULL, next;

    while(hashmap->map_size > 0 && increase <= hashmap->list_size)
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

/* default function for checking entry existence */
static bool
default_exists(HashMap hashmap, void *key, size_t ksize)
{
    if(!(hashmap && key))
        return false;

    unsigned int lsidx = hashmap->index(hashmap, key, ksize);
    Entry walk = hashmap->list[lsidx];

    while(walk && !hashmap->equal(key, ksize, walk->key, walk->ksize))
        walk = walk->next;

    return walk? true:false;
}

/* default function for fetching next entry iterator */
static void
next_entry(HashMapIterator iterator)
{
    if(!iterator)
        return ;

    if(iterator->current->next == NULL) {
        unsigned int lsidx = iterator->lsidx;
        while(++lsidx < iterator->hashmap->list_size && iterator->hashmap->list[lsidx] == NULL) ;
        // there is no next entry, reset iterator
        if(lsidx == iterator->hashmap->list_size) {
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

/* default function for checking if exists next entry in hash map */
static bool
has_next_entry(HashMapIterator iterator)
{
    if(!iterator)
        return false;

    if(iterator->current->next == NULL) {
        unsigned int lsidx = iterator->lsidx;
        while(++lsidx < iterator->hashmap->list_size && iterator->hashmap->list[lsidx] == NULL) ;
        if(lsidx == iterator->hashmap->list_size)
            return false;
    }
    return true;
}

/* default function for releasing allocated space for iterator */
static void
free_iterator(HashMapIterator *iterator_p)
{
    if(!(iterator_p && *iterator_p))
        return ;

    MEMZERO(*iterator_p, sizeof(struct hashMapIterator));
    free(*iterator_p);
}

/* create and initialize new hash map */
HashMap
create_hashmap(HashFunc hashfunc, Compare equal)
{
    // malloc space for hash map
    HashMap hashmap = newHashMap();
    // initialize first entry list with size 4(2^2)
    Entry *list = newEntryList(HASHMAP_INIT_CAPACITY);
    MEMZERO(list, HASHMAP_INIT_CAPACITY*sizeof(Entry));

    //initialize hash map
    hashmap->map_size = 0;
    hashmap->conflicts = 0;
    hashmap->list_size = HASHMAP_INIT_CAPACITY;
    hashmap->list = list;
    // bind hash map functions
    hashmap->hashfunc = hashfunc==NULL ? string_hash : hashfunc;
    hashmap->equal = equal==NULL ? default_equal : equal;
    hashmap->put = default_put;
    hashmap->get = default_get;
    hashmap->remove = default_remove;
    hashmap->resize = default_resize;
    hashmap->exists = default_exists;
    hashmap->free = default_free;
    hashmap->index = default_index;
    hashmap->iterator = NULL;

    return hashmap;
}

/* reset current hash map iterator to point to first entry */
bool
hashmap_reset_iterator(HashMap hashmap)
{
    if(!hashmap)
        return false;

    if(hashmap->iterator == NULL) 
        hashmap->iterator = newHashMapIterator();

    HashMapIterator iterator = hashmap->iterator;

    iterator->iterations = 1;
    iterator->lsidx = 0;
    iterator->hashmap = hashmap;

    //walk to first entry in hash map list
    while(hashmap->list[iterator->lsidx] == NULL) iterator->lsidx++;
    iterator->current = hashmap->list[iterator->lsidx];

    iterator->next_one = next_entry;
    iterator->has_next_one = has_next_entry;
    iterator->free_iterator = free_iterator;

    hashmap->iterator = iterator;

    return true;
}
