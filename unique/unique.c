/*
 * Author: Ming Li
 * Date: 2018/12/11
 * Note -:
 *  1. codes follow C99 standard compilation. only MacOS has been verified.
 *  2. not original thought, it just a C implementation of thought of dsimcha on StackOverflow.
 *  3. for more info, please go website as below,
 *  https://stackoverflow.com/questions/1532819/algorithm-efficient-way-to-remove-duplicate-integers-from-an-array#
 *
 */

#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>   // mmap, munmap

#include "unique.h"

extern void hash_in_place_unique(UniqList list, unsigned long start, unsigned long end);
extern unsigned int hashword(void *word, int mod);

static bool equalword(char *word1, char *word2);
static void wipeword(char *word);

void
hash_in_place_unique(UniqList list, unsigned long start, unsigned long length)
{
    char **wordblock = &list->list[start];

    if(length <= 1)
        return ;

    /* use in-place hash index to remove duplicates */
    unsigned int current = 0, hash, hash_of_hash;
    char *swap;
    while(current < length) {

        if(wordblock[current] == NULL) continue;

        hash = hashword(wordblock[current], length);  // calculate current word hash value as index in list

        if(hash == current) { // if in place luckily, move to next one
            current ++;
            continue;
        }
        /* current word not in place with its hash index */
        if(wordblock[hash] == NULL) {   // target place is a removed duplicate, just put current word in place and move on
            wordblock[hash] = wordblock[current];
            wordblock[current] = NULL;
            current ++;
        }

        if(equalword(wordblock[current], wordblock[hash])) {    // current word is a duplicate, wipe this word and move to next one
            wipeword(wordblock[current]);
            wordblock[current] = NULL;
            list->removed ++;
            current ++;
            continue;
        }

        /* the rest scenarios are hash bumps, deal with them */
        hash_of_hash = hashword(wordblock[hash], length);  // calculate target place word hash value and check
        /* target word is not in place with its hash index */
        if(hash_of_hash != hash) {
            swap = wordblock[hash];
            wordblock[hash] = wordblock[current];
            wordblock[current] = swap;    // swap words, put current word in place first

            if(hash < current)      // if target index is less than current, means target place is in front of current place,
                current ++;         // the word we swapped back has been processed before, then we just move to next,
                                    // otherwise it's a new word, we have to keep current index, and continue to process it.
        /* target word is in place with its hash index */
        } else {
            current ++;     // now that target word is in place, we just move on
        }
    }

    /* reorder the list, calculate hash bumps and get new list to be ordered */
    unsigned int in_place_pos = 0, nullptr_counter = 0;
    /* process in place words and wiped NULL words */
    for(unsigned int i=0; i<length; i++) {
        if(hashword(wordblock[i], length) == i) {  // in place words
            swap = wordblock[in_place_pos];
            wordblock[in_place_pos++] = wordblock[i];
            wordblock[i] = swap;                   // swap words, move words to the beginning of list
        }
    }

    for(unsigned int i=in_place_pos; i<length; i++) {
        if(wordblock[i] == NULL)
            nullptr_counter ++;
        else
            wordblock[i-nullptr_counter] = wordblock[i];
    }

    unsigned long new_start = in_place_pos, new_length = length - nullptr_counter - in_place_pos;
    list->words -= nullptr_counter;

    hash_in_place_unique(list, new_start, new_length);
}

unsigned int
hashword(void *word, int mod)
{
    if(!word)
        return -1;

    uint32_t hash = 5381;
    int8_t *p = word;

    while(*p++)
        hash = ((hash << 5) + hash) + *p;

    return (unsigned int)hash % mod;
}

bool
equalword(char *word1, char *word2)
{
    if(!(word1 && word2))
        return false;

    return strcmp(word1, word2) ? false : true;
}

void
wipeword(char *word)
{
    size_t size = strlen(word);

    for(int i=0; i<size; i++)
        word[i] = ' ';
}
/*
static void
default_print(UniqList list)
{
    if(!list)
        return ;

    printf("Below lists word arrangement after unique:\n");
    for(long i=0, j=0; i<list->words; i++, j++) {
        if(j == 15) {
            putchar('\n');
            j = 0;
        }

        if(list->list[i])
            printf("%s, ", list->list[i]);
    }

    putchar('\n');
}
*/
static void
default_unique_func(UniqList list)
{
    hash_in_place_unique(list, 0, list->list_len-1);
}

