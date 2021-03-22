#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[]) {
    int *pi;     
    pi = 0;
    //int a = 1;
    int c = *pi; 
    printf(1, "XV6_TEST_OUTPUT pi=%d c=%d\n\n", pi, c);
}

// int
// main(int argc, char *argv[]) {
//     int pid = getpid();
//     printf(1, "XV6_TEST_OUTPUT pid %d\n\n", pid);

//     int a, b, c; // some integers
//     int *pi;     // a pointer to an integer

//     a = 5;
//     pi = &a; // pi points to a
//     b = *pi; // b is now 5
//     printf(1, "XV6_TEST_OUTPUT pid %d %d\n\n", pi, b);
//     pi = 0;
//     c = *pi; // this is a NULL pointer dereference
//     printf(1, "XV6_TEST_OUTPUT pid %d %d\n\n", pi, c);

//     exit();
// }