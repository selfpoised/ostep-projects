#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "common_threads.h"
#include "io_helper.h"
#include <pthread.h>

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

void *consume(void *arg);

int file_count = 0;
int count = 0;
int done = 0;
char **pArgv = NULL;

void pzip(char *c, long size, FILE *fp)
{
    int i;
    int per_count = 1;
    for (i = 0; i < size; i++)
    {
        if (i == (size - 1))
        {
            fwrite(&per_count, 4, 1, fp);
            fprintf(fp, "%c", c[i]);
            break;
        }

        if (c[i] != c[i + 1])
        {
            fwrite(&per_count, 4, 1, fp);
            fprintf(fp, "%c", c[i]);
            per_count = 1;
        }
        else
        {
            per_count++;
        }
    }
}

int main(int argc, char *argv[])
{
    pArgv = argv;

    if (argc <= 1)
    {
        printf("pzip: file1 [file2 ...]\n");
        exit(1);
    }

    file_count = argc - 1;
    count = 0;

    clock_t tic = clock();

    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);
    fprintf(stderr, "This system has %d processors\n", num_cores);

    // 线程池子数量取决于核数与待压缩文件数量
    int num_thread = 1;
    if ((argc - 1) >= num_cores)
    {
        num_thread = num_cores;
    }
    else
    {
        num_thread = argc - 1;
    }
    fprintf(stderr, "thread pool size %d\n", num_thread);
    for (int i = 0; i < num_thread; i++)
    {
        pthread_t p;
        Pthread_create(&p, NULL, consume, argv[i]);
    }

    Mutex_lock(&m);
    while (done == 0)
    {
        Cond_wait(&cond, &m);
    }
    Mutex_unlock(&m);

    clock_t toc = clock();
    fprintf(stderr, "Elapsed: %f seconds\n", (double)(toc - tic) / CLOCKS_PER_SEC);

    exit(0);
}

void *consume(void *arg)
{
    while (1)
    {
        int k = 0;
        Mutex_lock(&m);
        count++;
        k = count;
        Mutex_unlock(&m);

        if(k > file_count){
            break;
        }

        char *fileName = pArgv[k];
        printf("\n####thread %li %s\n", (unsigned long int)pthread_self(), fileName);

        FILE *fp = fopen(fileName, "rb");
        if (fp == NULL)
        {
            fprintf(stderr, "pzip: cannot open file %s\n", fileName);
            exit(1);
        }

        char *toFileName = malloc(strlen(fileName) + 2 + 1);
        strcat(toFileName, fileName);
        strcat(toFileName, ".z");
        FILE *fp1 = fopen(toFileName, "w");
        if (fp1 == NULL)
        {
            fprintf(stderr, "pzip: cannot open file %s\n", toFileName);
            exit(1);
        }
        free(toFileName);

        fseek(fp, 0, SEEK_END);
        long fsize = ftell(fp);
        fseek(fp, 0, SEEK_SET);

        char *file_content = malloc(fsize + 1);
        fread(file_content, 1, fsize, fp);
        fclose(fp);

        pzip(file_content, fsize, fp1);

        fclose(fp1);

        free(file_content);

        Mutex_lock(&m);
        if(k >= file_count){
            done = 1;
        }
        Cond_signal(&cond);
        Mutex_unlock(&m);
    }

    return NULL;
}