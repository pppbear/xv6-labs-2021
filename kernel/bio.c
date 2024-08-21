// Buffer cache.
//
// The buffer cache is a linked list of buf structures holding
// cached copies of disk block contents.  Caching disk blocks
// in memory reduces the number of disk reads and also provides
// a synchronization point for disk blocks used by multiple processes.
//
// Interface:
// * To get a buffer for a particular disk block, call bread.
// * After changing buffer data, call bwrite to write it to disk.
// * When done with the buffer, call brelse.
// * Do not use the buffer after calling brelse.
// * Only one process at a time can use a buffer,
//     so do not keep them longer than necessary.


#include "types.h"
#include "param.h"
#include "spinlock.h"
#include "sleeplock.h"
#include "riscv.h"
#include "defs.h"
#include "fs.h"
#include "buf.h"

#define NBUCKETS 13  // 哈希桶数量
#define HASH(blockno) (blockno % NBUCKETS)

struct {
  struct spinlock lock;   // used for the buf alloc and size
  struct buf buf[NBUF];
  int size;     
  struct buf buckets[NBUCKETS];
  struct spinlock locks[NBUCKETS];
  struct spinlock hashlock;     
  // Linked list of all buffers, through prev/next.
  // Sorted by how recently the buffer was used.
  // head.next is most recent, head.prev is least.
} bcache;

void
binit(void)
{
  int i;
  struct buf* b;

  bcache.size = 0; 
  initlock(&bcache.lock, "bcache");
  initlock(&bcache.hashlock, "bcache_hash");    
  for (i = 0; i < NBUCKETS; ++i) {
    initlock(&bcache.locks[i], "bcache_bucket");
  }
  for (b = bcache.buf; b < bcache.buf + NBUF; b++) {
    initsleeplock(&b->lock, "buffer");
  }
}

// Look through buffer cache for block on device dev.
// If not found, allocate a buffer.
// In either case, return locked buffer.
static struct buf*
bget(uint dev, uint blockno)
{
  struct buf* b;
  int idx = HASH(blockno);
  struct buf* pre, * minb = 0, * minpre;
  uint mintimestamp;
  int i;

  // 查找块缓冲区
  acquire(&bcache.locks[idx]);  // lab8-2
  for (b = bcache.buckets[idx].next; b; b = b->next) {
    if (b->dev == dev && b->blockno == blockno) {
      b->refcnt++;
      release(&bcache.locks[idx]);  // lab8-2 
      acquiresleep(&b->lock);
      return b;
    }
  }

  // Not cached.
  // 检查是否有未使用的缓冲区 lab8-2 
  acquire(&bcache.lock);
  if (bcache.size < NBUF) {
    b = &bcache.buf[bcache.size++];
    release(&bcache.lock);
    b->dev = dev;
    b->blockno = blockno;
    b->valid = 0;
    b->refcnt = 1;
    b->next = bcache.buckets[idx].next;
    bcache.buckets[idx].next = b;
    release(&bcache.locks[idx]);
    acquiresleep(&b->lock);
    return b;
  }
  release(&bcache.lock);
  release(&bcache.locks[idx]);

  // 选择最近最少使用的缓冲区
  // 模仿LRU算法
  acquire(&bcache.hashlock);
  for (i = 0; i < NBUCKETS; ++i) {
    mintimestamp = -1;
    acquire(&bcache.locks[idx]);
    for (pre = &bcache.buckets[idx], b = pre->next; b; pre = b, b = b->next) {
      // research the block
      if (idx == HASH(blockno) && b->dev == dev && b->blockno == blockno) {
        b->refcnt++;
        release(&bcache.locks[idx]);
        release(&bcache.hashlock);
        acquiresleep(&b->lock);
        return b;
      }
      if (b->refcnt == 0 && b->timestamp < mintimestamp) {
        minb = b;
        minpre = pre;
        mintimestamp = b->timestamp;
      }
    }
    // find an unused block
    if (minb) {
      minb->dev = dev;
      minb->blockno = blockno;
      minb->valid = 0;
      minb->refcnt = 1;
      // if block in another bucket, we should move it to correct bucket
      if (idx != HASH(blockno)) {
        minpre->next = minb->next;    // remove block
        release(&bcache.locks[idx]);
        idx = HASH(blockno);  // the correct bucket index
        acquire(&bcache.locks[idx]);
        minb->next = bcache.buckets[idx].next;    // move block to correct bucket
        bcache.buckets[idx].next = minb;
      }
      release(&bcache.locks[idx]);
      release(&bcache.hashlock);
      acquiresleep(&minb->lock);
      return minb;
    }
    release(&bcache.locks[idx]);
    if (++idx == NBUCKETS) {
      idx = 0;
    }
  }
  panic("bget: no buffers");
}
// Return a locked buf with the contents of the indicated block.
struct buf*
bread(uint dev, uint blockno)
{
  struct buf *b;

  b = bget(dev, blockno);
  if(!b->valid) {
    virtio_disk_rw(b, 0);
    b->valid = 1;
  }
  return b;
}

// Write b's contents to disk.  Must be locked.
void
bwrite(struct buf* b)
{
  if (!holdingsleep(&b->lock))
    panic("bwrite");

  virtio_disk_rw(b, 1);
}
// Release a locked buffer.
// Move to the head of the most-recently-used list.
extern uint ticks;  // lab8-2

void
brelse(struct buf* b)
{
  int idx;
  if (!holdingsleep(&b->lock))
    panic("brelse");

  releasesleep(&b->lock);

  idx = HASH(b->blockno);// 计算哈希值
  // 在释放缓冲区锁之前先获取哈希桶锁
  acquire(&bcache.locks[idx]);
  // 修改引用计数和时间戳
  b->refcnt--;
  if (b->refcnt == 0) {
    b->timestamp = ticks;
  }

  release(&bcache.locks[idx]);
}
void
bpin(struct buf* b) {
  int idx = HASH(b->blockno);
  acquire(&bcache.locks[idx]);
  b->refcnt++;
  release(&bcache.locks[idx]);
}

void
bunpin(struct buf* b) {
  int idx = HASH(b->blockno);
  acquire(&bcache.locks[idx]);
  b->refcnt--;
  release(&bcache.locks[idx]);
}


