#ifndef __AKFS_H__
#define __AKFS_H__

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/pagemap.h>
#include <linux/init.h>
#include <linux/kobject.h>
#include <linux/namei.h>
#include <linux/fsnotify.h>
#include <linux/string.h>
#include <linux/seq_file.h>
#include <linux/parser.h>
#include <linux/magic.h>
#include <linux/slab.h>
#include <linux/atomic.h>
#include <linux/device.h>
#include <linux/file.h>
#include <linux/poll.h>
#include <uapi/asm/ioctls.h>

#include "ring.h"
#include "otp.h"
#include "hook.h"
#include "dpath.h"

#define AKFS_DEFAULT_MODE   0644

#ifndef AKFS_MAGIC
#define AKFS_MAGIC          0x9180922
#endif

/**
 * @brief akfs权限
 */
enum {
    Opt_uid,
    Opt_gid,
    Opt_mode,
    Opt_err
};

/**
 * @brief akfs支持的mount参数 
 */
struct akfs_mount_opts {
    kuid_t uid;
    kgid_t gid;
    umode_t mode;
};

struct akfs_fs_info {
    struct akfs_mount_opts mount_opts;
};

/**
 * @brief fops的私有参数
 */
typedef struct akfs_args_s{
    akfs_otp_t otp;
    int (*ioctl_handle)(unsigned int cmd, unsigned long args);
    wait_queue_head_t waitq;
    unsigned int condition;
    unsigned int interrupt;
    unsigned int enable;
    unsigned int rlen;
    akfs_ring_t *ring;
    void *reserve;
}akfs_args_t;

/**
 * @brief channel结构体
 */
typedef struct akfs_init_s{
    struct dentry *dir;
    akfs_args_t args;
#define a_ring args.ring
#define a_lrn args.rlen
#define a_waitq args.waitq
#define a_condition args.condition
#define a_interrupt args.interrupt
}akfs_channel_t;

/**
 * @brief 模块hook section的首尾地址
 */
typedef struct akfs_hook_s{
    akfs_trace_t *start;
    akfs_trace_t *end;
}akfs_hook_t;

/**
 * @brief akfs的模块定义
 */
typedef struct akfs_module_s{
    const char *name;
    akfs_channel_t chan;
#define c_dir chan.dir 
#define c_args chan.args
#define c_ring chan.args.ring
#define c_rlen chan.args.rlen
#define c_otp_seed chan.args.otp.seed
#define c_otp_status chan.args.otp.status
    akfs_hook_t hook;
#define h_start hook.start
#define h_end hook.end
}akfs_module_t;

/**
 * @brief akfs操作函数
 */
typedef struct akfs_operation_s{
    void (*init)(akfs_module_t *,const char *,unsigned int ,
            unsigned long int ,unsigned long int);
    int (*reg)(akfs_module_t *);
    void (*unreg)(akfs_module_t *);
    void *(*alloc)(akfs_ring_t * ,int);
    void (*free)(akfs_ring_t *ring ,void *);
    int (*put)(akfs_args_t * ,void * ,unsigned int);
    int (*get_fpath)(struct file * ,char * ,int);
    int (*get_dpath)(struct dentry * ,char * ,int);
    int (*get_ipath)(struct inode * ,char * ,int);
    int (*get_ppath)(struct path * ,char * ,int);
    int (*get_tpath)(struct task_struct * ,char * ,int);
    int (*get_args)(struct linux_binprm * ,char * ,int);
    int (*get_cmdline)(struct task_struct * ,char * ,int);
    int (*get_thash)(struct task_struct * ,char * ,int);
    int (*get_fhash)(struct file * ,char * ,int);
    void (*get_timestamp)(char * ,int);
    uint64_t (*get_unixts)(void);
}akfs_operation_t;

extern const struct file_operations akfs_file_operations;

struct dentry *akfs_create_file(const char *name, umode_t mode,
        struct dentry *parent, void *data,
        const struct file_operations *fops);

struct dentry *akfs_create_dir(const char *name, struct dentry *parent);

void akfs_remove(struct dentry *dentry);

/**
 * @brief 通用ioctl处理函数
 */
int akfs_get_mlen(akfs_args_t *args_ptr ,unsigned long args);

#endif
