#include "types.h"
#include "riscv.h"
#include "param.h"
#include "defs.h"
#include "date.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"


uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
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
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
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

uint64
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


#ifdef LAB_PGTBL
int
sys_pgaccess(void)
{
  uint64 base;      
  uint64 mask;      
  int len;          
  pagetable_t pagetable = 0;     // 当前进程的页表
  unsigned int procmask = 0;     // 用于记录哪些页面已被访问过的位掩码
  pte_t* pte;                    
  struct proc* p = myproc();    
  // 获取系统调用参数，并进行检查
  if (argaddr(0, &base) < 0 || argint(1, &len) < 0 || argaddr(2, &mask) < 0)
    return -1;
  // 如果请求的页面数量超过了位掩码能够表示的范围，限制长度
  if (len > sizeof(int) * 8)
    len = sizeof(int) * 8;
  // 遍历所有要检查的页面
  for (int i = 0; i < len; i++) {
    pagetable = p->pagetable;  
    // 检查虚拟地址是否超过允许的最大虚拟地址范围
    if (base >= MAXVA)
      panic("pgaccess");
    // 遍历页表层级（从顶层到最低层）
    for (int level = 2; level > 0; level--) {
      pte = &pagetable[PX(level, base)];  // 获取当前层级页表项
      if (*pte & PTE_V) {  // 检查页表项是否有效
        pagetable = (pagetable_t)PTE2PA(*pte);  // 获取下一层页表的物理地址
      }
      else {
        return -1; 
      }
    }
    // 获取最低层页表项
    pte = &pagetable[PX(0, base)];
    if (pte == 0)
      return -1;  // 如果页表项指针为空，返回错误
    if ((*pte & PTE_V) == 0)
      return -1;  // 如果页表项无效，返回错误
    if ((*pte & PTE_U) == 0)
      return -1;  // 如果页表项不在用户空间中，返回错误
    // 检查页表项的访问位（PTE_A）
    if (*pte & PTE_A) {
      procmask = procmask | (1L << i);  // 如果已访问，将对应的位设置为1
      *pte = *pte & (~PTE_A);           // 清除访问位，准备检测下一次访问
    }
    base += PGSIZE;  // 移动到下一个页面
  }
  pagetable = p->pagetable;  // 重新获取当前进程的页表
  // 将生成的位掩码拷贝到用户空间
  return copyout(pagetable, mask, (char*)&procmask, sizeof(unsigned int));
}
#endif

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
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
