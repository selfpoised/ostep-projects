#include <stdio.h>
#include <stdlib.h>

void
wcat(FILE *fp)
{
    char buf[1024];
    while( fgets (buf, 1024, fp)!=NULL ) {
        printf("%s", buf);
    }
}

int
main(int argc, char *argv[])
{
    if(argc <= 1){
        exit(0);
    }
    
    int i;
    for(i = 1; i < argc; i++){
        FILE *fp = fopen(argv[i], "r");
        if (fp == NULL) {
            printf("wcat: cannot open file\n");
            exit(1);
        }
        wcat(fp);
        fclose(fp);
    }
    exit(0);
}


