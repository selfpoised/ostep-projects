#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct ZippedItem {
    int   length;
    char  c;
};

void
wunzip(char * filename)
{
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL) {
        printf("wunzip: cannot open file\n");
        exit(1);
    }
    
    int count = 0;
    while(fread(&count, sizeof(int), 1, fp) == 1){
        int c = 0;
        fread(&c, 1, 1, fp);
        for(int i = 1; i <= count; i++){
            printf("%c", (char)c);
        }
    }
    
    fclose(fp);
}

int
main(int argc, char *argv[])
{
    if(argc <= 1){
        printf("wunzip: file1 [file2 ...]\n");
        exit(1);
    }
    
    for(int i = 1; i < argc; i++){
        wunzip(argv[i]);
    }
    
    exit(0);
}

// gcc -o wunzip wunzip.c -Wall -Werror

