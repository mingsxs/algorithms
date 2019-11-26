#include <sys/types.h>
#include <stdlib.h>
#include <stdio.h>

/* macros doing swap */
#define SWAP(x, y) do { \
if(*x != *y) {          \
    *x = *x ^ *y;       \
    *y = *x ^ *y;       \
    *x = *x ^ *y; }} while(0)

/* macros doing memzero */
#define MEMZERO(__dest_p, __size_t) do { \
unsigned int i; char *start=NULL; \
for(start=(char *)(__dest_p), i=0; i<(__size_t); i++) \
    *(start + i) = 0x0; } while(0)

/* macros doing memcpy */
#define MEMCPY(__dest_p, __src_p, __size_t) do { \
unsigned int i; char *start=NULL; \
for(start=(char *)(__dest_p), i=0; i<(__size_t); i++) \
    *(start + i) = *((char *)(__src_p) + i); } while(0)

extern void bubble_sort(int *array, int len);
extern void select_sort(int *array, int len);
extern void merge_sort(int *array, int len);
extern void quick_sort(int *array, int len);
extern void shell_sort(int *array, int len);
extern void insert_sort(int *array, int len);
extern void heap_sort(int *array, int len);
extern void count_sort(int *array, int len);
extern void radix_sort(int *array, int len);

/*--------------------------------------------------------------------------------*/
void
bubble_sort(int *array, int len)
{
    int i, j;

    for(i=len; i>0; i--)
        for(j=1; j<i; j++)
            if(array[j-1] > array[j])
                SWAP(&array[j-1], &array[j]);
}
/*--------------------------------------------------------------------------------*/
void
select_sort(int *array, int len)
{
    int i, j, min_idx;

    for(min_idx=0, i=1; i<len; i++) {
        for(j=i; j<len; j++)
            if(array[j] < array[min_idx])
                min_idx = j;
        SWAP(&array[min_idx], &array[i-1]);
        min_idx = i;
    }
}
/*--------------------------------------------------------------------------------*/
void
merge_sort(int *array, int len)
{
    if(len <= 1)
        return ;

    int mid = len/2;

    merge_sort(array, mid);             // sort left sub-array
    merge_sort(array+mid, len-mid);     // sort right sub-array

    /* merge 2 sorted left and right sub-arrays to 1 sorted array */
    int larraylen = mid, rarraylen = len-mid;
    int *larray = array, *rarray = array+mid;
    int temp, *start, i, j;

    int *result = (int *)malloc((larraylen+rarraylen) * sizeof(int));

    for(i=0, j=0; i<larraylen && j<rarraylen; ) {
        if(larray[i] < rarray[j])
            temp = larray[i++];
        else
            temp = rarray[j++];

        result[i+j-1] = temp;
    }
    /* append the rest if there are some */
    if(i == larraylen) {
        temp = rarraylen - j;
        start = rarray + j;
    } else {
        temp = larraylen - i;
        start = larray + i;
    }
    for(int k = 0; k < temp; k++)
        result[i+j+k] = *(start + k);
    /* copy array memory */
    for(i = 0; i < larraylen+rarraylen; i++)
        *(larray + i) = result[i];

    free(result);
}
/*--------------------------------------------------------------------------------*/
void
insert_sort(int *array, int len)
{
    // cur : current walk index, walk through unsorted part, --->
    // pre : previous walk index, walk through sorted part, <---
    // ins : current value to be inserted after the one less or equal
    int cur, pre, ins;

    for(cur=1; cur<len; cur++) {
        ins = array[cur];       // pick the next one as the new inserted
        pre = cur-1;            // walk backwards in previous sorted part
        while(pre >= 0 && array[pre] > ins) {
            array[pre+1] = array[pre];  // if new less than his previous one, move the previous one to overwrite his next
            pre--;
        }
        array[pre+1] = ins;
    }
}
/*--------------------------------------------------------------------------------*/
void
quick_sort(int *array, int len)
{
    if(len < 2)
        return ;

    int i = 0, j = len-1, pivot = array[0]; // always use the first element as pivot.

    while(1) {
        while(i<j && array[j]>=pivot) j--;  // j takes the first step to find smaller value from right side.
        if(i == j) break;
        array[i] = array[j];
        while(i<j && array[i]<=pivot) i++;
        if(i == j) break;
        array[j] = array[i];
    }
    array[i] = pivot;                       // fill the pivot to the location where i and j meet.

    quick_sort(array, i);                   // sort left side of the pivot location
    quick_sort(array+i+1, len-i-1);         // sort right side of the pivot location
}
/*--------------------------------------------------------------------------------*/
/* adjustify the heap to a new maxheap after root node is changed */
static void
MaxHeapify(int *array, int len, int root)
{
    int lchild = root*2 + 1;
    int rchild = root*2 + 2;
    int largest = root;

    if(lchild<len && array[lchild] > array[largest])
        largest = lchild;

    if(rchild<len && array[rchild] > array[largest])
        largest = rchild;

    if(largest != root) {
        SWAP(&array[root], &array[largest]);
        MaxHeapify(array, len, largest);
    }
}

