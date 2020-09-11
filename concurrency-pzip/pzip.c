#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include "common_threads.h"
#include "io_helper.h"
#include <pthread.h>

/**
 * 本实现采用线程池处理输入文件方式，线程池竞争消费文件
 * 更合理的方式也许是将文件切分为一定的片段，然后再进行处理
 */

// check file content or final zip output
// od -c file
// od -c t.z

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

    int num_cores = sysconf(_SC_NPROCESSORS_ONLN);

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

    // in this stage, all input files are already zipped with *.z, just merge all and output to stdout
    char *pTemp = "tmp.z";
    FILE *fpTemp = fopen(pTemp, "w");
    if (fpTemp == NULL)
    {
        fprintf(stderr, "pzip: cannot open file %s\n", pTemp);
        exit(1);
    }
    for (int i = 1; i < argc; i++)
    {
        char *fromFileName = malloc(strlen(argv[i]) + 2 + 1);
        bzero(fromFileName, strlen(argv[i]) + 2 + 1);
        strcat(fromFileName, argv[i]);
        strcat(fromFileName, ".z");
        FILE *fp = fopen(fromFileName, "rb");
        if (fp == NULL)
        {
            fprintf(stderr, "pzip: cannot open file %s\n", fromFileName);
            exit(1);
        }
        free(fromFileName);

        int count = 0;
        while (fread(&count, sizeof(int), 1, fp) == 1)
        {
            int c = 0;
            fread(&c, 1, 1, fp);

            fwrite(&count, 4, 1, fpTemp);
            fprintf(fpTemp, "%c", c);
        }
        fclose(fp);
    }
    fclose(fpTemp);

    FILE *fp = fopen(pTemp, "rb");
    if (fp == NULL)
    {
        fprintf(stderr, "pzip: cannot open file %s\n", pTemp);
        exit(1);
    }
    int count = 0;
    int last_c = 0;
    int last_count = 0;
    while (fread(&count, sizeof(int), 1, fp) == 1)
    {
        int c = 0;
        fread(&c, 1, 1, fp);

        if (last_count == 0)
        {
            last_c = c;
            last_count = last_count + count;
        }
        else
        {
            if (c == last_c)
            {
                last_count = last_count + count;
            }
            else
            {
                fwrite(&last_count, 4, 1, stdout);
                fprintf(stdout, "%c", last_c);
                last_c = c;
                last_count = count;
            }
        }
    }
    if (last_count > 0)
    {
        fwrite(&last_count, 4, 1, stdout);
        fprintf(stdout, "%c", last_c);
    }
    fclose(fp);

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

        if (k > file_count)
        {
            break;
        }

        char *fileName = pArgv[k];
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
        if (k >= file_count)
        {
            done = 1;
        }
        Cond_signal(&cond);
        Mutex_unlock(&m);
    }

    return NULL;
}