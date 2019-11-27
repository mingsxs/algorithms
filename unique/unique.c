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


//for cpp extension
#ifdef __cplusplus
extern "C" {
#endif


static unsigned long djb2(Elem e);
static BOOL is_character(char ch);
static FILE *open_file(const char *filename);
static uint64_t get_word_count(const char *filename);
static void cut_sentinel(List *data, unsigned long org_len);
extern BOOL elem_cmp(Elem a, Elem b);
extern BOOL elem_assign(Elem *dst, Elem *src);
extern BOOL elem_swap(List *data, unsigned long a, unsigned long b);

static BOOL is_character(char ch)
{
    return ((ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z') || \
                    (ch >= '0' && ch <= '9') || (ch == '-') || (ch == '_'));
}

/*
 * initialize elements array data.
 */
extern BOOL init_null_ctx(Elems *data, unsigned long elem_count)
{
    if (data == NULL)
        return false;

    if(data->head == NULL) {
        data->head = (Elem **)malloc(elem_count * sizeof(uint8_t *));
        memset(data->head, NULL, elem_count * sizeof(uint8_t *));
        data->len = 0;
    } else {
        printf("context object already initialized.\n");
    }
    return true;
}

/*
 * free the heap memory manually allocated, should cooperate with init_null_ctx.
 */
extern void free_ctx(List *ctx)
{
    unsigned long i;
    unsigned long list_size = sizeof ctx->head / sizeof(uint8_t *)
    for (i = 0; i < list_size; i++)
    {
        if(ctx->head[i]->word){
            free(ctx->head[i]->word);
            ctx->head[i]->word = NULL;
        }

        if(ctx->head[i]) {
            free(ctx->head[i]);
            ctx->head[i] = NULL;
            ctx->elem_count--;
        }

    }

    if(ctx->elem_count == 0) {
        if(ctx->head){
            free(ctx->head);
            ctx->head = NULL;
        }
    } else {
        printf("Error occurred, element count mismatch\n");
        ctx->elem_count = 0;
    }
}

/*
 * To compare 2 given elements. if equal, return true. else false.
 */
extern BOOL elem_cmp(Elem a, Elem b)
{
    if (a.size != b.size)
        return false;

    unsigned int i;
    for(i = 0; i < a.size; i++)
        if(a.content[i] != b.content[i])
            return false;

    return true;
}

/*
 * To assign src element to dst element.
 */
extern BOOL elem_assign(Elem *dst, Elem *src)
{
    if(dst == NULL || src == NULL || src->word == NULL || src->word_size <= 0)
        return false;

    if(dst->word_size != src->word_size)
        realloc(dst->word, src->word_size);

    memcpy(dst->word, src->word, src->word_size);

    return true;
}

/*
 * To swap 2 elements in an array.
 */
extern BOOL elem_swap(List *data, unsigned long a, unsigned long b)
{
    if(data == NULL || (a == b))
        return false;

    Elem *p = data->head[a];
    data->head[a] = data->head[b];
    data->head[b] = *p;
    return true;
}

/*
 * To cut the sentinels at the tail of the array.
 */
static void cut_sentinel(List *data, unsigned long org_len)
{
    unsigned long i;

    for(i = data->len; i < org_len; i++){
        free(data->head[i]);
        data->head[i] = NULL;
    }
}

/*
 * to tell a file readable or not, if so, return true, else false.
 */
static FILE *open_file(const char *filename)
{
    struct stat buf;

    stat(filename, &buf);

    if((buf.st_mode & S_IFMT) == S_IFDIR){
        printf("%s is a directory, specify a regular file\n", filename);
        return NULL;
    }

    if((buf.st_mode & S_IFMT) == S_IFREG && access(filename, F_OK)){
        printf("file %s exists, yet not accessible\n", filename);
        return NULL;
    }

    FILE *fh= fopen(filename, "r");
    if (fh == NULL) {
        printf("file %s can't be opened\n", filename);
        return NULL;
    }

    return fh;
}

/*
 * Get words count of a specific file.
 */
static uint64_t get_word_count(const char *filename)
{
    FILE *fh;
    if ((fh = open_file(filename)) == NULL)
        return ;

    uint64_t word_count = 0;
    char pre = 0;
    unsigned int len;
    int i;
    char line[1024];
    while (len = fread(line, 1, 1024, fh)) {
        for(i = 0; i < len; i++)
            if(!is_character(line[i]) && is_character(pre)){
                word_count ++;
                pre = line[i];
            }
    }

#ifdef __DEBUG__
    printf("file %s has %lu words.\n", filename, word_count);
#endif
    return word_count;
}

/*
 * To read a file and split into char array words for unique operation.
 */
BOOL collect_ctx_by_file(const char *filename, List *ctx)
{
    if (ctx == NULL || ctx->head == NULL)
        return false;

    FILE *fh;
    if((fh = read_file(filename)) == NULL)
        return false;

    char line[1024];
    unsigned int len;
    unsigned int i;
    unsigned int last;
    unsigned int word_size = 0;

    while(len = fread(line, 1, 1024, fh)){
        for (i = 0, last = 0; i < len; i++){
            if(is_character(line[i])) {
                word_size++;
                continue;
            } else if (word_size > 0) {
                Elem *elem = (Elem *)malloc(sizeof(Elem));
                if (elem == NULL) {
                    printf("Memory issue happens, can't malloc now \n");
                    return false;
                }
                elem->word_size= word_size + 1;
                elem->word = (uint8_t *)malloc(elem->word_size);
                memset(elem->content, 0x0, elem->word_size);
                memcpy(elem->content, &line[last], word_size);
                word_size = 0;
                last = i + 1;
                //if(ptr->len <= MAX_LENGTH){
                ctx->head[ctx->len] = elem;
                ctx->elem_count++;
#ifdef __DEBUG__
                printf("Below was appended to the list:\n");
                printf("%s   ", (const char*)(ctx->head[ctx->elem_count - 1]->word));
                printf("size:%u\n", ctx->head[ctx->elem_count - 1]->word_size);
#endif
                //} else {
                //    printf("Reserved %d size array list overflowed\n", MAX_LENGTH);
                //    return false;
                //}
            } else
                continue;
        }
    }

    fclose(fh);
#ifdef __DEBUG__
    int j;
    for(j = 0; j < ctx->elem_count; j++)
        printf("%s\n", ctx->head[j]->word);
    printf("size: %lu\n", ctx->elem_count);
#endif
    return true;
}


/*
 * Algorithm:
 *
 * For recursive operation of unique.
 * Note -
 *  after each cycle of unique operation, index meeting its key-hash will be put at the 1st part of data,
 *  the duplicates which are found in this cycle will be put at the 3rd part of data,
 *  elements not orgnized in this cycle marked as to-be-unique will be put at the 2nd pard between orgnized
 *  part and sentinels part. so basically, after one cycle done, the data will be fixed to array as below,
 *
 *      [orgnized part + to-be-unique + sentinels]
 */
static BOOL unique_loop(List *data, unsigned long start)
{
    if(data->elem_count - start < 2)
        return true;

    /*
     * Choose 1st element as sentinel element.
     * Note -
     *  1. sentinel is a import concept of this algorithm, sentinel will always be the element at
     *  start point, since the start value updates after this round unique operation, so it differs in each cycle.
     *  2. start point is the end point of orgnized elements and the begining of unorgnized ones in last cycle.
     */
    Elem sentinel = *data->head[start];

    Elems part; // mid part of unsorted.

    //1st element which is sentinel is excluded.
    part.head = &(data->head[start + 1]);
    part.len = data->len - start - 1;

    unsigned long index;

    //Traversing each item in to-be-unique, put elements to the index which is its key's hash modulu data length.
    for(index = 0; index < part.len; )
    {
        /*
         * handle elements between data[index] and data[hash_index], when data[hash_index], the real
         * index we should put data[index] to, is sentinel or duplicate, then we are free to do swap
         * between data[index] and data[hash_index].
         */
        //skip elements which equals to sentinel.
        if(elem_cmp(*part.head[index], sentinel)){
            index++;
            continue;
        }
        // hash_index is the index where we should actually put element data[index] to.
        unsigned long hash_index = djb2(*part.head[index]) % part.len;
#ifdef __DEBUG__
        printf("current word is: %s   ", part.head[index]->content);
        printf("word size is: %u   ", part.head[index]->size);
        printf("to-be-unique length is: %lu\n", part.len);
#endif
        //skip elements which is already in place.
        if(index == hash_index) {
            index++;
            continue;
        }
        // when index element equals to its key's hash modulu value, we find a duplicate, set to sentinel.
        if(elem_cmp(*part.head[index], *part.head[hash_index])){
            elem_assign(part.head[index], &sentinel);
            index++;
            continue;
        }
        // when data[hash_index] is sentinel, then place [hash_index] is available, we are free to
        // put data[index] to data[hash_index].
        if(elem_cmp(*part.head[hash_index], sentinel)){
           elem_swap(&part, hash_index, index);
            index++;
            continue;
        }

        /*
         * when element data[hash_index] is not sentinel either duplicate, we need to consider more
         * scenarios, like we should know where data[hash_index] should be placed to.
         */
        unsigned long Hash_hash_index = djb2(*part.head[hash_index]) % part.len;

        // when the place we should put data[hash_index] differs from the place we put data[index],
        // we can do swap.
        // if Hash_hash_index == hash_index, it means we should put data[index] and data[hash_index]
        // to the same place in the array, if them don't duplicate, it will be a hash collision.
        if (Hash_hash_index != hash_index){
            elem_swap(&part, hash_index, index);
            // only when we put index element which is being operated now to the place before index,
            // we can traverse to next element, otherwise if we put index element behind, where is
            // inside to_be_unique part, we should check data[index] again after the swap, that's why
            // we use a if condition here.
            if (hash_index < index)
                index++;
        } else
            index++;

    }

    /*
     * Move elements which has been placed to its hash_index to the beginning of the array.
     */
    unsigned long i;
    unsigned long swapPos = 0;
    for(i = 0; i < part.len; i++) {
        if(!elem_cmp(*part.head[i], sentinel) && (i == djb2(*part.head[i]) % part.len))
            elem_swap(&part, i, swapPos++);
    }

    /*
     * Move duplicate elements which has been changed to sentinel to the tail of the array.
     */
    unsigned long sentinelPos = part.len;
    for(i = swapPos; i < sentinelPos; ) {
        if(elem_cmp(*part.head[i], sentinel))
            // we don't do i++ here, because we only know data[i] is sentinel, we don't know
            // if data[sentinelPos] is sentinel too, we should check data[i] again which is the
            // data[sentinelPos] before the swap operation after the swap.
            elem_swap(&part, i, --sentinelPos);
        else
            i++;
    }

    /*
     * Adjust the start point, recursively loop down.
     */
    data->len = sentinelPos + start + 2;
    start = start + swapPos + 2;

    unique_loop(data, start);

}

// C implementation
BOOL process_ctx(List *data)
{
    if (data == NULL)
        return false;

    //Save original length before unique operation. for cutting tail sentinel elements.
    unsigned long array_len = data->elem_count;

    if(unique_loop(data, 0))
        cut_sentinel(data, array_len);

    return true;
}

/*
 * To print the list.
 */
BOOL print_list(Elems *data, DataType type)
{
    if (data == NULL)
        return false;

    unsigned long i;
    unsigned int j = 0;

    printf("array listed as below:\n");
    for (i = 0; i < data->len; i++, j++){
        if(j == 10){
            j = 0;
            printf("\n");
        }
        switch (type) {
            case STRING: 
                printf("%s\t", (char *)(data->head[i]->content));
            case CHAR:
                printf("%c\t", (char)(*data->head[i]->content));
            case UCHAR:
                printf("%c\t", (char)(*data->head[i]->content));
            case INT:
                printf("%d\t", (int32_t)(*data->head[i]->content));
            case UINT:
                printf("%u\t", (uint32_t)(*data->head[i]->content));
            case LONG:
                printf("%lld\t", (int64_t)(*data->head[i]->content));
            case ULONG:
                printf("%llu\t", (uint64_t)(*data->head[i]->content));
            case FLOAT:
                printf("%F\t", (float)(*data->head[i]->content));
            case DOUBLE:
                printf("%F\t", (double)(*data->head[i]->content));
        }
    }

    return true;
}

#ifdef __cplusplus
}
#endif  //cpp extension
