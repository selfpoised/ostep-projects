#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <ctype.h>

const char error_message[30] = "An error has occurred\n";

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

int main(int argc, char *argv[]) {
    
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    
    fprintf(stdout, "wish> ");
    while((nread = getline(&line, &len, stdin)) > 0) {
        if (strcmp(line, "exit\n") == 0)
        {
            exit(0);
        }
        
        int rc = fork();
        if (rc < 0) {
            // fork failed
            fprintf(stderr, "fork failed\n");
            exit(1);
        } else if (rc == 0) {
            // child: redirect standard output to a file
            // close(STDOUT_FILENO);
            // open("./ls.output", O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
            
            // support at most 10 parameters
            char *myargs[10];
            int count = 0;
            char *found;
            while( (found = strsep(&line," ")) != NULL ){
                if(count == 9){
                    write(STDERR_FILENO, error_message, strlen(error_message));
                    exit(1);
                }

                if(count == 0){
                    char path[256] = "/bin/";
                    strncat(path, trim(found), 250);
                    myargs[0] = strdup(path);
                    printf("%s ", myargs[0]);
                } else {
                    // https://blog.csdn.net/aaajj/article/details/53426767?utm_medium=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-4.channel_param&depth_1-utm_source=distribute.pc_relevant.none-task-blog-BlogCommendFromMachineLearnPai2-4.channel_param
                    // 大坑：数组参数不能有空格，换行等，否则死活执行不了。是因为输入最后一个参数后回车导致有换行符，必须trim
                    myargs[count] = strdup(trim(found));
                    printf("%s ", myargs[count]);
                }
                count++;
            }
            myargs[count] = NULL;
            printf("\n");
            
            execv(myargs[0], myargs);
        } else {
            // parent goes down this path (main)
            wait(NULL);
            fprintf(stdout, "wish> ");
        }
    }
    
    free(line);
}

// gcc -o wish wish.c -Wall -Werror
