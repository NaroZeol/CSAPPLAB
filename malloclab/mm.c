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

/* Define of data structure */
/* List Head Pointer */
static char **free_lists;
static const int groupSize = 28;
// Data Structure

static int binary_search(int size) {
    int left = 0;
    int right = groupSize - 1;
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

/* 
 * mm_init - initialize the malloc package.
 */
int mm_init(void)
{
    free_lists = mem_sbrk(sizeof(char *) * groupSize);
    for (int i = 0; i < groupSize; i++) {
        free_lists[i] = NULL;
    }
    return 0;
}

/* 
 * mm_malloc - Allocate a block by incrementing the brk pointer.
 *     Always allocate a block whose size is a multiple of the alignment.
 */
void *mm_malloc(size_t size)
{
    if (size == 0)
        return NULL;

    int newsize = ALIGN(size) + 8;
    int indexOfRes = 0;
    int is_find = FALSE;
    char *res = NULL;
    
    // 通过二分查找找到最大的小于要求的组的下表
    int start = binary_search(newsize);
    for (int i = start; i < groupSize; i++) {
        if (free_lists[i] != NULL) {
            char *p = free_lists[i];
            while (p != NULL) {
                if (GET_SIZE(p) >= newsize) {
                    res = p;
                    indexOfRes = i;
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
        // 将节点与原链表断绝关系
        char *pre = GET_PREV(res);
        char *next = GET_NEXT(res);
        // 与前节点断绝关系
        if (pre == NULL) {// 为了便于操作，我们将位于最上方的链表节点的前驱设置为NULL，这样就可以分辨是否是最上方节点
            free_lists[indexOfRes] = next;
        }else{
            SET_NEXT(pre, next);
        }
        // 与后节点断绝关系
        if (next != NULL) { // 如果本身就是最后一个节点，就什么都不用做
            SET_PREV(next, pre); 
        }

        // 如果得到的节点的大小比要求的大小大16字节（这表示我们至少还能分割出一个节点），则将其分割为两个节点
        // 而且我们保证newsize是向8对齐的，所以不用在意是否会出现不对齐的问题
        if (GET_SIZE(res) - newsize >= 16){
            #ifdef DEBUG
                printf("Try to split the node\n");
            #endif
            char *newNode;
            int newNodeSize = GET_SIZE(res) - newsize;

            newNode = res + newsize; // 在满足原有节点的情况下的新节点地址
            SET_SIZE(res, newsize);
            SET_ALLOC(res, TRUE);
            SET_RIGHT_NODE_WHEN_ALLOC(res);

            SET_SIZE(newNode, newNodeSize); // 这两条设置头部
            SET_ALLOC(newNode, FALSE);      //
            SET_FOOTER(newNode);            // 将头部的内容复制到脚部
            SET_PREV(newNode, NULL);
            SET_NEXT(newNode, NULL);

            // 二分查找找到要插入的组
            int inseartIndex = binary_search(newNodeSize);
            if (free_lists[inseartIndex] == NULL){ // 如果要插入的组为空组
                free_lists[inseartIndex] = newNode; // 直接设置free_list指向newNode，我们前面已经把prev和next设为NULL了
            }
            else {
                char *next = free_lists[inseartIndex]; // 不为空组，那么就插入到头部
                free_lists[inseartIndex] = newNode; // 将free_list直接指向newNode
                SET_PREV(next, newNode); // 将之前的头部节点的prev指向新节点
                SET_NEXT(newNode, next); // 将新节点的next指向原来的头部节点
            }  
        }// 分割节点操作结束

        return res + 8; // 返回有效载荷的首地址
    }
    else { //如果根本没有在池中找到可用的节点，调整堆指针，开辟一段新的空间
        char *newSpace =  (char *)mem_sbrk(newsize);
        if ((int)newSpace == -1) {
            return NULL;
        }

        SET_SIZE(newSpace, newsize);
        SET_ALLOC(newSpace, TRUE);
        SET_RIGHT_NODE_WHEN_ALLOC(newSpace); // 在右侧节点根本没有存在的情况下去设置是否有一些问题。。。

        return newSpace + 8;
    }
}


char * try_merge_free_node(char *node) {
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

        char *pre = NULL;
        char *next = NULL;
        int index = 0;

        // 从链表中删除左节点
        pre = GET_PREV(left_node);
        next = GET_NEXT(left_node);
        // 二分查找找到左节点所在的组
        index = binary_search(left_node_size);
        if (pre == NULL) {
            free_lists[index] = next;
        } else {
            SET_NEXT(pre, next);
        }
        if (next != NULL) {
            SET_PREV(next, pre);
        }

        // 从链表中删除右节点
        pre = GET_PREV(right_node);
        next = GET_NEXT(right_node);
        // 二分查找找到右节点所在的组
        index = binary_search(right_node_size);
        if (pre == NULL) {
            free_lists[index] = next;
        } else {
            SET_NEXT(pre, next);
        }
        if (next != NULL) {
            SET_PREV(next, pre);
        }

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

        // 从链表中删除左节点
        char *pre = GET_PREV(left_node);
        char *next = GET_NEXT(left_node);
        int index = binary_search(left_node_size);// 二分查找找到左节点所在的组
        if (pre == NULL) {
            free_lists[index] = next;
        } else {
            SET_NEXT(pre, next);
        }
        if (next != NULL) {
            SET_PREV(next, pre);
        }

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

        // 从链表中删除右节点
        char *pre = GET_PREV(right_node);
        char *next = GET_NEXT(right_node);
        int index = binary_search(right_node_size);// 二分查找找到右节点所在的组
        if (pre == NULL) {
            free_lists[index] = next;
        } else {
            SET_NEXT(pre, next);
        }
        if (next != NULL) {
            SET_PREV(next, pre);
        }
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
    new_node = try_merge_free_node(new_node);
    return new_node;
}

/*
 * mm_free - Freeing a block does nothing.
 */
void mm_free(void *ptr)
{
    char *node = GET_NODE_FROM_PAYLOAD(ptr);
    SET_ALLOC(node, FALSE);
    SET_FOOTER(node);
    SET_RIGHT_NODE_WHEN_FREE(node);

    // 尝试合并节点
    node = try_merge_free_node(node);
    SET_NEXT(node, NULL);
    SET_PREV(node, NULL);

    // 将节点插入到链表中
    int node_size = GET_SIZE(node);
    int index = binary_search(node_size);
    if (free_lists[index] == NULL){ // 如果要插入的组为空组
        free_lists[index] = node; // 直接设置free_list指向newNode，我们前面已经把prev和next设为NULL了
    }
    else {
        char *next = free_lists[index]; // 不为空组，那么就插入到头部
        free_lists[index] = node; // 将free_list直接指向newNode
        SET_PREV(next, node); // 将之前的头部节点的prev指向新节点
        SET_NEXT(node, next); // 将新节点的next指向原来的头部节点
    }

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
