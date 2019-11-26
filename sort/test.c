#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

// extern outscope functions
extern void bubble_sort(int *array, int len);
extern void select_sort(int *array, int len);
extern void merge_sort(int *array, int len);
extern void quick_sort(int *array, int len);
extern void shell_sort(int *array, int len);
extern void insert_sort(int *array, int len);
extern void heap_sort(int *array, int len);
extern void count_sort(int *array, int len);
extern void radix_sort(int *array, int len);


#define ARRAY_LENGTH 16

int main(int argc, char **argv)
{
    int array[ARRAY_LENGTH] = {0};
    srand(time(NULL));

    for(int i=0, j=0; i<9; i++) {
        memset(array, 0x0, sizeof array);
        /* generate random number to initialize array and print*/
        printf("%d round, raw:\t { ", i+1);
        for(j=0; j<ARRAY_LENGTH; j++) {
            array[j] = rand() % 100;
            printf(j+1<ARRAY_LENGTH? "%d, ":"%d ", array[j]);
        }
        printf("}\n");
        switch (i) {
            case 0:
                bubble_sort(array, ARRAY_LENGTH);
                printf("bubble sort:\t { ");
                break;
            case 1:
                select_sort(array, ARRAY_LENGTH);
                printf("select sort:\t { ");
                break;
            case 2:
                merge_sort(array, ARRAY_LENGTH);
                printf("merge sort:\t { ");
                break;
            case 3:
                quick_sort(array, ARRAY_LENGTH);
                printf("quick sort:\t { ");
                break;
            case 4:
                shell_sort(array, ARRAY_LENGTH);
                printf("shell sort:\t { ");
                break;
            case 5:
                insert_sort(array, ARRAY_LENGTH);
                printf("insert sort:\t { ");
                break;
            case 6:
                heap_sort(array, ARRAY_LENGTH);
                printf("heap sort:\t { ");
                break;
            case 7:
                count_sort(array, ARRAY_LENGTH);
                printf("count sort:\t { ");
                break;
            case 8:
                radix_sort(array, ARRAY_LENGTH);
                printf("radix sort:\t { ");
                break;
        }
        for(j=0; j<ARRAY_LENGTH; j++)
            printf(j+1<ARRAY_LENGTH? "%d, ":"%d ", array[j]);
        printf("}\n\n");
    }

    return 0;
}
