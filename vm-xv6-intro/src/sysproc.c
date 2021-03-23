#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "pstat.h"

extern int inner_getpinfo(struct pstat *p);

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_settickets()
{
    int tickets;

    if(argint(0, &tickets) < 0)
        return -1;

    if(tickets < 1)
        return -1;

    cprintf("sys_settickets tickets is %d \n", tickets);

    myproc()->tickets = tickets;
    return 0;
}

int
sys_getpinfo()
{
    struct pstat *p;
    if(argptr(0, (void*)&p, sizeof(struct pstat)) < 0)
        return -1;

    inner_getpinfo(p);

    return 0;
}


int 
sys_mprotect(void)
{
  void *addr;
  int len;
  if(argptr(0, (void*)&addr, sizeof(void *)) < 0)
     return -1;
  if(argint(1, &len) < 0 || len <= 0)
     return -1;
  if(((uint)addr) % PGSIZE != 0)
    return -1;

  struct proc* pr = myproc();

  // check range
  for(int j=0; j<len; j++)
  {
    char *temp = addr + j * PGSIZE;

    pde_t *pde;
    pte_t *pgtab;
    pde = &(pr->pgdir[PDX(temp)]);
    if(*pde & PTE_P)
    {
      pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
      pte_t *pte = &pgtab[PTX(temp)];
      if(*pte & PTE_P)
      {
        continue;
      }
      else
      {
        return -1;
      }
    }
    else
    {
      return -1;
    }
  }

  // change page table entry to readonly
  for(int j=0; j<len; j++)
  {
    char *temp = addr + j * PGSIZE;

    pde_t *pde;
    pte_t *pgtab;
    pde = &(pr->pgdir[PDX(temp)]);
    pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
    pte_t *pte = &pgtab[PTX(temp)];
    cprintf("sys_mprotect %d %d %d\n", j, *pte, temp);
    *pte = *pte & (~PTE_W);
    cprintf("sys_mprotect %d %d %d\n", j, *pte, ~PTE_W);
  }
  lcr3(V2P(pr->pgdir));
  cprintf("sys_mprotect called %d %d \n", addr, len);

  return 0;
}

int sys_munprotect(void)
{
  void *addr;
  int len;
  if(argptr(0, (void*)&addr, sizeof(void *)) < 0)
     return -1;
  if(argint(1, &len) < 0 || len <= 0)
     return -1;
  if(((uint)addr) % PGSIZE != 0)
    return -1;

  struct proc* pr = myproc();

  // check range
  for(int j=0; j<len; j++)
  {
    char *temp = addr + j * PGSIZE;

    pde_t *pde;
    pte_t *pgtab;
    pde = &(pr->pgdir[PDX(temp)]);
    if(*pde & PTE_P)
    {
      pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
      pte_t *pte = &pgtab[PTX(temp)];
      if(*pte & PTE_P)
      {
        continue;
      }
      else
      {
        return -1;
      }
    }
    else
    {
      return -1;
    }
  }

  // change page table entry to readonly
  for(int j=0; j<len; j++)
  {
    char *temp = addr + j * PGSIZE;

    pde_t *pde;
    pte_t *pgtab;
    pde = &(pr->pgdir[PDX(temp)]);
    pgtab = (pte_t*)P2V(PTE_ADDR(*pde));
    pte_t *pte = &pgtab[PTX(temp)];
    cprintf("sys_munprotect %d %d %d\n", j, *pte, temp);
    *pte = *pte | PTE_W;
    cprintf("sys_munprotect %d %d %d\n", j, *pte, PTE_W);
  }
  lcr3(V2P(pr->pgdir));
  cprintf("sys_munprotect called %d %d \n", addr, len);

  return 0;
}