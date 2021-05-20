#include "dpath.h"

/**
 * @brief akfs_get_ppath 
 *   从struct path获取全路加
 */
int akfs_get_ppath(struct path *path ,char *buffer ,int size)
{
    char *ptr = NULL, *cache = NULL;
    int ret = -ENOMEM;

    cache = kmalloc(PATH_MAX, GFP_TEMPORARY);
    assert_error(cache ,-ENOMEM);

    ptr = d_path(path ,cache ,PATH_MAX);
    assert_goto(!IS_ERR(ptr) ,out ,);

    ret = snprintf(buffer ,size ,"%s" ,ptr);

out:
    kfree(cache);

    return ret;
}

/**
 * @brief akfs_get_path 
 *      获取file全路径
 */
int akfs_get_fpath(struct file *file ,char *buffer ,int size)
{
    char *ptr = NULL, *cache = NULL;
    int ret = -ENOMEM;

    cache = kmalloc(PATH_MAX, GFP_TEMPORARY);
    assert_error(cache ,-ENOMEM);

    ptr = d_path(&file->f_path ,cache ,PATH_MAX);
    assert_goto(!IS_ERR(ptr) ,out ,);

    ret = snprintf(buffer ,size ,"%s" ,ptr);

out:
    kfree(cache);

    return ret;
}

/**
 * @brief akfs_get_dpath 
 *      根据dentry获取全路径
 */
int akfs_get_dpath(struct dentry *dentry ,char *buffer ,int size)
{
    int ret = 0;
    char *page = NULL, *path = NULL;

    page = (char *)__get_free_page(GFP_KERNEL);
    assert_error(page ,-ENOMEM);

    path = dentry_path_raw(dentry ,page ,PAGE_SIZE);
    assert_goto(!IS_ERR(path) ,out_free ,ret = PTR_ERR(path));

    ret = snprintf(buffer ,size ,"%s" ,path);  

out_free: 
    free_page((unsigned long)page);

    return ret;
}

/**
 * @brief akfs_get_ipath 
 *   dentry = d_find_alias(inode);
 */
int akfs_get_ipath(struct inode *inode ,char *buffer ,int size)
{
    struct dentry *dentry = NULL;

    dentry = d_find_alias(inode);
    assert_error(dentry ,-EACCES);

    return akfs_get_dpath(dentry ,buffer ,size);
}

/**
 * @brief 获取进程的全路径 
 */
int akfs_get_tpath(struct task_struct *task ,char *buffer ,int size)
{
    struct mm_struct *mm = NULL;
    struct file *exe_file = NULL;
    char *pathbuf = NULL ,*path = NULL;
    int ret = 0;

    mm = get_task_mm(task);
    assert_error(mm ,-EACCES);

    exe_file = get_mm_exe_file(mm);
    mmput(mm);

    pathbuf = kmalloc(PATH_MAX, GFP_KERNEL);
    assert_goto(pathbuf ,out_put ,ret = -ENOMEM);

    path = d_path(&exe_file->f_path, pathbuf, PATH_MAX);
    assert_goto(!IS_ERR(path) ,out_free ,ret = PTR_ERR(path););

    ret = snprintf(buffer ,size ,"%s" ,path);

out_free:
    kfree(pathbuf);

out_put:
    fput(exe_file);

    return ret;
}

/**
 * @brief 获取程序启动时候的参数 
 */
int akfs_get_cmdline(struct task_struct *task ,char *args ,int size)
{
    int res = 0;
    unsigned int len;
    struct mm_struct *mm = NULL;
    char *buffer = NULL;

    mm = get_task_mm(task);
    if (!mm || !mm->arg_end){
        goto out;
    }

    buffer = kzalloc(PAGE_SIZE ,GFP_KERNEL);
    assert_goto(buffer ,out_mm ,);

    len = mm->arg_end - mm->arg_start;

    if (len > PAGE_SIZE){
        len = PAGE_SIZE;
    }

    res = access_process_vm2(task, mm->arg_start, buffer, len, 0);

    if (res > 0 && buffer[res-1] != '\0' && len < PAGE_SIZE) {
        len = strnlen(buffer, res);
        if (len < res) {
            res = len;
        } else {
            len = mm->env_end - mm->env_start;
            if (len > PAGE_SIZE - res)
                len = PAGE_SIZE - res;
            res += access_process_vm2(task, mm->env_start, buffer+res, len, 0);
            res = strnlen(buffer, res);
        }
    }

    snprintf(args ,size ,"%s" ,buffer);

    kfree(buffer);
out_mm:
    mmput(mm);
out:
    return res;
}

/**
 * @brief 从bprm里面获取参数 
 */