static void
default_destroy(UniqList list)
{
    if(!list)
        return ;

    if(list->list)
        free(list->list);

    free(list);
}

static void
default_flush(UniqList list)
{
    char *block = list->mapblock;
    // flush all spaces in block
    for(long i=0, spaces=0; i<list->blocksize; i++) {
        if(block[i] == ' ') spaces++;
        else block[i-spaces] = block[i];
    }
}

static void
default_feedtxt(UniqList list) {
    // bind all fixed words to list list
    char *block = list->mapblock, c;
    bool walkin = false;
    for(long i=0, j=0; i<list->blocksize; i++) {
        c = block[i];
        if(((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) && walkin==false) {
            list->list[j++] = &block[i];
            walkin = true;      // walk inside a word
        } else if(!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) && walkin==true) {
            walkin = false;     // walk outside a word
        }
    }
}

static void
default_writebacktxt(UniqList list)
{
    char *block = list->mapblock;
    // revert '\0' back to ' '(space)
    for(long i=0; i<list->blocksize; i++)
        if(block[i] == '\0')
            block[i] = ' ';

    munmap(list->mapblock, list->blocksize);
    list->mapblock = NULL;
}

UniqList create_uniq_list(const char *path)
{
    if(!path)
        return NULL;

    struct stat fstatus;
    stat(path, &fstatus);

    /* not a regular file */
    if((fstatus.st_mode&S_IFMT) != S_IFREG) {
        printf("Error: no such regular file.\n");
        return NULL;
    }

    // copy to new file.
    char writebackpath[64], syscmd[64];
    strcpy(writebackpath, path);
    strcat(writebackpath, ".back");
    if(access(writebackpath, F_OK) != -1) {
        sprintf(syscmd, "rm -rf %s", writebackpath);
        system(syscmd);
    }
    sprintf(syscmd, "cp %s %s", path, writebackpath);
    system(syscmd);

    /* map file as memory block to user space with mmap system call*/
    int fd = open(writebackpath, O_RDWR | O_CREAT);
    if(fd == -1) {
        printf("Error: open file failed.\n");
        return NULL;
    }

    void *mapblock = mmap(NULL, fstatus.st_size, PROT_READ|PROT_WRITE, MAP_SHARED, fd, 0);

    if(mapblock == MAP_FAILED) {   // map failed
        printf("Error: map file to memory failed.\n");
        return NULL;
    }
    if(close(fd) == -1) {   // close opened fd as we have already mapped this block to memory
        printf("Error: close fd failed.\n");
        munmap(mapblock, fstatus.st_size);
        return NULL;
    }

    char *block = mapblock, c;
    long i;
    // change all non-alphabet char to ' '(space)
    for(i=0; i<fstatus.st_size; i++) {
        c = block[i];
        if(!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) && c!='\n')
            block[i] = ' ';
    }

    long word_counter = 0;
    bool walkin = false;
    // count words and also trim words
    for(i=0; i<fstatus.st_size; i++) {
        c = block[i];
        if(((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) && walkin==false) {
            word_counter++;
            walkin = true;
        } else if(!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z')) && walkin==true) {
            block[i] = '\0';
            walkin = false;
        }
    }

    // initialize list structure
    UniqList pool = newUniqStruct();
    pool->words = word_counter;
    pool->removed = 0;
    pool->mapblock = mapblock;
    pool->blocksize = (unsigned long)fstatus.st_size;
    pool->list = newWordList(word_counter);
    MEMZERO(pool->list, word_counter * sizeof(char *));
    pool->list_len = word_counter;
    pool->feedtxt = default_feedtxt;
    pool->writebacktxt = default_writebacktxt;
    pool->unique_func = default_unique_func;
    pool->flushspace = default_flush;
    pool->destroy = default_destroy;

    return pool;
}
