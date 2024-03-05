/*
 * mm-naive.c - The fastest, least memory-efficient malloc package.
 * 
 * In this naive approach, a block is allocated by simply incrementing
 * the brk pointer.  A block is pure payload. There are no headers or
 * footers.  Blocks are never coalesced or reused. Realloc is
 * implemented directly using mm_malloc and mm_free.
 *
 * NOTE TO STUDENTS: Replace this header comment with your own header
 * comment that gives a high level description of your solution.
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>

#include "mm.h"
#include "memlib.h"

/*********************************************************
 * NOTE TO STUDENTS: Before you do anything else, please
 * provide your team information in the following struct.
 ********************************************************/
team_t team = {
    /* Team name */
    "ateam",
    /* First member's full name */
    "Harry Bovik",
    /* First member's email address */
    "bovik@cs.cmu.edu",
    /* Second member's full name (leave blank if none) */
    "",
    /* Second member's email address (leave blank if none) */
    ""
};

/* single word (4) or double word (8) alignment */
#define ALIGNMENT 8

/* rounds up to the nearest multiple of ALIGNMENT */
#define ALIGN(size) (((size) + (ALIGNMENT-1)) & ~0x7)


#define SIZE_T_SIZE (ALIGN(sizeof(size_t)))

// 操作节点的宏
// p 为一个已知地址的节点， 类型为 char *

#define GET_SIZE(p) (*(unsigned int *)(p) & 0x1fffffff) 
#define GET_ALLOC(p) ((*(unsigned int *)(p) & 0x80000000) >> 31) // 0 为未分配，1 为已分配
#define GET_PREV(p) (*(char * *)((p) + 4))
#define GET_NEXT(p) ((*(char * *)((p) + 8)))
#define GET_FOOTER(p) (*(unsigned int *)((p) + GET_SIZE((p)) - 4))
#define GET_SIZE_FROM_FOOTER(footer) (*(unsigned int *)(footer) & 0x1fffffff) // 从footer中取出size
#define GET_ALLOC_FROM_FOOTER(footer) ((*(unsigned int *)(footer) & 0x80000000) >> 31) // 从footer中取出alloc

// 从一个已知地址的节点中设置各种信息
#define SET_SIZE(p, size) (*(unsigned int *)(p) = ((size) | (0xe0000000 & *(unsigned int *)(p)))) // size 不应该超过 0x1fffffff
#define SET_ALLOC(p, alloc) (*(unsigned int *)(p) = ((*(unsigned int *)(p) & 0x7fffffff) | ((alloc) << 31))) // alloc 只能为 0 或 1
#define SET_PREV(p, prev) (*(char * *)((p) + 4) = (prev))
#define SET_NEXT(p, next) (*(char * *)((p) + 8) = (next))
#define SET_FOOTER(p) (*(unsigned int *)((p) + GET_SIZE((p)) - 4) = *(unsigned int *)(p)) // 将header的信息复制到footer

// p 为一个已知地址的节点， 类型为 char *
// 为了和prev以及next区分，我们将前一个节点称为left，后一个节点称为right

#define SET_RIGHT_NODE_WHEN_ALLOC(p) (*(unsigned int *)((p) + GET_SIZE((p))) = (*(unsigned int *)((p) + GET_SIZE((p))) | 0x20000000)) // 当一个节点被分配时，调用该宏将下一个节点的上一节点标志位设置为已分配
#define SET_RIGHT_NODE_WHEN_FREE(p) (*(unsigned int *)((p) + GET_SIZE((p))) = (*(unsigned int *)((p) + GET_SIZE(p)) & 0xdfffffff)) // 当一个节点被释放时，调用该宏将下一个节点的上一节点标志位设置为未分配

#define IS_LEFT_ALLOC(p) ((*(unsigned int *)((p)) & 0x20000000) >> 29)
#define IS_RIGHT_ALLOC(p) ((*(unsigned int *)((p) + GET_SIZE((p))) & 0x80000000) >> 31) // 检查下一节点是否被分配

#define GET_NODE_FROM_PAYLOAD(p) ((p) - 8) // 从有效载荷的首地址得到节点的首地址

// 判断是否为有效节点的最左边
#define IS_LEFTEST(p) ((p) == mem_heap_lo() + 112)

// 判断是否为堆的最右边
#define IS_RIGHTEST(p) ((p) + GET_SIZE((p)) == (mem_heap_hi() + 1))

// 便于给alloc赋值
#define TRUE 1
#define FALSE 0

static char **free_lists; // 用于存放空闲链表的数组
static const int group_size = 28;

static int binary_search(int size) {
    int left = 0;
    int right = group_size - 1;
    while (left <= right) {
        int mid = (left + right) / 2;
        if (size <= (1 << (mid + 4))) {
            right = mid - 1;
        } else {
            left = mid + 1;
        }
    }
    right = (right < 0) ? 0 : right; // 修正right为-1的情况
    return right;
}

#ifdef DEBUG
static int check(char *to_check){
    if (to_check == NULL) {
        return 1;
    }
    if (to_check < (char *)(mem_heap_lo() + 112) || to_check > (char *)(mem_heap_hi() + 1)) {
        return 0;
    }
    return 1;
}
#endif

