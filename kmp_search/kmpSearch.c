#include <stdio.h>
#include <stdlib.h>
#include <string.h>


/* external API interface */
extern int kmp_search(const char *source, const char *pattern);

/* local forward declarations */
static int *kmp_index_table_p = NULL;          /* kmp partial match index table */

/* function to create kmp table */
static void kmp_create_table(const char *pattern)
{
    int p_len = (int )strlen(pattern);
    int i, j;

    if(kmp_index_table_p) {
        free(kmp_index_table_p);
        kmp_index_table_p = NULL;
    }

    kmp_index_table_p = (int *)malloc(p_len*sizeof(int));
    /* initialize with 0xFF to avoid 0 index case */
    memset(kmp_index_table_p, 0xff, p_len*sizeof(int));

    kmp_index_table_p[0] = 0;
    /*
     * hopefully, it's very easy to understand.
     * j points to first part of the given pattern string which has all unique characters.
     * i is iteration index of both the given pattern string and the kmp index table items.
     * eg:
     *    if pattern is 'ABCDABE', j will always traverse within first part with all unique
     *    characters 'ABCD', and i is the iteration index of whole pattern string and local
     *    kmp index table.
     *
     */

    for(j=0,i=1; pattern[i]; ) {
        if(pattern[i] == pattern[j]) {
            kmp_index_table_p[i++] = ++j;
        } else {
            if(j == 0) {
                kmp_index_table_p[i++] = j;
            } else {
                j = 0;
            }
        }
    }
}


int kmp_search(const char *source, const char *pattern)
{
    int p_len = (int) strlen(pattern);
    int i, j, shift;

    kmp_create_table(pattern);

    for(i=0,j=0; source[i]&&pattern[j]; ){
        if(source[i]==pattern[j]) {
            ++i;
            ++j;
            continue;
        }
        if(j == 0) {
            ++i;
            continue;
        }

        shift = j - kmp_index_table_p[j-1];
        j-=shift;
    }

    return !pattern[j] ? (i- p_len) : -1;
}
