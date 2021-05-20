/**
 * @brief  
 *  Ring the realization of the algorithm from linux2.6.32 / kernel/kfifo.c
 *  This version is the best implementation!
 */

#include "ring.h"

/**
 * @brief akfs_ring_alloc 
 *      ring结构体占用一个页，以后扩充用.
 */
akfs_ring_t *akfs_ring_init(unsigned int size)
{
    akfs_ring_t *ring = NULL;

    ring = vmalloc_user(size);
    assert_goto(ring ,out ,);

    ring->size = size - PAGE_SIZE;

    ring->offset = PAGE_SIZE;

    ring->in = ring->out = 0;

out:
    return ring;
}

/**
 * @brief akfs_ring_free 
 *      内核模块退出的时候释放内存
 */
void akfs_ring_exit(akfs_ring_t *ring)
{
    assert_void(!ring ,vfree(ring));
}

/**
 * @brief akfs_ring_put 
 *      当内存映射到用户态的时候，初始地址是会变的。因此采用偏移的方式来和用户态同步数据。
 *      当前的场景是内存往ring里面写数据，用户态程序读数据，因此内核只提供put接口。
 *      之后需要考虑下直接从内核转发数据的情况，或者DMA等方式
 */
unsigned int __akfs_ring_put(akfs_ring_t *ring ,void *data ,unsigned int len)
{
    unsigned int l;
    unsigned char *buffer = NULL;

    assert_error(likely(ring) ,-ENOMEM);

    buffer = (unsigned char *)ring + ring->offset;

    //拷贝的长度最大就是ring的size大小，超过了实在是不能够的
    len = min(len ,ring->size - ring->in + ring->out);

    smp_mb();

    l = min(len, ring->size - (ring->in & (ring->size - 1)));
    memcpy(buffer + (ring->in & (ring->size - 1)), data, l);

    memcpy(buffer ,data + l ,len - l);

    smp_wmb();

    ring->in += len;

    return len;
}

/**
 * @brief akfs_ring_alloc 
 *   申请一块ring内存
 */
void *akfs_ring_alloc(akfs_ring_t *ring ,int size){
    return kzalloc(size * PAGE_SIZE ,GFP_KERNEL);
}

/**
 * @brief akfs_ring_free 
 *   释放一块内存
 */
void akfs_ring_free(akfs_ring_t *ring ,void *data){
    kfree(data);
}
