#include "symbol.h"

extern unsigned long int __sym_hook_start;
extern unsigned long int __sym_hook_end;

akfs_operation_t *__akfs_ops = NULL;
akfs_module_t __net;

static int aknet_init(void)
{
    int ret = 0;

    __akfs_ops = (akfs_operation_t *)kallsyms_lookup_name(AKFS_OPS_NAME);
    assert_error(__akfs_ops ,-EACCES);

    __akfs_ops->init(&__net ,"net" ,AKFS_DEF_SIZE,THIS_MODULE,
            (unsigned long int)&__sym_hook_start ,
            (unsigned long int)&__sym_hook_end ,NULL);

    ret = __akfs_ops->reg(&__net);
    assert_error(!ret ,ret);

    return 0;
}

static void aknet_exit(void){
    __akfs_ops->unreg(&__net);
}

module_init(aknet_init);
module_exit(aknet_exit);
MODULE_LICENSE("GPL");
