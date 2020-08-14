#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

void
reverse(FILE *fpIn, FILE *fpOut)
{
    char **pArray = malloc(1024 * sizeof(char*));
    int count = 0;
    
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    while((nread = getline(&line, &len, fpIn)) != -1) {
        if(count >= 1024){
            fprintf(stderr, "输入文件行数太多，受不了了\n");
            exit(1);
        }
        
        pArray[count] = line;
        count++;
        
        // 如果不置NULL，则getline下一次不会重新分配内存，潜在的坑
        line = NULL;
    }
    
    for(int i=count-1;i>=0;i--){
        fprintf(fpOut, "%s", (char *)pArray[i]);
    }
    
    for(int i=0;i<count;i++){
        free(pArray[i]);
    }
}

// 检查俩文件是否指向同一个文件
int same_file(int fd1, int fd2) {
    struct stat stat1, stat2;
    if(fstat(fd1, &stat1) < 0) return -1;
    if(fstat(fd2, &stat2) < 0) return -1;
    return (stat1.st_dev == stat2.st_dev) && (stat1.st_ino == stat2.st_ino);
}

int
main(int argc, char *argv[])
{
    if(argc > 3){
        fprintf(stderr, "usage: reverse <input> <output>\n");
        exit(1);
    }
    if(argc == 3 && strcmp(argv[1],argv[2]) == 0){
        fprintf(stderr, "reverse: input and output file must differ\n");
        exit(1);
    }
    
    if(argc == 1){
        // \ as line change
        // Ctrl+D as end of input
        reverse(stdin, stdout);
    } else if (argc == 2){
        FILE *fp = fopen(argv[1], "r");
        if (fp == NULL) {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[1]);
            exit(1);
        }
        
        reverse(fp, stdout);
        
        fclose(fp);
    } else {
        FILE *fp1 = fopen(argv[1], "r");
        if (fp1 == NULL) {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[1]);
            exit(1);
        }
        
        FILE *fp2 = fopen(argv[2], "w");
        if (fp2 == NULL) {
            fprintf(stderr, "reverse: cannot open file '%s'\n", argv[2]);
            exit(1);
        }
        
        if(same_file(fileno(fp1), fileno(fp2)) > 0){
            fprintf(stderr, "reverse: input and output file must differ\n");
            exit(1);
        }
        
        reverse(fp1, fp2);
        
        fclose(fp1);
        fclose(fp2);
    }
    
    exit(0);
}


// gcc -o reverse reverse.c -Wall -Werror


