#include "types.h"
#include "stat.h"
#include "user.h"
#include "mmu.h"

int
main(int argc, char *argv[]) {
    // for err 5,6,7 ref: https://chamilo.grenoble-inp.fr/courses/ENSIMAG4MMPCSEF/document/traps.pdf
    // Figure 5-7. Page-Fault Error Code

    // err 5: the access causing the fault was a write
    // trap 14 err 5 on cpu 0 eip 0xffffffff addr 0xffffffff--kill proc
    // without protection
    // int *p = (int *)main;
    // printf(1, "XV6_TEST_OUTPUT %d %d %d %d\n\n", main, 4096, 1, p);
    // *(p+4) = 2;

    // err 6: the fault was caused by a non-present page
    // trap 14 err 6 on cpu 0 eip 0x3c addr 0x28010--kill proc
    // mprotect(main, 2);
    // int *p = (int *)main+4096*10;
    // printf(1, "XV6_TEST_OUTPUT %d %d %d %d\n\n", main, 4096, 1, p);
    // *(p+4) = 2;

    // err 7: the access causing the fault was a write
    // trap 14 err 7 on cpu 0 eip 0x3c addr 0x10--kill proc
    mprotect(main, 2);
    int *p = (int *)main;
    printf(1, "XV6_TEST_OUTPUT %d %d %d %d\n\n", main, 4096, 1, p);
    *(p+4) = 2;

    // null dereference trial
    // int pi = 0;     
    // int a = 1;
    // int c = *pi; 
    // printf(1, "XV6_TEST_OUTPUT pi=%d c=%d\n\n", pi, c);
    // int i0 = mprotect(main, 2);
    // int i1 = mprotect((void *)PGROUNDDOWN((uint) &argv[0]), 1);
    // int i2 = munprotect((void *)PGROUNDDOWN((uint) &argv[0]), 2);
    // printf(1, "XV6_TEST_OUTPUT %d %d %d\n\n", PGROUNDDOWN((uint) &argv[0]), i1, i2);
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