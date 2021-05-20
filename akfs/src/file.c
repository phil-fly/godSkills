#include "akfs.h"

static int akfs_mmap_alloc(akfs_args_t *args);
static int default_open(struct inode *inode, struct file *file);
static ssize_t default_read_file(struct file *file, char __user *buf,
        size_t count, loff_t *ppos);
static ssize_t default_write_file(struct file *file, const char __user *buf,
        size_t count, loff_t *ppos);
static long default_ioctl(struct file *file, unsigned int cmd, unsigned long args);
static unsigned int default_poll(struct file *file, poll_table *wait);
static int default_mmap(struct file *file ,struct vm_area_struct *vma);
static int default_release(struct inode *inode, struct file *file);


/**
 * @brief akfs 默认支持的
 */
const struct file_operations akfs_file_operations = {
    .open           = default_open,
    .release        = default_release,
    .unlocked_ioctl = default_ioctl,
    .poll           = default_poll,
    .mmap           = default_mmap,
    .read           = default_read_file,
    .write          = default_write_file,
};

/**
 * @brief default_open 
 *   akfs open函数
 *   当chan状态不为AKFS_FSA_REQ时表示
 *   chan对于的文件以及被打开
 *   chan目前仅支持单例
 */
static int default_open(struct inode *inode, struct file *file)
{
    akfs_args_t *args = (akfs_args_t *)inode->i_private;

    assert_error(args && (args->otp.status & AKFS_FSA_WAIT) ,-EACCES);

    args->otp.status = AKFS_FSA_REQ;

    file->private_data = inode->i_private;

    return 0;
}

/**
 * @brief default_read_file 
 *   读写点 暂时未使用
*/
static ssize_t default_read_file(struct file *file, char __user *buf,
        size_t count, loff_t *ppos)
{
    return 0;
}

static ssize_t default_write_file(struct file *file, const char __user *buf,
        size_t count, loff_t *ppos)
{
    return count;
}

/**
 * @brief default_ioctl 
 *   ioctl点 处理相关指令
 */
static long default_ioctl(struct file *file, unsigned int cmd, unsigned long args)
{
    akfs_args_t *args_ptr = (akfs_args_t *)file->private_data;
    int ret = 0;

    assert_error(args_ptr ,-EACCES);

    switch(cmd)
    {
        case AKFS_IOCTL_MLEN:
            ret = akfs_get_mlen(args_ptr ,args);
            break;
        case AKFS_IOCTL_OTP:
            ret = akfs_otp_authentiaction(&args_ptr->otp ,args);
            break;
        default:
            ret = args_ptr->ioctl_handle ?
                (args_ptr->ioctl_handle(cmd ,args)) : -EACCES;
    }

    return ret;
}

/**
 * @brief default_poll 
 *   poll通知点
 */
static unsigned int default_poll(struct file *file, poll_table *wait)
{
    akfs_args_t *args = (akfs_args_t *)file->private_data;
    unsigned int mask = 0;

    //otp认证过了之后才可以使用
    assert_error(args && (args->otp.status & AKFS_FSA_ACCESS) ,-EACCES);

    poll_wait(file ,&args->waitq ,wait);

    //唤醒条件成立则返回
    //同时将唤醒条件重置
    if(args->condition){
        mask |= (POLLIN | POLLRDNORM);
        args->condition = 0;
    }

    return mask;
}

/**
 * @brief default_mmap 
 *   mmap点
 */
static int default_mmap(struct file *file ,struct vm_area_struct *vma)
{
    akfs_args_t *args = (akfs_args_t *)file->private_data;

    //otp认证过了才可以mmap
    assert_error(args && (args->otp.status & AKFS_FSA_ACCESS) ,-EACCES);

    //首次初始化ring
    assert_error(!akfs_mmap_alloc(args) ,-ENOMEM);

    //用户层映射的长度需要等于rlen的长度
    assert_error((vma->vm_end - vma->vm_start) == args->rlen ,-EACCES);

    return remap_vmalloc_range(vma ,args->ring ,0);
}

/**
 * @brief default_release 
 *   otp认证通过的进程关闭的时候重置otp状态
 */
static int default_release(struct inode *inode, struct file *file)
{
    akfs_args_t *args = (akfs_args_t *)file->private_data;

    assert_error(args ,-EACCES);
    args->otp.status = AKFS_FSA_WAIT;

    return 0;
}

/**
 * @brief akfs_mmap_alloc 
 *   初始化mmap的ring buffer
 *   仅初始化一次
 */
static int akfs_mmap_alloc(akfs_args_t *args)
{
    assert_error(!args->ring ,0);

    args->ring = akfs_ring_init(args->rlen);

    return args->ring ? 0 : -ENOMEM;
}

