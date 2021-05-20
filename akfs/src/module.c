#include "akfs.h"

/**
 * @brief akfs_module_init 
 *   akfs模块初始化
 */
static void akfs_module_init(akfs_module_t *module ,
        const char *name ,unsigned int size ,
        unsigned long int start ,unsigned long int end)
{
    module->name = name;

    //多出的PAGE是留给ring结构体本身的
    module->c_rlen = size * PAGE_SIZE;
    module->c_rlen = PAGE_ALIGN(module->c_rlen) + PAGE_SIZE;

    module->c_ring = NULL;

    module->h_start = (akfs_trace_t *)start;
    module->h_end = (akfs_trace_t *)end;

    module->c_otp_seed = 0;
    module->c_otp_status = AKFS_FSA_WAIT;
}

/**
* @brief akfs_data_put 
*   数据添加到ring并唤醒等待队列
*/
static int akfs_data_put(akfs_args_t *args ,void *data ,unsigned int len)
{
    int size;

    size = __akfs_ring_put(args->ring ,data ,len);

    smp_wmb();
    args->condition = 1;

    wake_up_interruptible(&args->waitq);

    return size;
}

/**
 * @brief akfs_hook_register 
 *   hook section的点
 */
static int akfs_hook_register(akfs_hook_t *hook)
{
    akfs_trace_t *trace = NULL;

    for(trace = hook->start ;trace < hook->end ;trace++){
        assert_error(!akfs_trace_mount(trace) ,-EACCES);
    }

    return 0;
}

/**
 * @brief akfs_hook_unregister 
 *   卸载hook点
 */
static void akfs_hook_unregister(akfs_hook_t *hook)
{
    akfs_trace_t *trace = NULL;

    for(trace = hook->start ;trace < hook->end ;trace++){
        akfs_trace_umount(trace);
    }
}

/**
 * @brief akfs_chan_register 
 *   注册channel文件
 */
static int akfs_chan_register(akfs_channel_t *chan ,const char *name)
{
    init_waitqueue_head(&chan->a_waitq);

    chan->a_condition = 0;

    chan->a_interrupt = 0;

    chan->dir = akfs_create_file(name ,0644 ,NULL ,
            &chan->args ,NULL);

    return chan->dir ? 0: -EACCES;
}

/**
 * @brief akfs_chan_unregister 
 *   移除channel文件
 */
static void akfs_chan_unregister(akfs_channel_t *chan){
    assert_void(!chan->dir ,akfs_remove(chan->dir));
    assert_void(!chan->a_ring ,akfs_ring_exit(chan->a_ring););
}

/**
 * @brief akfs_module_register 
 *   akfs的module注册接口
 */
static int akfs_module_register(akfs_module_t *module)
{
    int ret = 0;

    ret = akfs_chan_register(&module->chan ,module->name);
    assert_error(!ret ,ret);

    ret = akfs_hook_register(&module->hook);
    assert_void(!ret ,akfs_hook_unregister(&module->hook);
            akfs_chan_unregister(&module->chan););

    return ret;
}

/**
 * @brief akfs_module_unregister 
 *   akfs的模块卸载接口
 */
static void akfs_module_unregister(akfs_module_t *module)
{
    akfs_hook_unregister(&module->hook);

    akfs_chan_unregister(&module->chan);
}

/**
 * @brief 对外导出的akfs模块操作接口
 */
static akfs_operation_t __akfs_module_ops = {
    .init = akfs_module_init,
    .reg = akfs_module_register ,
    .unreg = akfs_module_unregister,
    .put = akfs_data_put,
    .alloc = akfs_ring_alloc,
    .free = akfs_ring_free,
    .get_ppath = akfs_get_ppath,  
    .get_fpath = akfs_get_fpath,
    .get_dpath = akfs_get_dpath,
    .get_ipath = akfs_get_ipath,
    .get_tpath = akfs_get_tpath,
    .get_cmdline = akfs_get_cmdline,
    .get_args = akfs_get_args,
    .get_thash = akfs_get_thash,
    .get_fhash = akfs_get_fhash,
    .get_timestamp = akfs_get_timestamp,
    .get_unixts = akfs_get_unixts,
};
EXPORT_SYMBOL_GPL(__akfs_module_ops);