void
heap_sort(int *array, int len)
{
    /* initialize the maxheap */
    /* starts with the last non-leaf, then backwards */
    /* think about why the last non-leaf array index is (len/2)-1 ?? */
    for(int i=(len/2)-1; i>=0; i--)
        MaxHeapify(array, len, i);
    /* do swap and MaxHeapify */
    for(int last = len-1; last>0; last--) {
        SWAP(&array[0], &array[last]);

        MaxHeapify(array, --len, 0);
    }
}
/*--------------------------------------------------------------------------------*/
void
count_sort(int *array, int len)
{
    int min = array[0];
    int max = array[0];

    for(int i=1; i<len; i++)
        if(array[i] > max)
            max = array[i];
        else if(array[i] < min)
            min = array[i];

    int array_counter_len = max + min - 1;
    int *array_counter = (int *)malloc(array_counter_len*sizeof(int));
    if(array_counter == NULL) {
        printf("Overflow while allocating %lu byte memory\n", array_counter_len*sizeof(int));
        return ;
    }
    MEMZERO(array_counter, array_counter_len);
    /* count sorting array */
    for(int i=0; i<len; i++)
        array_counter[array[i]-min]++;
    /* do accumulating to caculate array index */
    for(int i=1; i<array_counter_len; i++)
        array_counter[i] += array_counter[i-1];

    for(int i=0, previous=0; i<array_counter_len; previous=array_counter[i++])
        /* fill each bucket reversely in target array */
        for(int index=array_counter[i]-1; index>=previous; index--)
            array[index] = min + i;

    free(array_counter);
}
/*--------------------------------------------------------------------------------*/
void
radix_sort(int *array, int len)
{
    /* define a const var as the radix for sorting */
    const int radix = 10;
    /* allocate temp array to do count sorting for different orders */
    int *counter = (int *)malloc(radix * sizeof(int));
    /* allocate temp array to store temp results after sorting for one order */
    int *temp = (int *)malloc(len * sizeof(int));

    int max = array[0];
    int i, counter_index, counter_buf;
    /* get max value */
    for(i=0; i<len; i++)
        if(array[i] > max)
            max = array[i];
    /* do sorting for every order */
    for(int mod=1; max/mod>0; mod*=radix) {
        /* do count sorting for one order */
        MEMZERO(counter, radix*sizeof(int));
        for(i=0; i<len; i++) {
            counter_index = array[i]/mod - (array[i]/(mod*radix))*radix;
            counter[counter_index]++;
        }

        for(i=1, counter_buf=counter[0], counter[0]=0; i<radix; i++) {
            counter_buf += counter[i-1];
            SWAP(&counter_buf, &counter[i]);
        }

        for(i=0; i<len; i++) {
            counter_index = array[i]/mod - (array[i]/(mod*radix))*radix;
            temp[counter[counter_index]++] = array[i];
        }

        MEMCPY(array, temp, len*sizeof(int));
    }

    free(counter);
    free(temp);
}
/*--------------------------------------------------------------------------------*/
void
shell_sort(int *array, int len)
{
    int i, j, gap, current;
    /* initialize gap with len/2, think about why?? */
    for(gap=len/2; gap>0; gap--)
        for(i=gap; i<len; i++) {
            for(j=i, current=array[i]; j>=gap && current<array[j-gap]; j-=gap)
                array[j] = array[j-gap];

            array[j] = current;     // insert array[i] e.a.current.
        }
}
