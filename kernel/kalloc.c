// Physical memory allocator, for user processes,
// kernel stacks, page-table pages,
// and pipe buffers. Allocates whole 4096-byte pages.

#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "riscv.h"
#include "defs.h"

void freerange(void *pa_start, void *pa_end);

extern char end[]; // first address after kernel.
                   // defined by kernel.ld.

struct run {
  struct run *next;
};

struct {
  struct spinlock lock;
  struct run* freelist;
  uint8 ref_count[(PHYSTOP - KERNBASE) / PGSIZE];
} kmem;

void
increment_refcount(uint64 pa) {
  acquire(&kmem.lock);
  kmem.ref_count[(pa - KERNBASE) / PGSIZE]++;
  release(&kmem.lock);
}

int
get_refcount(uint64 pa){
  return kmem.ref_count[(pa - KERNBASE) / PGSIZE];
}

void
kinit()
{
  initlock(&kmem.lock, "kmem");
  freerange(end, (void*)PHYSTOP);
}

void
freerange(void* pa_start, void* pa_end)
{
  char* p;
  p = (char*)PGROUNDUP((uint64)pa_start);
  for (; p + PGSIZE <= (char*)pa_end; p += PGSIZE) {
    acquire(&kmem.lock);
    // 设置物理页面的引用计数为1，表示该页面已经被分配一次
    kmem.ref_count[((uint64)p - KERNBASE) / PGSIZE] = 1;
    release(&kmem.lock);
    kfree(p);
  }
}
// Free the page of physical memory pointed at by v,
// which normally should have been returned by a
// call to kalloc().  (The exception is when
// initializing the allocator; see kinit above.)
void
kfree(void* pa)
{
  struct run* r;

  if (((uint64)pa % PGSIZE) != 0 || (char*)pa < end || (uint64)pa >= PHYSTOP)
    panic("kfree");

  acquire(&kmem.lock);
  // 如果引用计数为0，则释放该页面
  if (--kmem.ref_count[((uint64)pa - KERNBASE) / PGSIZE] == 0) {
    release(&kmem.lock);
    // 用特定值填充整个页面内存
    memset(pa, 1, PGSIZE);

    r = (struct run*)pa;

    acquire(&kmem.lock);
    r->next = kmem.freelist;
    kmem.freelist = r;
    release(&kmem.lock);
  }
  else
    release(&kmem.lock);
}

// Allocate one 4096-byte page of physical memory.
// Returns a pointer that the kernel can use.
// Returns 0 if the memory cannot be allocated.
void*
kalloc(void)
{
  struct run* r;
  acquire(&kmem.lock);
  r = kmem.freelist;
  if (r) {
    // 当从空闲列表中分配一个页面时，设置该页面的引用计数为1  
    kmem.ref_count[((uint64)r - KERNBASE) / PGSIZE] = 1;
    // 将空闲列表指针移动到下一个空闲页面
    kmem.freelist = r->next;
  }
  release(&kmem.lock);

  if (r)
    memset((char*)r, 5, PGSIZE); // fill with junk
  return (void*)r;
}
