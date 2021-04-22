#include "types.h"
#include "stat.h"
#include "user.h"
#include "mmu.h"

void start_routine(void *a1, void *a2);

int
main(int argc, char *argv[]) {
    int a1 = 5;
    char* s = "pee pee";
    int pid = thread_create(&start_routine, &a1, (void *)s);
    printf(1, "\nXV6_TEST_OUTPUT thread %d created\n", pid);

    // this means newly created thread is sharing mem with process that fork it
    a1 = 6;

    for(int i=0; i<10; i++){
        printf(1, "\nXV6_TEST_OUTPUT parent pid=%d loop %d\n", getpid(), i);
    }
    int thread_pid = thread_join();
    printf(1, "\nXV6_TEST_OUTPUT parent pid=%d joined thread pid %d\n", pid, thread_pid);
}

void
start_routine(void *a1, void *a2)
{
    printf(1, "\nXV6_TEST_OUTPUT running start_routine in thread pid=%d, content %d, %s\n", getpid(), *((int *)a1), (char *)a2);
}