#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "hash_map.h"


typedef struct pair {
    void *key;
    void *value;
} pair_s;


int main(int argc, char **argv)
{
    struct pair pairs[13] = {
        {"key1", "value1"},
        {"key2", "value2"},
        {"key3", "value3"},
        {"key4", "value4"},
        {"key5", "value5"},
        {"key6", "value6"},
        {"key7", "value7"},
        {"key8", "value8"},
        {"key9", "value9"},
        {"key10", "value10"},
        {"ming", "love forever"},
        {"whoever", "I don't know"},
        {"love is ", "c'est la vie"},
    };

    HashMap map = create_hashmap(NULL, NULL, true);

    for(int i=0; i<13; i++) {
        PUT(map, pairs[i].key, strlen(pairs[i].key)+1, pairs[i].value);
    }
    printf("key: key5, exists: %s\n", EXISTS(map, "key5", 5) ? "true" : "false");
    printf("key5: %s\n", (char *)GET(map, "key5", 5));
    printf("remove key5 %s\n", REMOVE(map, "key5", 5) ? "true" : "false");
    printf("remove key5 %s\n", REMOVE(map, "key5", 5) ? "true" : "false");
    printf("key: key5, exists: %s\n", EXISTS(map, "key5", 5) ? "true" : "false");
    printf("remove whoever %s\n", REMOVE(map, "whoever", 8) ? "true" : "false");
    printf("key: whoever, exists: %s\n", EXISTS(map, "whoever", 8) ? "true" : "false");

    if(!hashmap_reset_iterator(map)) {
        printf("reset iterator failed.\n");
        map->destroy(&map);
        return -1;
    }

    HashMapIterator iterator = map->iterator;
    while (iterator->has_next_one(iterator)) {
        printf("{ key: %s, value: %s, key raw hashcode: %lu, list index: %u, conflicts: %u }\n",
            (char *)iterator->current->key, (char *)iterator->current->value, iterator->current->hashcode, iterator->lsidx, iterator->hashmap->conflicts);
        iterator->next_one(iterator);
    }

    printf("key: key9, exists: %s\n", EXISTS(map, "key9", 5) ? "true" : "false");
    printf("remove key9 %s\n", REMOVE(map, "key9", 5) ? "true" : "false");
    printf("key: key9, exists: %s\n", EXISTS(map, "key9", 5) ? "true" : "false");

    if(!hashmap_reset_iterator(map)) {
        printf("reset iterator failed.\n");
        map->destroy(&map);
        return -1;
    }

    iterator = map->iterator;
    while (iterator->has_next_one(iterator)) {
        printf("{ key: %s, value: %s, key raw hashcode: %lu, list index: %u, conflicts: %u }\n",
            (char *)iterator->current->key, (char *)iterator->current->value, iterator->current->hashcode, iterator->lsidx, iterator->hashmap->conflicts);
        iterator->next_one(iterator);
    }

    int i = 30;
    if(!hashmap_reset_iterator(map)) {
        printf("reset iterator failed.\n");
        map->destroy(&map);
        return -1;
    }
    printf("reset loop starts here:\n");
    while(--i > 0) {
        printf("{ key: %s, value: %s, key raw hashcode: %lu, list index: %u, conflicts: %u }\n",
            (char *)iterator->current->key, (char *)iterator->current->value, iterator->current->hashcode, iterator->lsidx, iterator->hashmap->conflicts);
        iterator->next_one(iterator);
    }

    map->destroy(&map);

    return 0;
}
