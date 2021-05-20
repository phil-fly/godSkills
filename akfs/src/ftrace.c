#include "hook.h"
#include "assertion.h"


#if defined(__USE_FTRACE__)

/**
 * @brief  ftrace_reset 
 *      默认触发ftrace调用的函数，通过该函数跳转到对应的自己的处理函数
 */
static void notrace ftrace_reset(unsigned long ip, unsigned long parent_ip,
        struct ftrace_ops *ops, struct pt_regs *regs){
    regs->ip = (unsigned long)(akfs_trace_offset(ops))->function;
}

/**
 * @brief  ftrace_mount 
 *      常规的hook两步
 *          1、查找要hook函数的地址
 *          2、在被hook函数入口点植入跳转指令
 */
int ftrace_mount(akfs_trace_t *trace)
{
    struct ftrace_ops *fops = (struct ftrace_ops *)trace->reserve;
    int ret = 0;

    trace->flags = 0;

    trace->address = kallsyms_lookup_name(trace->name);
    assert_error(trace->address ,-EPERM);

    trace->original = trace->address + MCOUNT_INSN_SIZE;

    fops->func =  ftrace_reset;

    fops->flags = FTRACE_OPS_FL_SAVE_REGS | FTRACE_OPS_FL_RECURSION_SAFE
        | FTRACE_OPS_FL_IPMODIFY;

    preempt_disable_notrace();

    ret = ftrace_set_filter_ip(fops ,trace->address ,0 ,0);
    assert_goto(!(ret < 0) ,out ,);

    ret = register_ftrace_function(fops);
    assert_goto(!(ret < 0) ,out ,);

    trace->flags = 1; 

out:
    preempt_enable_notrace();

    return ret;
}

/**
 * @brief  ftrace_mount 
 *      退出函数反着来，清理hook现场
 */
void ftrace_umount(akfs_trace_t *trace)
{
    int ret = 0;
    struct ftrace_ops *fops = (struct ftrace_ops *)trace->reserve;

    assert_ret(trace->flags & 1);

    preempt_disable_notrace();

    ret = unregister_ftrace_function(fops);
    assert_goto(!(ret < 0) ,out ,);

    ftrace_set_filter_ip(fops ,trace->address ,1 ,0);

out:
    preempt_enable_notrace();
}

int ftrace_mounts(akfs_trace_t **trace ,unsigned int size)
{
    int i ,ret = 0;

    for(i = 0 ;i < size ;i++){
        ret = ftrace_mount(trace[i]);
        assert_break(!ret ,);       
    }

    return ret;
}

void ftrace_umounts(akfs_trace_t **trace ,unsigned int size)
{
    int i;

    for(i = 0 ;i < size ;i++){
        ftrace_umount(trace[i]);
    }
}

#endif
