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
void my_exit();
void check_command(char *command);
char *prepare_command(char *command);

// https://man7.org/linux/man-pages/man7/environ.7.html
// program environments:
//    COMMAND_MODE=unix2003
//    PATH=/opt/local/bin:/opt/local/sbin:/usr/local/bin:...
//    HOME=/Users/wanghao18
//    SHELL=/bin/zsh
//    ...
//  int i = 0;
//  while(environ[i] != NULL){
//    printf("%s\n", environ[i]);
//    i++;
//  }
extern char **environ;

int main(int argc, char *argv[]) {
    int r = setenv("PATH", "/bin", 1);
    if(r != 0){
        write(STDERR_FILENO, error_message, strlen(error_message));
    }
    
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
            my_exit();
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
            if(strcmp(trim(found), "cd") == 0 || strcmp(trim(found), "exit") == 0 || strcmp(trim(found), "path") == 0){
                myargs[0] = strdup(trim(found));
            } else {
//                char path[1024];
//                bzero(path, 1024);
//                char *p = getenv("PATH");
//                strncat(path, p, strlen(p));
//                strncat(path + strlen(p), "/", 1);
//                strncat(path + strlen(p) + 1, trim(found), strlen(trim(found)));
//                myargs[0] = strdup(path);
                
                char *p = prepare_command(trim(found));
                myargs[0] = p;
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
            }
        }
        count++;
    }
    myargs[count] = NULL;
    
    if(strcmp(myargs[0], "cd") == 0){
        // https://indradhanush.github.io/blog/writing-a-unix-shell-part-2/
        
        // The current working directory of the parent has not changed,
        // since the command was executed in the child. As a result,
        // the cd command although successful, did not produce the result that we desired.
        // Thus, to support cd we will have to implement it on our own. We also need to ensure that,
        // if the command entered by the user is cd (or belongs to a list of pre-defined built-in commands),
        // we will not fork the process at all. Instead, we will execute our implementation of cd (or any other built-in)
        // and move on to wait for the next user input. For cd, thankfully we have the chdir function call available to us
        // and using it is straightforward. It accepts the path as an argument and returns 0 upon success and -1 upon a failure
        if (count == 1 || chdir(myargs[1]) < 0) {
            write(STDERR_FILENO, error_message, strlen(error_message));
        }
        return;
    } else if(strcmp(myargs[0], "exit") == 0){
        if (count > 1) {
            write(STDERR_FILENO, error_message, strlen(error_message));
        } else {
            my_exit();
        }
    } else if(strcmp(myargs[0], "path") == 0){
        if (count == 1) {
            write(STDERR_FILENO, error_message, strlen(error_message));
        } else {
            // set program path
            char path[2048];
            bzero(path, 2048);
            int bytes = 0;
    
            strncpy(path+bytes, "/bin:", strlen("/bin:"));
            bytes = bytes + strlen("/bin:");
            for(int i=1;i<count;i++){
                strncpy(path+bytes, myargs[i], strlen(myargs[i]));
                bytes = bytes + strlen(myargs[i]);
                strncpy(path+bytes+1, ":", 1);
                bytes = bytes + 1;
            }
            if(path[strlen(path)-1] == ':'){
                path[strlen(path)-1] = 0;
            }
        
            int r = setenv("PATH", path, 1);
            if(r != 0){
                write(STDERR_FILENO, error_message, strlen(error_message));
            }
        }
    } else {
        // check path
        check_command(myargs[0]);
        
        int rc = fork();
        if (rc < 0) {
            // fork failed
            fprintf(stderr, "fork failed\n");
            exit(1);
        } else if (rc == 0) {
            execv(myargs[0], myargs);
        } else {
            return;
        }
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

void my_exit(){
    char *myargs[2];
    myargs[0] = strdup("./exit");
    myargs[1] = NULL;
    execv(myargs[0], myargs);
}

// 根据/bin以及用户设置的路径查找命令绝对路径
char *prepare_command(char *command){
    char *found;
    
    char *path = getenv("PATH");
    char *p = malloc(1024);
    while((found = strsep(&path,":")) != NULL){
        bzero(p, 1024);
        strncpy(p, found, strlen(found));
        strncpy(p+strlen(found), "/", 1);
        strncpy(p+strlen(found)+1, command, strlen(command));
        int i = access(p, X_OK);
        if(i == 0){
            return p;
        }
    }
    
    write(STDERR_FILENO, error_message, strlen(error_message));
    exit(0);
}

void check_command(char *command){
    // ./... 当前路径
//    if(command[0] == '.'){
//        return;
//    }
    
//    char path[2048];
//    bzero(path, 2048);
//    strncpy(path, "/bin/", strlen("/bin/"));
//    strncpy(path+strlen("/bin/"), command, strlen(command));
    // printf("%s\n", command);
    // printf("path: %s\n", getenv("PATH"));
    int i = access(command, X_OK);
    if(i != 0){
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(0);
    }
}

// gcc -o wish wish.c -Wall -Werror
