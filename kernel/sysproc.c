#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"
#include "vm.h"

uint64
sys_interpose(void)
{
  int mask;
  char upath[MAXPATH];
  argint(0, &mask);
  argstr(1, upath, MAXPATH);

  struct proc *p = myproc();
  p->deny_mask = (uint64)mask;

  // If user provided a valid path (not empty and not "-"), copy it manually
  if (upath[0] != '\0' && !(upath[0] == '-' && upath[1] == '\0')) {
    int i = 0;
    while (i < MAXPATH - 1 && upath[i] != '\0') {
      p->allow_path[i] = upath[i];
      i++;
    }
    p->allow_path[i] = '\0';
  } else {
    // Otherwise, clear the allowed path (means "no special allowance")
    p->allow_path[0] = '\0';
  }

  return 0;
}


uint64
sys_exit(void)
{
  int n;
  argint(0, &n);
  kexit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return kfork();
}

uint64
sys_wait(void)
{
  uint64 p;
  argaddr(0, &p);
  return kwait(p);
}

uint64
sys_sbrk(void)
{
  uint64 addr;
  int t;
  int n;

  argint(0, &n);
  argint(1, &t);
  addr = myproc()->sz;

  if (t == SBRK_EAGER || n < 0) {
    if (growproc(n) < 0)
      return -1;
  } else {
    // Lazy allocation
    if (addr + n < addr)
      return -1;
    myproc()->sz += n;
  }

  return addr;
}

uint64
sys_pause(void)
{
  int n;
  uint ticks0;

  argint(0, &n);
  if (n < 0)
    n = 0;

  acquire(&tickslock);
  ticks0 = ticks;
  while (ticks - ticks0 < n) {
    if (killed(myproc())) {
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
  return kkill(pid);
}

uint64
sys_uptime(void)
{
  uint xticks;
  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

