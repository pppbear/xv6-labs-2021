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
  pagetable_t pagetable = 0;     // ��ǰ���̵�ҳ��
  unsigned int procmask = 0;     // ���ڼ�¼��Щҳ���ѱ����ʹ���λ����
  pte_t* pte;                    
  struct proc* p = myproc();    
  // ��ȡϵͳ���ò����������м��
  if (argaddr(0, &base) < 0 || argint(1, &len) < 0 || argaddr(2, &mask) < 0)
    return -1;
  // ��������ҳ������������λ�����ܹ���ʾ�ķ�Χ�����Ƴ���
  if (len > sizeof(int) * 8)
    len = sizeof(int) * 8;
  // ��������Ҫ����ҳ��
  for (int i = 0; i < len; i++) {
    pagetable = p->pagetable;  
    // ��������ַ�Ƿ񳬹��������������ַ��Χ
    if (base >= MAXVA)
      panic("pgaccess");
    // ����ҳ��㼶���Ӷ��㵽��Ͳ㣩
    for (int level = 2; level > 0; level--) {
      pte = &pagetable[PX(level, base)];  // ��ȡ��ǰ�㼶ҳ����
      if (*pte & PTE_V) {  // ���ҳ�����Ƿ���Ч
        pagetable = (pagetable_t)PTE2PA(*pte);  // ��ȡ��һ��ҳ��������ַ
      }
      else {
        return -1; 
      }
    }
    // ��ȡ��Ͳ�ҳ����
    pte = &pagetable[PX(0, base)];
    if (pte == 0)
      return -1;  // ���ҳ����ָ��Ϊ�գ����ش���
    if ((*pte & PTE_V) == 0)
      return -1;  // ���ҳ������Ч�����ش���
    if ((*pte & PTE_U) == 0)
      return -1;  // ���ҳ������û��ռ��У����ش���
    // ���ҳ����ķ���λ��PTE_A��
    if (*pte & PTE_A) {
      procmask = procmask | (1L << i);  // ����ѷ��ʣ�����Ӧ��λ����Ϊ1
      *pte = *pte & (~PTE_A);           // �������λ��׼�������һ�η���
    }
    base += PGSIZE;  // �ƶ�����һ��ҳ��
  }
  pagetable = p->pagetable;  // ���»�ȡ��ǰ���̵�ҳ��
  // �����ɵ�λ���뿽�����û��ռ�
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
