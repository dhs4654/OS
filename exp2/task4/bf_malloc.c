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
        unsigned len; 
    } meta;
    long x; /* Presence forces alignment of headers in memory.*/
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
        if ((block > iter || block < iter->meta.next) &&
            iter >= iter->meta.next)
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

void *bf_malloc(size_t size)
{
    union header *p, *prev;
    prev = first;
    unsigned true_size =
        (size + sizeof(union header) - 1) /
            sizeof(union header) +
        1;
    /* If the list of previously allocated fragments is empty,
     * initialize it.
     */
    if (first == NULL)
    {
        prev = &list;
        first = prev;
        // 首尾相连，形成一个环
        list.meta.next = first;
        list.meta.len = 0;
    }
    p = prev->meta.next;
    /* Traverse the list of previously allocated fragments, searching
     * for one sufficiently large to allocate.
     */

    int flag = 0;
    unsigned minSize = (unsigned)-1;

    while (1)
    {
        // 首次遍历
        if (flag == 0 && p->meta.len >= true_size)
        {
            if (p->meta.len == true_size)
            {
                prev->meta.next = p->meta.next;
                printf("User need %u size, but give %u size.\n",true_size*sizeof(union header), p->meta.len*sizeof(union header));
                return (void *)(p + 1);
            }
            else
            {
                if (p->meta.len < minSize)
                    minSize = p->meta.len;
            }
        }
        // 二次遍历
        if (flag == 1 && p->meta.len == minSize)
        {
            printf("User need %u size, but give %u size.\n",true_size*sizeof(union header), p->meta.len*sizeof(union header));
            if (p->meta.len == true_size)
            {
                prev->meta.next = p->meta.next;
            }
            else
            {
                p->meta.len -= true_size;
                p += p->meta.len;
                p->meta.len = true_size;
            }
            first = prev;
            return (void *)(p + 1);
        }

        /* If we reach the beginning of the list, no satisfactory fragment
         * was found, so we have to request a new one.
         */
        if (p == first)
        {
            if (minSize == (unsigned)-1)
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
                minSize = alloc_size;
            }
            flag = 1;
        }
        // 继续遍历
        prev = p;
        p = p->meta.next;
    }
    return NULL;
}

void *calloc(size_t num, size_t len)
{
    void *ptr = bf_malloc(num * len);

    /* Set the allocated array to 0's.*/
    if (ptr != NULL)
    {
        memset(ptr, 0, num * len);
    }

    return ptr;
}
