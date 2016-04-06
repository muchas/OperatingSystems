#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#define BUF_SIZE 1024


int main(int argc, char *argv[])
{
    char *output;
    FILE *ls_fp, *grep_fp;
    char buffer[BUF_SIZE];

    if(argc != 2) {
        printf("Invalid number of arguments. Usage: <OUTPUT>\n");
        exit(EXIT_FAILURE);
    }

    output = strdup(argv[1]);

    if(output == NULL) {
        printf("Invalid string");
        exit(EXIT_FAILURE);
    }

    ls_fp = popen("ls -l", "r");
    freopen(output, "w", stdout);
    grep_fp = popen("grep ^d", "w");


    if(ls_fp == NULL || grep_fp == NULL) {
        perror("Popen error\n");
        exit(EXIT_FAILURE);
    }

    while (fgets(buffer, BUF_SIZE, ls_fp) != NULL) {
        fputs(buffer, grep_fp);
    }

    pclose(ls_fp);
    pclose(grep_fp);

    free(output);

    return 0;
}