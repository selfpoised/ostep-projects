#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>

const char error_message[30] = "An error has occurred\n";

char *trim(char *s);
void exec_single(char *line);
void exec_batch(char *file);

int main(int argc, char *argv[]) {
    // batch mode
    if(argc >= 2){
        // fprintf(stdout, "batch mode: \n");
        exec_batch(argv[1]);
        exit(0);
    }
    
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    
    fprintf(stdout, "wish> ");
    while((nread = getline(&line, &len, stdin)) > 0) {
        if (strcmp(trim(line), "exit\n") == 0 || strcmp(trim(line), "exit") == 0) {
            // use built-in exit
            char *myargs[2];
            myargs[0] = strdup("./exit");
            myargs[1] = NULL;
            execv(myargs[0], myargs);
        }
        
        // Parallel Commands: cmd1 & cmd2 args1 args2 & cmd3 args1
        if(strstr(line, "&") != NULL){
            // printf("%s\n", line);
            char *found;
            while( (found = strsep(&line,"&")) != NULL ){
                char *t = strdup(trim(found));
                exec_single(t);
            }
        } else {
            exec_single(line);
        }
        
        // parent goes down this path (main)
        wait(NULL);
        fprintf(stdout, "wish> ");
    }
    
    free(line);
}

// https://stackoverflow.com/questions/656542/trim-a-string-in-c
char *trim(char *s) {
    char *ptr;
    if (!s)
        return NULL;   // handle NULL string
    if (!*s)
        return s;      // handle empty string
    for (ptr = s + strlen(s) - 1; (ptr >= s) && isspace(*ptr); --ptr);
    ptr[1] = '\0';
    return s;
}

void exec_single(char *line){
    int rc = fork();
    if (rc < 0) {
        // fork failed
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        char *line_refine = trim(line);
        
        // support at most 10 parameters
        char *myargs[10];
        int count = 0;
        char *found;
        
        while( (found = strsep(&line_refine," ")) != NULL ){
            if(count == 9){
                write(STDERR_FILENO, error_message, strlen(error_message));
                exit(1);
            }
            
            // 跳过拆分过程中可能的空格
            if(strlen(trim(found)) == 0){
                continue;
            }

            if(count == 0){
                if(strcmp(trim(found), "cd") == 0){
                    myargs[0] = strdup("cd");
                } else {
                    char path[256] = "/bin/";
                    strncat(path, trim(found), 250);
                    myargs[0] = strdup(path);
                }
            } else {
                // redirection
                if(strcmp(trim(found), ">") == 0){
                    // The reason this redirection works is due to an assumption about how the operating
                    // system manages file descriptors. Specifically, UNIX systems start looking for free file descriptors at zero.
                    // In this case, STDOUT FILENO will be the first available one and thus get assigned when open() is called.
                    // Subsequent writes by the child process to the standard output file descriptor, for example by routines such
                    // as printf(), will then be routed transparently to the newly-opened file instead of the screen
                    close(STDOUT_FILENO);
                    
                    found = strsep(&line_refine," ");
                    open(trim(found), O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
                    
                    break;
                } else {
                    // https://blog.csdn.net/aaajj/article/details/53426767?utm_medium=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-4.channel_param&depth_1-utm_source=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-4.channel_param
                    // 大坑：数组参数不能有空格，换行等，否则死活执行不了。是因为输入最后一个参数后回车导致有换行符，必须trim
                    myargs[count] = strdup(trim(found));
                    // printf("%s ", myargs[count]);
                }
            }
            count++;
        }
        myargs[count] = NULL;
        
        execv(myargs[0], myargs);
    } else {
        return;
    }
}

void exec_batch(char *file){
    // support at most 10 parameters
    FILE *fp = fopen(file, "r");
    if (fp == NULL) {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
    
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    int count = 0;
    while((nread = getline(&line, &len, fp)) != -1) {
        count++;
        
        exec_single(line);
        
        wait(NULL);
    }
    free(line);
    
    fclose(fp);
}

// gcc -o wish wish.c -Wall -Werror
