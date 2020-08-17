#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/wait.h>

int main(int argc, char* argv[])
{
        char* arg[] = {"aa", " ", NULL};
        int ret = execv("/bin/ls", arg);
                                                                    
        printf("ret = %d", ret);
        return 0;
}

int main1(int argc, char* argv[])
{
        char* arg[] = {"/var", NULL};
        int ret = execv("/bin/ls", arg);
                                                                    
        printf("ret = %d", ret);
        return 0;
}

// gcc -o t t.c -Wall -Werror


