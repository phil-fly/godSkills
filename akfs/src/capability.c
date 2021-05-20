#include "dpath.h"

static __akfs_access_process_vm_type_ptr akfs_access_process_vm_ptr = NULL;

static __akfs_get_mm_exe_file_type_ptr akfs_get_mm_exe_file_ptr = NULL;

int akfs_capability_init(void)
{
    akfs_access_process_vm_ptr = (__akfs_access_process_vm_type_ptr)kallsyms_lookup_name("access_process_vm");
    assert_error(akfs_access_process_vm_ptr ,-EACCES);

    akfs_get_mm_exe_file_ptr = (__akfs_get_mm_exe_file_type_ptr)kallsyms_lookup_name("get_mm_exe_file");
    assert_error(akfs_get_mm_exe_file_ptr ,-EACCES);

    return 0;
}

/**
 * @brief 内核导出函数 计算进程路径相关接口 
 */
int access_process_vm2(struct task_struct *tsk, unsigned long addr,
        void *buf, int len, int write){
    return akfs_access_process_vm_ptr(tsk ,addr ,buf ,len ,write);
}

struct file *get_mm_exe_file(struct mm_struct *mm){
    return akfs_get_mm_exe_file_ptr(mm);
}

#ifdef CONFIG_MMU

void acct_arg_size(struct linux_binprm *bprm, unsigned long pages)
{
    struct mm_struct *mm = current->mm;
    long diff = (long)(pages - bprm->vma_pages);

    if (!mm || !diff)
        return;

    bprm->vma_pages = pages;
    add_mm_counter(mm, MM_ANONPAGES, diff);
}

struct page *get_arg_page(struct linux_binprm *bprm, unsigned long pos,
        int write)
{   
    struct page *page;
    int ret;

#ifdef CONFIG_STACK_GROWSUP
    if (write) {
        ret = expand_downwards(bprm->vma, pos);
        if (ret < 0)
            return NULL;
    }
#endif
    ret = get_user_pages(current, bprm->mm, pos,
            1, write, 1, &page, NULL);
    if (ret <= 0)
        return NULL;

    if (write) { 
        unsigned long size = bprm->vma->vm_end - bprm->vma->vm_start;
        struct rlimit *rlim;

        acct_arg_size(bprm, size / PAGE_SIZE);

        /*
         * We've historically supported up to 32 pages (ARG_MAX)
         * of argument strings even with small stacks
         */
        if (size <= ARG_MAX)
            return page;

        /*
         * Limit to 1/4-th the stack size for the argv+env strings.
         * This ensures that:
         *  - the remaining binfmt code will not run out of stack space,
         *  - the program will have a reasonable amount of stack left
         *    to work from.
         */
        rlim = current->signal->rlim;
        if (size > ACCESS_ONCE(rlim[RLIMIT_STACK].rlim_cur) / 4) {
            put_page(page);
            return NULL;
        }
    }

    return page;
}


void put_arg_page(struct page *page)
{
    put_page(page);
}
#else

void acct_arg_size(struct linux_binprm *bprm, unsigned long pages)
{
}

struct page *get_arg_page(struct linux_binprm *bprm, unsigned long pos,
        int write)
{
    struct page *page;

    page = bprm->page[pos / PAGE_SIZE];
    if (!page && write) {
        page = alloc_page(GFP_HIGHUSER|__GFP_ZERO);
        if (!page)
            return NULL;
        bprm->page[pos / PAGE_SIZE] = page;
    }

    return page;
}

void put_arg_page(struct page *page)
{
}

#endif
