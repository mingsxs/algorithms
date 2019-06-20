/*
 *
 * Description: permutation source.
 * Author: Ming Li(adagio.ming@gmail.com).
 * License:
 *    No license, for learning purpose only.
 * Date: 2019/04/15
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* external interface */
extern int permutation(char *str);
/* local function declaritions */
static void sub_permutation(char *str, int rleft);
static long fact(int num);
/* local variable declaritions */
static int size = 0;
static char *line = NULL;
static long count = 0;


int permutation (char *str) {

	size = (int) strlen(str);

	if ((line = (char *)malloc(size)) == NULL) {
		printf("Memory allocation failure.\n");
		return -1;
	}
	memset(line, 0x0, size);

	printf("string: %s, string size: %d, factorial: %ld\n\n", str, size, fact(size));

	sub_permutation (str, size);

	printf("\nactual lines: %ld\n", count);

	if(line) {
		free(line);
	}

	return 0;
}

static void sub_permutation (char *str, int rleft) {
	if (rleft == 0) {
		memcpy(line, str, size);
		putchar('\t');
		puts(line);
		memset(line, 0x0, size);
		++count;
	} else if (rleft > 0) {

		char *pos, *head = str + size -rleft;
		char temp;

		for (pos = head; *pos; pos++) {

			/* swap current character with current first character */
			if(pos != head) {
				temp = *head;
				*head = *pos;
				*pos = temp;
			}

			/* recursively process down */
			sub_permutation (str , rleft-1);

			/* swap back */
			if(pos != head) {
				temp = *head;
				*head = *pos;
				*pos = temp;
			}
		}
	}
}

static long fact(int num) {
	if (num == 1) {
		return num;
	}

	return num*fact(num-1);
}
