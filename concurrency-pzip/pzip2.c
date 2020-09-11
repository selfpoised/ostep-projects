#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

void
wzip(char * c, long size)
{
    // printf("%s\n", c);
    
    int i;
    int per_count = 1;
    for(i = 0; i < size; i++){
        if(i == (size-1)){
            fwrite(&per_count, 4, 1, stdout);
            printf("%c", c[i]);
            break;
        }
    
        if(c[i] != c[i+1]){
            // printf("%d%c", per_count, c[i]);
            fwrite(&per_count, 4, 1, stdout);
            printf("%c", c[i]);
            per_count = 1;
        } else {
            per_count++;
        }
    }
}

int
main(int argc, char *argv[])
{
    if(argc <= 1){
        printf("pzip: file1 [file2 ...]\n");
        exit(1);
    }
    
    clock_t tic = clock();

    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    fprintf( stderr, "This system has %d processors\n", num_cores);
    // fprintf( stderr, "This system has %d processors configured and %d processors available.\n", get_nprocs_conf(), get_nprocs());

    int i;
    
    // 得到总的文件大小，好一次性分配内存，此法糙快猛，嘿嘿
    long total_size = 0;
    for(i = 1; i < argc; i++){
        FILE *fp = fopen(argv[i], "rb");
        if (fp == NULL) {
            printf("pzip: cannot open file\n");
            exit(1);
        }
        fseek(fp, 0, SEEK_END);
        long fsize = ftell(fp);
        total_size = total_size + fsize;
        fclose(fp);
    }
    
    char *original_content = malloc(total_size + 1);
    
    long temp = 0;
    for(i = 1; i < argc; i++){
        FILE *fp = fopen(argv[i], "rb");
        if (fp == NULL) {
            printf("pzip: cannot open file\n");
            exit(1);
        }
        
        fseek(fp, 0, SEEK_END);
        long fsize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char *file_content = malloc(fsize + 1);
        fread(file_content, 1, fsize, fp);
        fclose(fp);
        
        strncpy(original_content + temp, file_content, fsize);
        temp = temp + fsize;
        
        free(file_content);
    }
    
    original_content[total_size] = 0;
    wzip(original_content, total_size);
    
    free(original_content);
    
    clock_t toc = clock();
    fprintf( stderr, "Elapsed: %f seconds\n", (double)(toc - tic) / CLOCKS_PER_SEC);

    exit(0);
}


// gcc -o pzip pzip.c -Wall -Werror
// gcc -o opzip pzip.c -Wall -Werror -O