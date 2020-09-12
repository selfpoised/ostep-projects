#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <pthread.h>
#include "common_threads.h"
#include "io_helper.h"
#include "mapreduce.h"
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>

/**
 * 多线程方式模拟mapreduce:
 * 
 * 1. map  phase
 *    thread creation: num_mappers
 *    mapper
 *    num_partitions = num_mappers
 * 
 * 2. sort phase: 本处实现放入reduce入口执行
 *    all num_partitions intermediate files
 * 
 * 3. reduce phase
 *    thread creation: num_reducers
 *    output to stdout
 */

// check file content
// od -c file

pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
pthread_mutex_t m = PTHREAD_MUTEX_INITIALIZER;

Mapper mapper = NULL;
Reducer reducer = NULL;
Partitioner partitioner = NULL;

// 中间文件数量，此处设置为mapper数量，避免多线程append file问题
int partions_count = 0;

// 用户输入文件数量
int file_count = 0;
// 用户输入文件处理计数
int count = 0;
// 任务进度控制
int done = 0;
// char *argv[]
char **pArgv = NULL;

// mapper处理后的中间文件前缀，intermediate.1 ～ intermediate.partions_count
char *INTERMEDIATE_PREFIX = "x_intermediate.";

#define get_inter_file(i) \
    ({ char *p = malloc(1024); bzero(p, 1024); sprintf(p, "%s%d", INTERMEDIATE_PREFIX, i); p; })

void *mapper_consumer(void *arg);
void *reducer_consumer(void *arg);

// 对文件升序排列
void file_sort(char *file);

// 从某个中间文件遍历特定的key
char *getter(char *key, int partition_number);
// partition_number+1长度数组
// 用于记录某个partiton中getter遍历某个key时候的文件位置
// 一个key遍历完结会重置
int *p_partion_getter_pos = NULL;

void MR_Emit(char *key, char *value)
{
    unsigned long i = partitioner(key, partions_count);
    char *p = get_inter_file((int)i);
    FILE *fp = fopen(p, "a");
    if (fp == NULL)
    {
        fprintf(stderr, "MR_Emit: cannot open file %s\n", p);
        exit(1);
    }
    free(p);
    fprintf(fp, "%s\t%s\n", key, value);
    fclose(fp);
}

unsigned long MR_DefaultHashPartition(char *key, int num_partitions)
{
    unsigned long hash = 5381;
    int c;
    while ((c = *key++) != '\0')
        hash = hash * 33 + c;
    return (hash % num_partitions) + 1;
}

void MR_Run(int argc, char *argv[],
            Mapper map, int num_mappers,
            Reducer reduce, int num_reducers,
            Partitioner partition)
{
    // 0: param check
    if (argc <= 1)
    {
        printf("MR_Run: file1 [file2 ...]\n");
        exit(1);
    }
    if (num_mappers <= 0)
    {
        printf("num_mappers should be positive\n");
        exit(1);
    }
    if (num_reducers <= 0)
    {
        printf("num_reducers should be positive\n");
        exit(1);
    }

    // 1: param set
    partitioner = partition;
    if (partitioner == NULL)
    {
        partitioner = MR_DefaultHashPartition;
    }
    mapper = map;
    reducer = reduce;
    pArgv = argv;
    file_count = argc - 1;
    partions_count = num_mappers;
    p_partion_getter_pos = (int *)malloc((partions_count + 1) * sizeof(int *));
    for (int t = 0; t <= partions_count; t++)
    {
        p_partion_getter_pos[t] = -1;
    }

    // 2: map phase
    for (int i = 0; i < num_mappers; i++)
    {
        pthread_t p;
        Pthread_create(&p, NULL, mapper_consumer, NULL);
    }
    // wait mapper
    Mutex_lock(&m);
    while (done < file_count)
    {
        Cond_wait(&cond, &m);
    }
    Mutex_unlock(&m);

    // 3. reduce phase
    done = 0;
    count = 0;
    for (int i = 0; i < num_reducers; i++)
    {
        pthread_t p;
        Pthread_create(&p, NULL, reducer_consumer, NULL);
    }
    // wait reducer
    Mutex_lock(&m);
    while (done < partions_count)
    {
        Cond_wait(&cond, &m);
    }
    Mutex_unlock(&m);
}

void *mapper_consumer(void *arg)
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
        mapper(fileName);

        Mutex_lock(&m);
        done++;
        Cond_signal(&cond);
        Mutex_unlock(&m);
    }

    return NULL;
}

