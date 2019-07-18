/*
 * driver code for algorithm KMP pattern search.
 * Ming Li(adagio.ming@gmail.com)
 * 2019/07/18
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>


extern int kmp_search(char *source, char *pattern);

static void strip_space(char *str) 
{
    int i, shift = 0;

    for(i=0; str[i]; i++) {
        if(str[i]==' ' || str[i]=='\n') {
            ++shift;
        } else {
            str[i-shift] = str[i];
        }
    }

    str[i-shift] = '\0';
}

static void stdin_flush(void)
{
    int c;
    while((c = getchar()) != '\n' && c != EOF);
}

int main(int argc, char **argv)
{
    int exit = 0;
    char source[64];
    char pattern[64];
    char out[32], tmp;
    int pos;

    while(!exit) {
        fprintf(stdout, "source: ");
        fgets(source, 63, stdin);
        fprintf(stdout, "pattern: ");
        fgets(pattern, 63, stdin);
        putchar('\n');

        strip_space(source);
        strip_space(pattern);
        fprintf(stdout, "source: %s\n", source);
        fprintf(stdout, "pattern: %s\n", pattern);

        pos = kmp_search(source, pattern);
        if (pos < 0) strcpy(out, "\nNot matched!\n\n");
        else sprintf(out, "\nMatched at %d!\n\n", pos);
        fprintf(stdout, "%s", out);

        fprintf(stdout, "Continue?(Y/N): ");
        tmp = getchar(); // offer choice to continue;
        putchar('\n');
        if (tmp != 'Y' && tmp != 'y') {
            exit = 1;
        }
        stdin_flush();
    }
    return 0;
}


