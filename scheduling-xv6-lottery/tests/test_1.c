#include "types.h"
#include "stat.h"
#include "user.h"
#include "pstat.h"

int call_read(int times);

int
main(int argc, char *argv[]) {
  // process 1: tickets=30
  settickets(30);

  int rc = fork();
  if (rc < 0) { // fork failed; exit
     return 1;
   } else if (rc == 0) { // child (new process)
     // process 2: tickets=20
     settickets(20);
     int rc_0 = fork();
     if (rc_0 < 0)
     {
        return 1;
     } else if (rc_0 == 0) { // grand-child (new process)
        // process 3: tickets=10
        settickets(10);
        call_read(200000);
        return 0;
     } else {
     }
     call_read(200000);

     return 0;
   } else { // parent goes down this path (main)
   }

  call_read(200000);
  int x4 = getreadcount();

  int pid = getpid();
  printf(1, "XV6_TEST_OUTPUT pid %d\n\n", pid);

  struct pstat stat;
  getpinfo(&stat);
  for(int i=0; i<64;i++){
    //struct pstat {
    //  int inuse[NPROC];   // whether this slot of the process table is in use (1 or 0)
    //  int tickets[NPROC]; // the number of tickets this process has
    //  int pid[NPROC];     // the PID of each process
    //  int ticks[NPROC];   // the number of ticks each process has accumulated
    //};
    if(stat.inuse[i] != 0){
        printf(1, "XV6_TEST_OUTPUT proc %d %d %d\n", stat.pid[i], stat.tickets[i], stat.ticks[i]);
    }
  }
  printf(1, "\nXV6_TEST_OUTPUT read count %d\n", x4);
  exit();
}

int
call_read(int times)
{
    int i;
    char buf[100];
    for (i = 0; i < times; i++)
    {
        (void) read(4, buf, 1);
    }

    return 0;
}