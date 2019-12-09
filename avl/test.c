#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>

#include "avl.h"


int main(int argc, char **argv)
{
    int64_t array[] = {65, 99, 53, 26, 9, 87, 42, 38, 18, 95, 12, 3, 38, 45, 42, 89};

    AVLTree at = init_avl_tree(NULL);

    for(int i=0; i<sizeof array/sizeof(int64_t); i++) {
        AVLINSERT(at, (void *)array[i]);
    }

    AVLTRAVERSE(at);
    printf("exists: %d, %s\n", 42, AVLEXISTS(at, (void *)42)? "true":"false");
    printf("exists: %d, %s\n", 25, AVLEXISTS(at, (void *)25)? "true":"false");
    printf("%d predecessor is %lld\n", 42, (int64_t)AVLPREDECESSOR(at, (void *)42));
    printf("%d successor is %lld\n", 87, (int64_t)AVLSUCCESSOR(at, (void *)87));
    printf("exists: %d, %s\n", 20, AVLEXISTS(at, (void *)20)? "true":"false");
    printf("exists: %d, %s\n", 35, AVLEXISTS(at, (void *)35)? "true":"false");
    printf("remove: %d, %s\n", 42, AVLREMOVE(at, (void *)42)? "true":"false");
    printf("remove: %d, %s\n", 35, AVLREMOVE(at, (void *)35)? "true":"false");
    printf("exists: %d, %s\n", 42, AVLEXISTS(at, (void *)42)? "true":"false");
    printf("exists: %d, %s\n", 65, AVLEXISTS(at, (void *)65)? "true":"false");
    printf("remove: %d, %s\n", 65, AVLREMOVE(at, (void *)65)? "true":"false");
    printf("exists: %d, %s\n", 65, AVLEXISTS(at, (void *)65)? "true":"false");
    printf("%d predecessor is %lld\n", 42, (int64_t)AVLPREDECESSOR(at, (void *)42));
    printf("%d successor is %lld\n", 65, (int64_t)AVLSUCCESSOR(at, (void *)65));
    AVLTRAVERSE(at);

    at->freespace(&at);

    return 0;
}
