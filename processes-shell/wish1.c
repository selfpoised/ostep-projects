#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

const char error_message[30] = "An error has occurred\n";

int main(int argc, char *argv[]) {
    
    char *line = NULL;
    size_t len = 0;
    ssize_t nread;
    
    fprintf(stdout, "wish1> ");
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
            
            // support at most 4 parameters
            char *myargs[3];
            myargs[0] = strdup("/bin/ls");
            myargs[1] = strdup("/var");
            myargs[2] = NULL;
            execv(myargs[0], myargs);
        } else {
            // parent goes down this path (main)
            wait(NULL);
            fprintf(stdout, "wish1> ");
        }
    }
    
    free(line);
}

// gcc -o wish1 wish1.c -Wall -Werror