static void delete_node(char *p, int group_index) {
    char *prev =  GET_PREV(p);
    char *next = GET_NEXT(p);

    #ifdef DEBUG
        if ((check(p) && check(prev) && check(next)) == 0) {
            printf("\nError Happened\n\n");
            printf("delete_node: check failed\n");
            printf("delete_node: p = %p, prev = %p, next = %p, nodesize = %d\n", p, prev, next, GET_SIZE(p));
            printf("current brk %p -> %p\n", mem_heap_lo(), mem_heap_hi());
            exit(0);
        }
        else
            printf("delete_node: p = %p, prev = %p, next = %p, nodesize = %d\n", p, prev, next, GET_SIZE(p));
    #endif

    // 前节点指向后节点
    if (prev == NULL) { //prev为空意味着这是链表的第一个节点
        free_lists[group_index] = next; // 将链表数组中的对应项设为next
    }
    else { // 不是第一项
        SET_NEXT(prev, next); // 将前节点指向后节点
    }

    // 后节点指向前节点
    if (next != NULL) {
        SET_PREV(next, prev); 
    }
}

static void insert_node(char *p, int group_index){
    #ifdef DEBUG
        if (check(p) == 0) {
            printf("insert_node: check failed\n");
            printf("insert_node: p = %p to group %d\n", p, group_index);
            exit(0);
        }
        else
            printf("insert_node: p = %p to group %d, nodesize = %d\n", p, group_index, GET_SIZE(p));
    #endif

    // 总是插入到链表的头部
    char *next = free_lists[group_index];
    free_lists[group_index] = p;
    SET_NEXT(p, next);
    SET_PREV(p, NULL);
    if (next != NULL) {
        SET_PREV(next, p);
    }
}

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    free_lists = mem_sbrk(sizeof(char *) * group_size);
    for (int i = 0; i < group_size; i++) {
        free_lists[i] = NULL;
    }
    return 0;
}

/* 
 * mm_malloc - Allocate a block
 */
void *mm_malloc(size_t size)
{
    if (size == 0) {
        fprintf(stderr, "mm_malloc: size = 0\n");
        exit(1);
    }

    int new_size = ALIGN(size) + 8;
    int res_index = 0;
    int is_find = FALSE;
    char *res = NULL;

    #ifdef DEBUG
        printf("mm_malloc: newsize = %d\n", new_size);
    #endif

    // 通过二分查找找到最大的小于要求的组的下表
    int start = binary_search(new_size);
    for (int i = start; i < group_size; i++) {
        if (free_lists[i] != NULL) {
            char *p = free_lists[i];
            while (p != NULL) {
                if (GET_SIZE(p) >= new_size) {
                    res = p;
                    res_index = i;
                    is_find = TRUE;
                    break;
                }
                p = GET_NEXT(p);
            }
        }
        if (is_find) {
            break;
        }
    }

    if (is_find == TRUE) { //找到后我们进行一些处理就可以把它交给用户了
        //从链表中删除节点
        delete_node(res, res_index); 

        // 如果得到的节点的大小比要求的大小大16字节（这表示我们至少还能分割出一个节点），则将其分割为两个节点
        // 而且我们保证newsize是向8对齐的，所以不用在意是否会出现不对齐的问题
        if (GET_SIZE(res) - new_size >= 16){
            int old_node_size = GET_SIZE(res);
            char *new_node;
            int new_node_size = old_node_size - new_size;

            new_node = res + new_size; // 在满足原有节点的情况下的新节点地址
            SET_SIZE(res, new_size);

            SET_SIZE(new_node, new_node_size); // 这两条设置头部
            SET_ALLOC(new_node, FALSE);      //
            SET_FOOTER(new_node);            // 将头部的内容复制到脚部
            SET_PREV(new_node, NULL);
            SET_NEXT(new_node, NULL);

            // 二分查找找到要插入的组
            int inseart_index = binary_search(new_node_size);
            insert_node(new_node, inseart_index);

            #ifdef DEBUG
                printf("mm_malloc: newsize = %d, nodesize = %d\n", new_size, GET_SIZE(res));
                printf("mm_malloc: split node %p (size = %d) into %p (size = %d) and %p (size = %d)\n", res, old_node_size, res, new_size, new_node, new_node_size);
            #endif
        }// 分割节点操作结束

        #ifdef DEBUG
            printf("mm_malloc: adress = %p ,newsize = %d, nodesize = %d, ", res, new_size, GET_SIZE(res));
            printf("status: alloc from free list\n");
        #endif
        // 处理分配信息
        SET_ALLOC(res, TRUE);
        SET_RIGHT_NODE_WHEN_ALLOC(res);

        return res + 8; // 返回有效载荷的首地址
    }
    else { //如果根本没有在池中找到可用的节点，调整堆指针，开辟一段新的空间
        char *new_space =  (char *)mem_sbrk(new_size);
        if ((int)new_space == -1) {
            return NULL;
        }

        SET_SIZE(new_space, new_size);
        SET_ALLOC(new_space, TRUE);
        SET_RIGHT_NODE_WHEN_ALLOC(new_space); // 在右侧节点根本没有存在的情况下去设置是否有一些问题。。。
        #ifdef DEBUG
            printf("mm_malloc: address = %p ,newsize = %d, nodesize = %d, ", new_space, new_size, GET_SIZE(new_space));
            printf("status: new alloc from heap\n");
        #endif

        return new_space + 8;
    }
}


