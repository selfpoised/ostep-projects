#include "types.h"
#include "stat.h"
#include "user.h"
#include "mmu.h"

void start_routine(void *a1, void *a2);

int
main(int argc, char *argv[]) {
    int pid = thread_create(&start_routine, 0, 0);
    printf(1, "\nXV6_TEST_OUTPUT thread %d created\n", pid);

    for(int i=0; i<10; i++){
        printf(1, "\nXV6_TEST_OUTPUT parent pid=%d loop %d\n", getpid(), i);
    }
    int thread_pid = thread_join();
    printf(1, "\nXV6_TEST_OUTPUT parent pid=%d joined thread pid %d\n", pid, thread_pid);
}

void
start_routine(void *a1, void *a2)
{
    printf(1, "\nXV6_TEST_OUTPUT running start_routine in thread pid=%d\n", getpid());
}