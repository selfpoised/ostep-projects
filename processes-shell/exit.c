#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

const char error_message[30] = "An error has occurred\n";

int main(int argc, char* argv[])
{
    if(argc > 1){
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
    
    exit(0);
}

// gcc -o exit exit.c -Wall -Werror