void *reducer_consumer(void *arg)
{
    while (1)
    {
        int k = 0;
        Mutex_lock(&m);
        count++;
        k = count;
        Mutex_unlock(&m);
        if (k > partions_count)
        {
            break;
        }

        // 得到中间文件名：mapper处理的结果
        char *fileName = get_inter_file(k);
        file_sort(fileName);

        struct stat fileStat;
        if (stat(fileName, &fileStat) < 0 || fileStat.st_size <= 0)
        {
            //
        }
        else
        {
            // reduce处理
            FILE *fp = fopen(fileName, "r");
            assert(fp != NULL);

            char *line = NULL;
            size_t size = 0;
            char *last_key = calloc(1024, sizeof(char));
            while (getline(&line, &size, fp) != -1)
            {
                char *token, *dummy = line;
                token = strsep(&dummy, " \t\n\r");
                if (strcmp(token, last_key) != 0)
                {
                    reducer(token, getter, k);

                    bzero(last_key, 1024);
                    strcpy(last_key, token);
                }
            }
            free(line);
            free(last_key);
            fclose(fp);
        }
        free(fileName);

        Mutex_lock(&m);
        done++;
        Cond_signal(&cond);
        Mutex_unlock(&m);
    }

    return NULL;
}

void file_sort(char *file)
{
    struct stat fileStat;
    if (stat(file, &fileStat) < 0)
    {
        // fprintf(stderr, "stat %s failed\n", file);
        return;
    }

    int ret = 0;
    int length = 1024;
    char *command = (char *)calloc(length, sizeof(char));
    if (fileStat.st_size <= 0)
    {
        bzero(command, 1024);
        sprintf(command, "/bin/mv %s.s %s", file, file);
        ret = system(command);
        while (ret == -1 || ret == 127)
        {
            ret = system(command);
        }
        free(command);
        return;
    }

    sprintf(command, "/usr/bin/sort %s > %s.s", file, file);

    ret = system(command);
    while (ret == -1 || ret == 127)
    {
        ret = system(command);
    }

    bzero(command, 1024);
    sprintf(command, "/bin/rm %s", file);
    ret = system(command);
    while (ret == -1 || ret == 127)
    {
        ret = system(command);
    }

    bzero(command, 1024);
    sprintf(command, "/bin/mv %s.s %s", file, file);
    ret = system(command);
    while (ret == -1 || ret == 127)
    {
        ret = system(command);
    }

    free(command);
}

char *getter(char *key, int partition_number)
{
    int i = partition_number;
    char *p = get_inter_file(partition_number);

    FILE *fp = fopen(p, "r");
    assert(fp != NULL);

    char *line = NULL;
    size_t size = 0;

    char *value = NULL;
    if (p_partion_getter_pos[i] != -1)
    {
        fseek(fp, p_partion_getter_pos[i], SEEK_SET);
        if (getline(&line, &size, fp) != -1)
        {
            char *token, *dummy = line;
            token = strsep(&dummy, " \t\n\r");
            if (strcmp(token, key) == 0)
            {
                p_partion_getter_pos[i] = ftell(fp);
                token = strsep(&dummy, " \t\n\r");

                // todo: 考虑固定长度内存，不要每次分配
                value = calloc(strlen(token) + 1, sizeof(char));
                strcpy(value, token);
            }
            else
            {
                value = NULL;
                p_partion_getter_pos[i] = -1;
            }
        }
        else
        {
            value = NULL;
            p_partion_getter_pos[i] = -1;
        }
    }
    else
    {
        while (getline(&line, &size, fp) != -1)
        {
            char *token, *dummy = line;
            token = strsep(&dummy, " \t\n\r");
            if (strcmp(token, key) == 0)
            {
                p_partion_getter_pos[i] = ftell(fp);
                token = strsep(&dummy, " \t\n\r");
                value = calloc(strlen(token) + 1, sizeof(char));
                strcpy(value, token);
                break;
            }
        }
    }
    fclose(fp);
    free(line);

    if (value == NULL)
    {
        p_partion_getter_pos[i] = -1;
    }

    return value;
}

char *ltrim(char *s)
{
    while (isspace(*s))
        s++;
    return s;
}

char *rtrim(char *s)
{
    char *back = s + strlen(s);
    while (isspace(*--back))
        ;
    *(back + 1) = '\0';
    return s;
}

char *trim(char *s)
{
    return rtrim(ltrim(s));
}

void Map(char *file_name)
{
    FILE *fp = fopen(file_name, "r");
    assert(fp != NULL);

    char *line = NULL;
    size_t size = 0;
    while (getline(&line, &size, fp) != -1)
    {
        char *token, *dummy = line;
        while ((token = strsep(&dummy, " \t\n\r")) != NULL)
        {
            if (strlen(trim(token)) > 0)
            {
                MR_Emit(token, "1");
            }
        }
    }
    free(line);
    fclose(fp);
}

void Reduce(char *key, Getter get_next, int partition_number)
{
    int count = 0;
    char *value;
    while ((value = get_next(key, partition_number)) != NULL)
        count++;
    printf("%s %d\n", key, count);
}

int main(int argc, char *argv[])
{
    // ./mapreduce a t
    MR_Run(argc, argv, Map, 10, Reduce, 10, MR_DefaultHashPartition);
    return 0;
}