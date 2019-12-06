#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "unique.h"


int main(int argc, char **argv)
{
	char *path = "./book.txt";
	UniqList list = create_uniq_list(path);

	list->feedtxt(list);
	list->unique_func(list);
	list->flushspace(list);
	list->writebacktxt(list);
	list->destroy(list);

	printf("total words: %u\n", list->list_len);
	printf("removed words: %u\n", list->removed);
	printf("rest words: %u\n", list->words);

	return 0;
}

