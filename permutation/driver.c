/*
 *
 * Description: permutation driver code.
 * Author: Ming Li(adagio.ming@gmail.com)
 * License:
 *    No license, for learning purpose.
 * Note:
 *    Only character strings will be allowed as input.
 * Date: 2019/04/15
 *
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>


extern int permutation(char *str);

static int is_str(char *);

int main (int argc, char **argv) {
	if (argc == 2 && (!is_str(argv[1]))) {
		if (permutation(argv[1])) {
			printf("Error happened when processing!\n");
			return -1;
		}
	} else {
		printf ("Usage:\n\tpermutate string\n");
	}
	return 0;
}


static int is_str(char *src) {
	size_t len = strlen(src);
	int i;

	for (i=0; i<len; i++) {

		if(!isalpha(src[i])) {
			return -1;
		}
	}

	return 0;
}