int akfs_get_args(struct linux_binprm *bprm ,char *argv ,int size)
{
    char *kaddr = NULL ,*start = NULL, *end = NULL;
    struct page *page = NULL;
    int argc = 0 ,count = 0;
    unsigned long pos ,offset; 

    start = argv;
    end = argv + size;

    argc = bprm->argc;
    pos = bprm->p;

    do
    {
        offset = pos & ~PAGE_MASK;

        page = get_arg_page(bprm ,pos ,0);
        assert_error(page ,-EACCES);

        kaddr = kmap_atomic(page);

        for ( ;(offset < PAGE_SIZE) && (count < argc) &&(start < end) ;
                offset++, pos++ ,start++) {
            if (kaddr[offset] == '\0') {
                count++;
                pos++;
                *start = ' ';
                continue;
            }
            *start = kaddr[offset];
        }

        kunmap_atomic(kaddr);

        put_arg_page(page);
    }while(offset == PAGE_SIZE);

    return 0;
}

/**
 * @brief __hex_to_string 
 *   十六进制hash转化成字符串
 */
static inline void __hex_to_string(unsigned char *hash ,char *str){
    int i;

    for(i = 0 ;i < MD5_DIGEST_SIZE ;i++){
        snprintf(str + i*2 ,3/*末尾0*/ ,"%02x" ,hash[i]);
    }

    str[MD5_STR_SIZE] = '\0';    
}

/**
 * @brief akfs_calc_hash 
 *   计算文件hash
 */
static int akfs_calc_fhash(struct file *file ,char *buffer ,int len)
{
    mm_segment_t oldfs;
    loff_t oldpos;
    char *ptr = NULL;
    int rlen = 0;
    struct md5_ctx ctx;
    unsigned char md5[MD5_DIGEST_SIZE] = {0};

    assert_error(!(!file || IS_ERR(file) || (len < MD5_STR_SIZE)) ,-EACCES);

    ptr = kmalloc(PAGE_SIZE ,GFP_KERNEL);
    assert_error(ptr ,-ENOMEM);

    oldpos = file->f_pos;
    file->f_pos = 0;

    md5_init(&ctx);

    oldfs = get_fs();
    set_fs(KERNEL_DS);

    do{
        rlen = file->f_op->read(file ,ptr ,PAGE_SIZE ,&file->f_pos);
        md5_update(&ctx ,ptr ,rlen);
    }while(rlen > 0);

    set_fs(oldfs);

    md5_final(&ctx ,md5);

    kfree(ptr);
    file->f_pos = oldpos;

    //转化成字符串
    __hex_to_string(md5 ,buffer);

    return 0;
}

/**
 * @brief akfs_xattr_hash 
 *   当文件首次计算hash的时候xattr为空
 *   则计算文件hash，计算完毕之后将hash存放
 *   到xattr。
 *   在此获取的时候校验文件是否发生变化，未变化
 *   则从xattr获取hash值。
 */
static int akfs_xattr_hash(struct file *file ,char *buffer ,int len)
{
    int ret = 0;

    return 1;

    assert_error(file->f_path.dentry ,-EACCES);

    ret = generic_getxattr(file->f_path.dentry ,AKFS_XATTR_HASH ,buffer ,len);
    assert_error(ret > 0 ,ret);

    generic_setxattr(file->f_path.dentry ,AKFS_XATTR_HASH ,buffer ,len ,XATTR_CREATE);

    return 0;
}

/**
* @brief akfs_get_fhash 
*   获取文件hash
*/
int akfs_get_fhash(struct file *file ,char *buffer ,int len)
{
    int ret = 0;

    ret = akfs_xattr_hash(file ,buffer ,len);
    assert_error(ret ,0);

    return akfs_calc_fhash(file ,buffer ,len);
}

/**
 * @brief akfs_get_thash 
 *   获取task对应的文件hash
 */
int akfs_get_thash(struct task_struct *task ,char *buffer ,int len)
{
    struct mm_struct *mm = NULL;
    struct file *exe_file = NULL;
    int ret = 0;

    mm = get_task_mm(task);
    assert_error(mm ,-EACCES);

    exe_file = get_mm_exe_file(mm);
    mmput(mm);

    ret = akfs_get_fhash(exe_file ,buffer ,len); 

    fput(exe_file);

    return ret;
}

/**
* @brief akfs_get_timestamp 
*   获取当前时间
*   该timedatectl set-local-rtc 1 
*   将硬件时钟调整为与本地时钟一致
*/
void akfs_get_timestamp(char *buffer ,int len)
{
    struct timeval  utc;
    struct rtc_time tm;

    do_gettimeofday(&utc);

    assert_void(!sys_tz.tz_minuteswest ,
            utc.tv_sec -= sys_tz.tz_minuteswest * 60);

    rtc_time_to_tm(utc.tv_sec ,&tm);

    snprintf(buffer ,len ,"%04d-%02d-%02d %02d:%02d:%02d",
            tm.tm_year + 1900 ,tm.tm_mon + 1 ,
            tm.tm_mday ,tm.tm_hour ,tm.tm_min ,tm.tm_sec);
}

/**
 * @brief akfs_get_unixts 
 *   获取当前unix时间戳
 */
uint64_t akfs_get_unixts(void)
{
    struct timeval  utc;

    do_gettimeofday(&utc);

    assert_void(!sys_tz.tz_minuteswest ,
            utc.tv_sec -= sys_tz.tz_minuteswest * 60);

    return (uint64_t)utc.tv_sec;
}
