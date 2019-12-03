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
    struct pair pairs[] = {
        {"key1", "value1"},
        {"key2", "value2"},
        {"key3", "value3"},
        {"reqwhrun", "value4"},
        {"key5", "value5"},
        {"key6", "value6"},
        {"key7", "value7"},
        {"key8", "value8"},
        {"key9", "value9"},
        {"key10", "value10"},
        {"ming", "love forever"},
        {"whoever", "I don't know"},
        {"love is ", "c'est la vie"},
        {"hifhardaf", "hello world"},
        {"hifhardaf", "hello world"},
        {"dfasfas", "world"},
        {"daerqr", "world"},
        {"key10", "here what"},
        {"reqwhrun", "dfasdf"},
        {"jrjwehrui", "dfnjashfu"},
        {"fjsafdafs", "fnjsfjasnfj"},
        {"nfjsnfjas", "bjfsbfjasfj"},
        {"fnjsnfjasfj", "fnsjfbjasfwerqh"},
        {"nfjshfhauwrhqw", "bjfsajf"},
        {"fjabd", "njfsnfj"},
        {"njfhwqur", "njfsnf"},
        {"nfahruewh", "njfs"},
        {"fjajfsaj", "njfsfj"},
        {"nfdsfjajfj", "njfnfjdf"},
        {"fjanjfdshjjfj", "njfsnf234"},
        {"njfsfjajfj", "njfsnf32"},
        {"fjadnjfajfj", "njfsnfj"},
        {"jfnafjajfj", "njfsnfj"},
        {"fjanbjjfj", "njfsnfj"},
        {"fjajfnmvnsaj", "njfsnfj"},
        {".jfjajfj", "njfsnfj"},
        {"fm,ujajfj", "njfsnfj"},
        {"fjajsdajfj", "njfsnfj"},
        {"fjajfndawj", "njfsnfj"},
        {"djfashfja", "nfisafsa"},
        {"fjsakf", "fsjakfajs"},
        {"32422fjajfj", "nfjaskfj"},
        {"32423fjajfj", "njfsnfj"},
        {"32425fjajfj", "njfsnfj"},
        {"32430fjajfj", "njfsnfj"},
        {"32427fjajfj", "njfsnfj"},
        {"32426fjajfj", "njfsnfj"},
        {"32431fjajfj", "njfsnfj"},
        {"1fjanjfdshjjfj", "njfsnf234"},
        {"2njfsfjajfj", "njfsnf32"},
        {"3fjadnjfajfj", "njfsnfj"},
        {"4jfnafjajfj", "njfsnfj"},
        {"5fjanbjjfj", "njfsnfj"},
        {"6fjajfnmvnsaj", "njfsnfj"},
        {"7.jfjajfj", "njfsnfj"},
        {"8fm,ujajfj", "njfsnfj"},
        {"1ming", "love forever"},
        {"2whoever", "I don't know"},
        {"3love is ", "c'est la vie"},
        {"4hifhardaf", "hello world"},
        {"5hifhardaf", "hello world"},
        {"7dfasfas", "world"},
        {"8daerqr", "world"},
        {"9key10", "here what"},
        {"areqwhrun", "dfasdf"},
        {"bjrjwehrui", "dfnjashfu"},
        {"cjfjsafdafs", "fnjsfjasnfj"},
        {"djnfjsnfjas", "bjfsbfjasfj"},
        {"efnjsnfjasfj", "fnsjfbjasfwerqh"},
        {"fnfjshfhauwrhqw", "bjfsajf"},
        {"gfjabd", "njfsnfj"},
        {"hnjfhwqur", "njfsnf"},
        {"infahruewh", "njfs"},
        {"jfjajfsaj", "njfsfj"},
        {"knfdsfjajfj", "njfnfjdf"},
        {"lfjanjfdshjjfj", "njfsnf234"},
        {"mnjfsfjajfj", "njfsnf32"},
        {"nfjadnjfajfj", "njfsnfj"},
        {"ojfnafjajfj", "njfsnfj"},
        {"pfjanbjjfj", "njfsnfj"},
        {"qfjajfnmvnsaj", "njfsnfj"},
        {"r.jfjajfj", "njfsnfj"},
        {"sfm,ujajfj", "njfsnfj"},
        {"tfjajsdajfj", "njfsnfj"},
        {"ufjajfndawj", "njfsnfj"},
        {"w=vdjfashfja", "nfisafsa"},
        {"vfjsakf", "fsjakfajs"},
        {"w32422fjajfj", "nfjaskfj"},
        {"x32423fjajfj", "njfsnfj"},
        {"y;32425fjajfj", "njfsnfj"},
        {"z32430fjajfj", "njfsnfj"},
        {"ff32427fjajfj", "njfsnfj"},
    };

    HashMap map = create_hashmap(NULL, NULL);

    for(int i=0; i<sizeof pairs/sizeof(struct pair); i++) {
        PUT(map, pairs[i].key, strlen(pairs[i].key)+1, pairs[i].value);
    }
    printf("key: key5, exists: %s\n", EXISTS(map, "key5", 5) ? "true" : "false");
    printf("key5: %s\n", (char *)GET(map, "key5", 5));
    printf("remove key5 %s\n", REMOVE(map, "key5", 5) ? "true" : "false");
    printf("remove key5 %s\n", REMOVE(map, "key5", 5) ? "true" : "false");
    printf("key: key5, exists: %s\n", EXISTS(map, "key5", 5) ? "true" : "false");
    printf("remove whoever %s\n", REMOVE(map, "whoever", 8) ? "true" : "false");
    printf("key: whoever, exists: %s\n", EXISTS(map, "whoever", 8) ? "true" : "false");
    printf("key: reqwhrun: %s\n", GET(map, "reqwhrun", 9));

    if(!hashmap_reset_iterator(map)) {
        printf("reset iterator failed.\n");
        map->free(&map);
        return -1;
    }

    for(int i=sizeof pairs/sizeof(struct pair)-1; i>8; i--) {
        printf("remove key:%s, %s\n", pairs[i].key, REMOVE(map, pairs[i].key, strlen(pairs[i].key)+1) ? "true": "false");
    }

    HashMapIterator iterator = map->iterator;
    int count = 0;
    while(1) {
        printf("{ key: %s, value: %s, key raw hashcode: %lu, list index: %u, conflicts: %u, count:%d }\n",
            (char *)iterator->current->key, (char *)iterator->current->value, iterator->current->hashcode, iterator->lsidx, iterator->hashmap->conflicts, ++count);
        if(!iterator->has_next_one(iterator))
            break;
        iterator->next_one(iterator);
    };

    printf("key: key9, exists: %s\n", EXISTS(map, "key9", 5) ? "true" : "false");
    printf("remove key9 %s\n", REMOVE(map, "key9", 5) ? "true" : "false");
    printf("key: key9, exists: %s\n", EXISTS(map, "key9", 5) ? "true" : "false");

    if(!hashmap_reset_iterator(map)) {
        printf("reset iterator failed.\n");
        map->free(&map);
        return -1;
    }

    iterator = map->iterator;
    count = 0;
    while(1) {
        printf("{ key: %s, value: %s, key raw hashcode: %lu, list index: %u, conflicts: %u, count:%d }\n",
            (char *)iterator->current->key, (char *)iterator->current->value, iterator->current->hashcode, iterator->lsidx, iterator->hashmap->conflicts, ++count);
        if(!iterator->has_next_one(iterator))
            break;
        iterator->next_one(iterator);
    };

    int i = 30;
    if(!hashmap_reset_iterator(map)) {
        printf("reset iterator failed.\n");
        map->free(&map);
        return -1;
    }
    printf("reset loop starts here:\n");
    iterator = map->iterator;
    count = 0;
    while(--i > 0) {
        printf("{ key: %s, value: %s, key raw hashcode: %lu, list index: %u, conflicts: %u, count:%d }\n",
            (char *)iterator->current->key, (char *)iterator->current->value, iterator->current->hashcode, iterator->lsidx, iterator->hashmap->conflicts, ++count);
        iterator->next_one(iterator);
    }

    map->free(&map);

    return 0;
}
