#ifndef __ALWAYSKNOW_DPATH_H__
#define __ALWAYSKNOW_DPATH_H__

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/ftrace.h>
#include <linux/stop_machine.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/debugfs.h>
#include <linux/kallsyms.h>
#include <linux/highmem.h>
#include <linux/mm.h>
#include <linux/binfmts.h>
#include <linux/cred.h>
#include <linux/file.h>
#include <linux/poll.h>
#include <uapi/asm/ioctls.h>
#include <linux/xattr.h>
#include <linux/timer.h>
#include <linux/timex.h>
#include <linux/rtc.h>

#include "md5.h"
#include "assertion.h"

typedef int (*__akfs_access_process_vm_type_ptr)(struct task_struct *tsk, unsigned long addr, void *buf, int len, int write);

typedef struct file *(*__akfs_get_mm_exe_file_type_ptr)(struct mm_struct *mm);

int access_process_vm2(struct task_struct *tsk, unsigned long addr,
        void *buf, int len, int write);

struct file *get_mm_exe_file(struct mm_struct *mm);

void acct_arg_size(struct linux_binprm *bprm, unsigned long pages);

struct page *get_arg_page(struct linux_binprm *bprm ,unsigned long pos ,int write);

void put_arg_page(struct page *page);

int akfs_get_ppath(struct path *path ,char *buffer ,int size);

int akfs_get_fpath(struct file *file ,char *buffer ,int size);

int akfs_get_dpath(struct dentry *dentry ,char *buffer ,int size);

int akfs_get_ipath(struct inode *inode ,char *buffer ,int size);

int akfs_get_tpath(struct task_struct *task ,char *buffer ,int size);

int akfs_get_args(struct linux_binprm *bprm ,char *argv ,int size);

int akfs_get_cmdline(struct task_struct *task ,char *args ,int size);

int akfs_capability_init(void);

/**
 * @brief hash相关 
 */
//存储hash值
#define AKFS_XATTR_HASH "trusted.hash"

//存储时间戳用于记录文件是否变化
#define AKFS_XATTR_TS   "trusted.timestamp"

int akfs_get_fhash(struct file *file ,char *buffer ,int len);

int akfs_get_thash(struct task_struct *task ,char *buffer ,int len);

void akfs_get_timestamp(char *buffer ,int len);

uint64_t akfs_get_unixts(void);

#endif