static char * try_merge_free_node(char *node) {
    // 获取各种信息
    int size = GET_SIZE(node);
    int left_alloc = (IS_LEFTEST(node) == TRUE) ? TRUE : IS_LEFT_ALLOC(node); // 如果是最左边的节点，那么认为它的左节点是已分配的
    int right_alloc = (IS_RIGHTEST(node) == TRUE) ? TRUE : IS_RIGHT_ALLOC(node); // 如果是最右边的节点，那么认为它的右节点是已分配的

    char *new_node = NULL;

    if (left_alloc == FALSE && right_alloc == FALSE) { // 左右节点都空闲
        int left_node_size = GET_SIZE_FROM_FOOTER(node - 4);
        int right_node_size = GET_SIZE(node + size);

        char *left_node = node - left_node_size;
        char *right_node = node + size;

        int index = 0;

        #ifdef DEBUG
            printf("try_merge_free_node: left_node = %p, current_node = %p, right_node = %p\n", left_node, node, right_node);
        #endif

        // 从链表中删除左节点
        index = binary_search(left_node_size);
        delete_node(left_node, index);

        // 从链表中删除右节点
        index = binary_search(right_node_size);
        delete_node(right_node, index);

        // 合并节点
        SET_SIZE(left_node, left_node_size + size + right_node_size);
        SET_ALLOC(left_node, FALSE);
        SET_FOOTER(left_node);
        SET_RIGHT_NODE_WHEN_FREE(left_node);

        new_node = left_node;
    }
    else if (left_alloc == FALSE && right_alloc == TRUE) { // 左节点空闲，右节点已分配
        int left_node_size = GET_SIZE_FROM_FOOTER(node - 4);
        char *left_node = node - left_node_size;

        #ifdef DEBUG
            printf("try_merge_free_node: left_node = %p, current_node = %p\n", left_node, node);
        #endif

        // 从链表中删除左节点
        int index = binary_search(left_node_size);// 二分查找找到左节点所在的组
        delete_node(left_node, index);

        // 合并节点
        SET_SIZE(left_node, left_node_size + size);
        SET_ALLOC(left_node, FALSE);
        SET_FOOTER(left_node);
        SET_RIGHT_NODE_WHEN_FREE(left_node);

        new_node = left_node;
    }
    else if (left_alloc == TRUE && right_alloc == FALSE) { // 左节点已分配，右节点空闲
        int right_node_size = GET_SIZE(node + size);
        char *right_node = node + size;

        #ifdef DEBUG
            printf("try_merge_free_node: current_node = %p, right_node = %p\n", node, right_node);
        #endif

        // 从链表中删除右节点
        int index = binary_search(right_node_size);// 二分查找找到右节点所在的组
        delete_node(right_node, index);

        // 合并节点 
        SET_SIZE(node, size + right_node_size);
        SET_ALLOC(node, FALSE);
        SET_FOOTER(node);
        SET_RIGHT_NODE_WHEN_FREE(node);

        new_node = node;
    }else { // 左右节点都已分配
        // 不再递归尝试合并
        return node;
    }
    #ifdef DEBUG
        printf("try_merge_free_node: new_node = %p\n", new_node);
    #endif
    new_node = try_merge_free_node(new_node); // 递归尝试合并
    return new_node;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    #ifdef DEBUG
        printf("mm_free: ptr = %p\n", ptr);
    #endif
    char *node = GET_NODE_FROM_PAYLOAD(ptr);
    SET_ALLOC(node, FALSE);
    SET_FOOTER(node);
    SET_RIGHT_NODE_WHEN_FREE(node);

    // 尝试合并节点
    node = try_merge_free_node(node);
    SET_NEXT(node, NULL);
    SET_PREV(node, NULL);
    SET_RIGHT_NODE_WHEN_FREE(node);

    // 将节点插入到链表中
    int node_size = GET_SIZE(node);
    int index = binary_search(node_size);
    insert_node(node, index);

    #ifdef DEBUG
        printf("mm_free complete: group = %d, prev = %p, next = %p\n", index, GET_PREV(node), GET_NEXT(node));
    #endif
    return;
}

/*
 * mm_realloc - Implemented simply in terms of mm_malloc and mm_free
 */
void *mm_realloc(void *ptr, size_t size)
{
    void *oldptr = ptr;
    void *newptr;
    size_t copySize;

    newptr = mm_malloc(size);
    if (newptr == NULL)
        return NULL;
    copySize = *(size_t *)((char *)oldptr - SIZE_T_SIZE);
    if (size < copySize)
        copySize = size;
    memcpy(newptr, oldptr, copySize);
    mm_free(oldptr);
    return newptr;
}
