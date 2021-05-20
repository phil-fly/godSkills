#ifndef __AKFS_RING_H__
#define __AKFS_RING_H__

#include <linux/kernel.h>
#include <linux/vmalloc.h>
#include <linux/slab.h>
#include <linux/mm.h>

#include "assertion.h"

typedef struct akfs_ring_s{
    unsigned int offset;
    unsigned int size;
    unsigned int in;
    unsigned int out;
}akfs_ring_t;


/**
 * @brief akfs_ring_alloc 
 *      ring结构体占用一个页，以后扩充用.
 */
akfs_ring_t *akfs_ring_init(unsigned int size);

/**
 * @brief akfs_ring_free 
 *      内核模块退出的时候释放内存
 */
void akfs_ring_exit(akfs_ring_t *ring);

/**
 * @brief akfs_ring_put 
 *      当内存映射到用户态的时候，初始地址是会变的。因此采用偏移的方式来和用户态同步数据。
 *      当前的场景是内存往ring里面写数据，用户态程序读数据，因此内核只提供put接口。
 *      之后需要考虑下直接从内核转发数据的情况，或者DMA等方式
 */
unsigned int __akfs_ring_put(akfs_ring_t *ring ,void *data ,unsigned int len);

void *akfs_ring_alloc(akfs_ring_t * ,int size);

void akfs_ring_free(akfs_ring_t * ,void *data);

#endif

