/* High-level overview: This works by maintaining a singly-linked list of previously-used memory segments that have been freed. */
// 高级概述:这是通过维护先前使用的已释放的内存段的单链表来工作的。

#define NALLOC 1024

#include <errno.h>
#include <limits.h>
#include <string.h>
#include <unistd.h>

/*
This header is stored at the beginning of memory segments in the list.
这个头存储在列表中内存段的开头。
*/
union header
{
  struct
  {
    union header *next;
    unsigned len;         // 记录当前区块下可用的单元块大小
  } meta;
  long x; /* Presence forces alignment of headers in memory. 存在强制对内存中的标头进行对齐。*/
};

static union header list;
static union header *first = NULL;

void free(void *ptr)
{
  if (ptr == NULL)
  {
    return;
  }
  union header *iter, *block;
  iter = first;
  block = (union header *)ptr - 1;

  /*
     Traverse to the spot in the list to insert the freed fragment,
     such that the list is ordered by memory address (for coalescing).
     遍历到列表中的位置以插入释放的片段，这样列表就按照内存地址排序(用于合并)。
  */
  while (block <= iter || block >= iter->meta.next)
  {
    if ((block > iter || block < iter->meta.next) && iter >= iter->meta.next)
    {
      break;
    }
    iter = iter->meta.next;
  }

  /* If the new fragment is adjacent in memory to any others, merge
   * them (we only have to check the adjacent elements because the
   * order semantics are enforced).
   * 如果新片段在内存中与任何其他片段相邻，则合并它们(我们只需要检查相邻元素，因为强制执行了顺序语义)。
   * */
  if (block + block->meta.len == iter->meta.next)
  {
    block->meta.len += iter->meta.next->meta.len;
    block->meta.next = iter->meta.next->meta.next;
  }
  else
  {
    block->meta.next = iter->meta.next;
  }
  
  if (iter + iter->meta.len == block)
  {
    iter->meta.len += block->meta.len;
    iter->meta.next = block->meta.next;
  }
  else
  {
    iter->meta.next = block;
  }

  first = iter;
}

void *ff_malloc(size_t size)
{
  union header *p, *prev;
  prev = first;
  // 实际分配的内存块大小
  unsigned true_size = (size + sizeof(union header) - 1) / sizeof(union header) + 1;
  
  /* If the list of previously allocated fragments is empty,
   * initialize it.
   */
  if (first == NULL)
  {
    prev = &list;
    first = prev;
    list.meta.next = first;
    list.meta.len = 0;
  }
  p = prev->meta.next;
  /* Traverse the list of previously allocated fragments, searching
   * for one sufficiently large to allocate.
   遍历这个之前分配过的内存块链表，寻找一个足够大去分配的块   
   */
  while (1)
  {
    if (p->meta.len >= true_size)
    {
      printf("User need %u size, but give %u size.",true_size*sizeof(union header), p->meta.len*sizeof(union header));
      if (p->meta.len == true_size) //  = true_size，直接返回
      {
        prev->meta.next = p->meta.next;
      }
      else // > true_size，
      {
        /* Otherwise, split the fragment, returning the first half and
         * storing the back half as another element in the list.
         前半部分留在链表中，而后半部分分配给用户
         p这个指针只是指向这个header节点，下面三行代码始终没有改变prev->meta->next，说明prev下一个节点还是在原p处
         */
        // 将原p处的长度更新，这样原header就更新完毕了，以后p再发生改变，和原来的header就没有关系了
        p->meta.len -= true_size;
        // 分配给用户内存的header处，将该处的长度设为true_size
        p += p->meta.len;
        p->meta.len = true_size;
      }
      first = prev;
      return (void *)(p + 1);
    }
    /* If we reach the beginning of the list, no satisfactory fragment
     * was found, so we have to request a new one.
     整个链表都没有适合大小的碎片，需要请求一个新的页
     请求完一个新页后继续遍历
     */
    if (p == first)
    {
      char *page;
      union header *block;
      unsigned alloc_size = true_size;
      /* We have to request memory of at least a certain size.
      */
      if (alloc_size < NALLOC)
      {
        alloc_size = NALLOC;
      }
      page = sbrk((intptr_t)(alloc_size * sizeof(union header)));
      if (page == (char *)-1)
      {
        /* There was no memory left to allocate. */
        errno = ENOMEM;
        return NULL;
      }
      /* Create a fragment from this new memory and add it to the list
       * so the above logic can handle breaking it if necessary. */
      block = (union header *)page;
      block->meta.len = alloc_size;
      free((void *)(block + 1));
      p = first;
    }
    // 继续遍历
    prev = p;
    p = p->meta.next;
  }
  return NULL;
}

void *calloc(size_t num, size_t len)
{
  void *ptr = ff_malloc(num * len);

  /* Set the allocated array to 0's.*/
  if (ptr != NULL)
  {
    memset(ptr, 0, num * len);
  }

  return ptr;
}
