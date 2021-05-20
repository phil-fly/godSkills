#ifndef __AKFS_IOCTL_H__
#define __AKFS_IOCTL_H__

#include <linux/io.h>

#define AKFS_IOCTL_MAGIC 'a'

//获取mmap映射长度
#define AKFS_IOCTL_MLEN _IO(AKFS_IOCTL_MAGIC ,0)

#endif
