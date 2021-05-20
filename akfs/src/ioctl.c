#include "akfs.h"

/**
 * @brief akfs_get_mlen 
 *   获取映射长度接口
 */
int akfs_get_mlen(akfs_args_t *args_ptr ,unsigned long args){
    return copy_to_user((void __user *)args ,&args_ptr->rlen ,sizeof(unsigned int));
}
