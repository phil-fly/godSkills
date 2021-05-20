#ifndef __ALWAYSKNOW_AKFS_H_
#define __ALWAYSKNOW_AKFS_H_

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/mount.h>
#include <linux/pagemap.h>
#include <linux/ftrace.h>
#include <linux/stop_machine.h>
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

#define AKFS_OPS_NAME "__akfs_module_ops"

#define AKFS_DEF_SIZE 1024

/**
* @brief 常用逻辑判断宏定义 
*/
#define  assert_error(condition ,error) do{ \
    if(!(condition)){   \
        return (error); \
    }   \
}while(0)

#define  assert_ret(condition)  \
    if(!(condition)){   \
        return; \
    }   

#define  assert_goto(condition ,label ,ops) \
    if(!(condition)){   \
        ops;    \
        goto label; \
    }

#define  assert_break(condition ,ops) \
    if(!(condition)){   \
        ops;    \
        break; \
    }

#define  assert_continue(condition ,ops) \
    if(!(condition)){   \
        ops;    \
        continue; \
    }

#define  assert_void(condition ,ops)    do{ \
    if(!(condition)){   \
        ops;    \
    }   \
}while(0)


enum AKFS_FSA_S{
    AKFS_FSA_WAIT    = 1,
    AKFS_FSA_REQ     = 2,
    AKFS_FSA_REPLY   = 4,
    AKFS_FSA_ACCESS  = 8,
};

typedef struct akfs_otp_s{
    unsigned int status;
    unsigned int seed;
}akfs_otp_t;

typedef struct akfs_trace_s{
    const char *name;
    void *function;
    int flags;
    unsigned long address;
    unsigned long original;
    unsigned char reserve[128];
}akfs_trace_t __attribute__((aligned(8)));

/**
 * @brief ring
 */
typedef struct akfs_ring_s{
    unsigned int offset;
    unsigned int size;
    unsigned int in;
    unsigned int out;
}akfs_ring_t;

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
    unsigned int refer;
    unsigned int rlen;
    struct module *module;
    akfs_ring_t *ring;
    void *reserve;
}akfs_args_t;

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
#define c_ioctl chan.args.ioctl_handle
#define c_reserve chan.args.reserve
#define c_module chan.args.module
#define c_refer chan.args.refer
    akfs_hook_t hook;
#define h_start hook.start
#define h_end hook.end
}akfs_module_t;

/**
 * @brief akfs操作函数
 */
typedef struct akfs_operation_s{
    void (*init)(akfs_module_t *,const char *,unsigned int ,
            struct module *, unsigned long int ,unsigned long int ,void *);
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

/**
 *@brief 获取进程的namespace和uid 
 */
#define akfs_get_nspid(task)    \
    task->nsproxy && task->nsproxy->pid_ns ? \
task->nsproxy->pid_ns->proc_inum : 0 

#ifdef CONFIG_UIDGID_STRICT_TYPE_CHECKS

#define akfs_get_uid(task)  (current_cred())->uid.val
#define akfs_get_gid(task)  (current_cred())->gid.val

#else

#define akfs_get_uid(task) (current_cred())->uid
#define akfs_get_gid(task) (current_cred())->gid

#endif

#endif
