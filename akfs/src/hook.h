#ifndef __ALWAYSKNOW_TRACE__
#define __ALWAYSKNOW_TRACE__

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/ftrace.h>
#include <linux/stop_machine.h>
#include <linux/slab.h>
#include <linux/sched.h>
#include <linux/kallsyms.h>
#include <linux/highmem.h>
#include <linux/mm.h>
#include <linux/binfmts.h>
#include <linux/cred.h>
#include <linux/file.h>

typedef struct akfs_trace_s{
    const char *name;
    void *function;
    int flags;
    unsigned long address;
    unsigned long original;
    unsigned char reserve[128];
}akfs_trace_t __attribute__((aligned(8)));

#define akfs_trace_offset(ops)  \
    (akfs_trace_t *)((unsigned char *)ops - offsetof(akfs_trace_t ,reserve))

enum __akfs_trace_type_s{
    AKFS_TRACE_TYPE_FTRACE = 0 ,
    AKFS_TRACE_TYPE_MAX ,
};

/**
 * @brief 
 * 默认trace 函数
 */
static inline int __trace_def_mount(akfs_trace_t *trace){
    return 0;
}

static inline void __trace_def_umount(akfs_trace_t *trace){

}

static inline int __trace_def_mounts(akfs_trace_t **trace ,unsigned int size){
    return 0;
}

static inline void __trace_def_umounts(akfs_trace_t **trace ,unsigned int size){

}

int ftrace_mount(akfs_trace_t *trace);

int ftrace_mounts(akfs_trace_t **trace ,unsigned int size);

void  ftrace_umount(akfs_trace_t *trace);

void  ftrace_umounts(akfs_trace_t **trace ,unsigned int size);

#if defined(__USE_FTRACE__)
#define akfs_trace_mount  ftrace_mount
#define akfs_trace_umount  ftrace_umount

#define akfs_trace_mounts  ftrace_mounts
#define akfs_trace_umounts  ftrace_umounts

#else

#define akfs_trace_mount   __trace_def_mount
#define akfs_trace_umount  __trace_def_umount

#define akfs_trace_mounts  __trace_def_mounts
#define akfs_trace_umounts __trace_def_umounts

#endif

#endif
