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
    
    struct ZippedItem item;
    while(fread(&item, sizeof(item), 1, fp) == 1){
        for(int i = 1; i <= item.length; i++){
            printf("%c", item.c);
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

