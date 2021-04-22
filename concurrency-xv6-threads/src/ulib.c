#include "types.h"
#include "stat.h"
#include "fcntl.h"
#include "user.h"
#include "x86.h"
#include "mmu.h"

char*
strcpy(char *s, const char *t)
{
  char *os;

  os = s;
  while((*s++ = *t++) != 0)
    ;
  return os;
}

int
strcmp(const char *p, const char *q)
{
  while(*p && *p == *q)
    p++, q++;
  return (uchar)*p - (uchar)*q;
}

uint
strlen(const char *s)
{
  int n;

  for(n = 0; s[n]; n++)
    ;
  return n;
}

void*
memset(void *dst, int c, uint n)
{
  stosb(dst, c, n);
  return dst;
}

char*
strchr(const char *s, char c)
{
  for(; *s; s++)
    if(*s == c)
      return (char*)s;
  return 0;
}

char*
gets(char *buf, int max)
{
  int i, cc;
  char c;

  for(i=0; i+1 < max; ){
    cc = read(0, &c, 1);
    if(cc < 1)
      break;
    buf[i++] = c;
    if(c == '\n' || c == '\r')
      break;
  }
  buf[i] = '\0';
  return buf;
}

int
stat(const char *n, struct stat *st)
{
  int fd;
  int r;

  fd = open(n, O_RDONLY);
  if(fd < 0)
    return -1;
  r = fstat(fd, st);
  close(fd);
  return r;
}

int
atoi(const char *s)
{
  int n;

  n = 0;
  while('0' <= *s && *s <= '9')
    n = n*10 + *s++ - '0';
  return n;
}

void*
memmove(void *vdst, const void *vsrc, int n)
{
  char *dst;
  const char *src;

  dst = vdst;
  src = vsrc;
  while(n-- > 0)
    *dst++ = *src++;
  return vdst;
}

extern void* malloc(uint);
extern void free(void*);

int 
thread_create(void (*start_routine)(void *, void *), void *arg1, void *arg2)
{
  void *stack = malloc((uint)PGSIZE);
  if(stack == 0)
  {
    return -1;
  }

  int pid = clone(start_routine, arg1, arg2, stack);
  return pid;
}

// An int thread_join() call should also be created, 
// which calls the underlying join() system call, frees the user stack, 
// and then returns. It returns the waited-for PID (when successful), -1 otherwise
int 
thread_join()
{
  void *p_stack = malloc((uint)4);
  *(uint *)p_stack = 0;
  void **pp_stack = &p_stack;

  printf(1, "thread stack to be freed %d %d before\n", pp_stack, *pp_stack);
  int pid = join(pp_stack);
  printf(1, "thread stack to be freed %d\n", *(uint *)p_stack);
  if(*(uint *)p_stack != 0){
    free((void *) *(uint *)p_stack);
  }
  free(p_stack);
  return pid;
}

// C program to demonstrate pointer to pointer
// https://www.geeksforgeeks.org/double-pointer-pointer-pointer-c/
// https://stackoverflow.com/questions/897366/how-do-pointer-to-pointers-work-in-c-and-when-might-you-use-them
// int fn()
// {
//     int var = 789;
  
//     // pointer for var
//     int *ptr2;
  
//     // double pointer for ptr2
//     int **ptr1;
  
//     // storing address of var in ptr2
//     ptr2 = &var;
      
//     // Storing address of ptr2 in ptr1
//     ptr1 = &ptr2;
      
//     // Displaying value of var using
//     // both single and double pointers
//     cprintf("Value of var = %d\n", var );
//     cprintf("Value of var using single pointer = %d\n", *ptr2 );
//     cprintf("Value of var using double pointer = %d\n", **ptr1);
    
//   return 0;
// } 