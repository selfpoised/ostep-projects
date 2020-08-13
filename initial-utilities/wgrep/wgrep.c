#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void
wgrep(char* word, FILE *fp)
{
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    while((nread = getline(&line, &len, fp)) != -1) {
        if (word[0] == '\0')
        {
            printf("%s", line);
        } else{
            if(strstr(line, word) != NULL){
                printf("%s", line);
            }
        }
    }
    
    free(line);
}

int
main(int argc, char *argv[])
{
    if(argc <= 1){
        printf("wgrep: searchterm [file ...]\n");
        exit(1);
    }
    
    if(argc == 2){
        char buf[1024];
        while( fgets(buf, 1024, stdin) != NULL ) {
            if(strstr(buf, argv[1]) != NULL){
                printf("%s", buf);
            }
        }
        exit(0);
    }
    
    int i;
    for(i = 2; i < argc; i++){
        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL) {
            printf("wgrep: cannot open file\n");
            exit(1);
        }
        wgrep(argv[1], fp);
        fclose(fp);
    }
    exit(0);
}


// gcc -o wgrep wgrep.c -Wall -Werror

