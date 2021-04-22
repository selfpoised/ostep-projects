#include "types.h"
#include "stat.h"
#include "user.h"
#include "mmu.h"

void start_routine(void *a1, void *a2);

int
main(int argc, char *argv[]) {
    int pid = thread_create(&start_routine, 0, 0);
    printf(1, "\nXV6_TEST_OUTPUT thread %d\n", pid);

    for(int i=0; i<10; i++){
        printf(1, "\nXV6_TEST_OUTPUT pid=%d %d\n", getpid(), i);
    }
    int j_pid = thread_join();
    printf(1, "\nXV6_TEST_OUTPUT pid=%d joined_pid=%d\n", pid, j_pid);
}

void
start_routine(void *a1, void *a2)
{
    printf(1, "\n***in thread*** XV6_TEST_OUTPUT running in start_routine pid=%d\n", getpid());
}