#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  exit(n);
  return 0; // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return wait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int n;

  argint(0, &n);
  addr = myproc()->sz;
  if (growproc(n) < 0)
    return -1;
  return addr;
}

uint64
sys_sleep(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n)
  {
    if (killed(myproc()))
    {
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

uint64
sys_kill(void)
{
  int pid;

  argint(0, &pid);
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

uint64
sys_waitx(void)
{
  uint64 addr, addr1, addr2;
  uint wtime, rtime;
  argaddr(0, &addr);
  argaddr(1, &addr1); // user virtual memory
  argaddr(2, &addr2);
  int ret = waitx(addr, &wtime, &rtime);
  struct proc *p = myproc();
  if (copyout(p->pagetable, addr1, (char *)&wtime, sizeof(int)) < 0)
    return -1;
  if (copyout(p->pagetable, addr2, (char *)&rtime, sizeof(int)) < 0)
    return -1;
  return ret;
}

uint64 sys_getreadcount(void)
{
  return myproc()->read_counter;      //get readcount reference : https://gist.github.com/bridgesign/e932115f1d58c7e763e6e443500c6561
}
uint64 sys_sigalarm(void)       // reference : https://xiayingp.gitbook.io/build_a_os/labs/lab-6-alarm 
{
  uint64 adr_of_funcn;
  int interval_argument;

  argint(0, &interval_argument);    //0 means first argument of sigalarm system call and stores it in interval_Argument variable
  argaddr(1, &adr_of_funcn);        // 1 means 2nd argument of sigalarm system call and stores it in adr_of_funcn

  struct proc * myprocess=myproc();   // pointer to access current process data
  myprocess->alarm_on=0;     //indicates alarm is right now not active
  myprocess->ticks=interval_argument;   
  myprocess->cur_ticks=0;
  myprocess->handler=adr_of_funcn; 
  return 0;
}

uint64 sys_sigreturn(void)        // reference : https://xiayingp.gitbook.io/build_a_os/labs/lab-6-alarm 
{
  struct proc *myprocess = myproc();
  memmove(myprocess->trapframe, myprocess->alarm_tf, PGSIZE); //copies sigalarm trapframe to process trapframe copying this saved state back to the trapframe, the process resumes execution from where it was interrupted by the alarm.

  kfree(myprocess->alarm_tf); //deallocates the memory 
  myprocess->alarm_tf = 0;
  myprocess->alarm_on = 0;
  myprocess->cur_ticks = 0;
  return myprocess->trapframe->a0;
}

