/**
 * @file inode.c
 * @brief 
 * @author mangosteen
 * @version 1.0.0.0
 * @date 2021-03-06
 */

#include "akfs.h"

/**
 * @brief 全局变量
 */
static struct vfsmount *akfs_mount;
static int akfs_mount_count;

/**
 * @brief akfs_get_inode 
 *   从super_block上获取一个inode节点
 */
static struct inode *akfs_get_inode(struct super_block *sb ,
        umode_t mode, dev_t dev,
        void *data, const struct file_operations *fops)

{
    struct inode *inode = new_inode(sb);

    assert_error(inode ,NULL);

    inode->i_ino = get_next_ino();
    inode->i_mode = mode;
    inode->i_atime = inode->i_mtime = inode->i_ctime = CURRENT_TIME;

    switch (mode & S_IFMT) {
        default:
            init_special_inode(inode, mode, dev);
            break;
        case S_IFREG:
            inode->i_fop = fops ? fops : &akfs_file_operations;
            inode->i_private = data;
            break;
        case S_IFDIR:
            inode->i_op = &simple_dir_inode_operations;
            inode->i_fop = &simple_dir_operations;

            inc_nlink(inode);
            break;
    }

    return inode; 
}

/**
 * @brief akfs_mknod 
 *   创建inode节点
 */
static int akfs_mknod(struct inode *dir, struct dentry *dentry,
        umode_t mode, dev_t dev, void *data,
        const struct file_operations *fops)
{
    struct inode *inode = NULL;

    assert_error(!(dentry->d_inode) ,-EEXIST);

    inode = akfs_get_inode(dir->i_sb, mode, dev, data, fops);
    assert_error(inode ,-EPERM);

    d_instantiate(dentry, inode);
    dget(dentry);

    return 0;
}

/**
 * @brief akfs_mkdir 
 *   创建目录
 */
static int akfs_mkdir(struct inode *dir, struct dentry *dentry, umode_t mode)
{
    int res;

    mode = (mode & (S_IRWXUGO | S_ISVTX)) | S_IFDIR;

    res = akfs_mknod(dir, dentry, mode, 0, NULL, NULL);
    if(!res){
        inc_nlink(dir);
        fsnotify_mkdir(dir, dentry);
    }

    return res;
}

static int akfs_create(struct inode *dir, struct dentry *dentry, umode_t mode,
        void *data, const struct file_operations *fops)
{
    mode = (mode & S_IALLUGO) | S_IFREG;

    return akfs_mknod(dir, dentry, mode, 0, data, fops);
}

static inline int akfs_positive(struct dentry *dentry){
    return dentry->d_inode && !d_unhashed(dentry);
}

static const match_table_t tokens = {
    {Opt_uid, "uid=%u"},
    {Opt_gid, "gid=%u"},
    {Opt_mode, "mode=%o"},
    {Opt_err, NULL}
};

/**
 * @brief akfs_parse_options 
 *   解析mount参数
 */
static int akfs_parse_options(char *data, struct akfs_mount_opts *opts)
{
    substring_t args[MAX_OPT_ARGS];
    int option;
    int token;
    kuid_t uid;
    kgid_t gid;
    char *p;

    opts->mode = AKFS_DEFAULT_MODE;

    while ((p = strsep(&data, ",")) != NULL) {
        if (!*p){
            continue;
        }

        token = match_token(p, tokens, args);
        switch (token) {
            case Opt_uid:
                if (match_int(&args[0], &option))
                    return -EINVAL;
                uid = make_kuid(current_user_ns(), option);
                if (!uid_valid(uid))
                    return -EINVAL;
                opts->uid = uid;
                break;
            case Opt_gid:
                if (match_int(&args[0], &option))
                    return -EINVAL;
                gid = make_kgid(current_user_ns(), option);
                if (!gid_valid(gid))
                    return -EINVAL;
                opts->gid = gid;
                break;
            case Opt_mode:
                if (match_octal(&args[0], &option))
                    return -EINVAL;
                opts->mode = option & S_IALLUGO;
                break;
        }
    }

    return 0;
}

static int akfs_apply_options(struct super_block *sb)
{
    struct akfs_fs_info *fsi = sb->s_fs_info;
    struct inode *inode = sb->s_root->d_inode;
    struct akfs_mount_opts *opts = &fsi->mount_opts;

    inode->i_mode &= ~S_IALLUGO;
    inode->i_mode |= opts->mode;

    inode->i_uid = opts->uid;
    inode->i_gid = opts->gid;

    return 0;
}

static int akfs_remount(struct super_block *sb, int *flags, char *data)
{
    int err;
    struct akfs_fs_info *fsi = sb->s_fs_info;

    err = akfs_parse_options(data, &fsi->mount_opts);
    assert_goto(!err ,out ,);

    akfs_apply_options(sb);

out:
    return err;
}

static int akfs_show_options(struct seq_file *m, struct dentry *root)
{
    struct akfs_fs_info *fsi = root->d_sb->s_fs_info;
    struct akfs_mount_opts *opts = &fsi->mount_opts;

    if (!uid_eq(opts->uid, GLOBAL_ROOT_UID))
        seq_printf(m, ",uid=%u",
                from_kuid_munged(&init_user_ns, opts->uid));
    if (!gid_eq(opts->gid, GLOBAL_ROOT_GID))
        seq_printf(m, ",gid=%u",
                from_kgid_munged(&init_user_ns, opts->gid));
    if (opts->mode != AKFS_DEFAULT_MODE)
        seq_printf(m, ",mode=%o", opts->mode);

    return 0;
}

static const struct super_operations akfs_super_operations = {
    .statfs		= simple_statfs,
    .remount_fs	= akfs_remount,
    .show_options	= akfs_show_options,
};

static int akfs_fill_super(struct super_block *sb, void *data, int silent)
{
    static struct tree_descr akfs_files[] = {{""}};
    struct akfs_fs_info *fsi;
    int err;

    save_mount_options(sb, data);

    fsi = kzalloc(sizeof(struct akfs_fs_info), GFP_KERNEL);
    sb->s_fs_info = fsi;
    assert_goto(fsi ,out ,err = -ENOMEM);

    err = akfs_parse_options(data, &fsi->mount_opts);
    assert_goto(!err ,out ,);

    err  =  simple_fill_super(sb, AKFS_MAGIC, akfs_files);
    assert_goto(!err ,out ,);

    sb->s_op = &akfs_super_operations;

    akfs_apply_options(sb);

    return 0;

out:
    kfree(fsi);
    sb->s_fs_info = NULL;
    return err;
}

static struct dentry *__akfs_mount(struct file_system_type *fs_type,
        int flags, const char *dev_name,
        void *data){
    return mount_single(fs_type, flags, data, akfs_fill_super);
}

static struct file_system_type akfs_fs_type = {
    .owner =	THIS_MODULE,
    .name =		"akfs",
    .mount =	__akfs_mount,
    .kill_sb =	kill_litter_super,
};
MODULE_ALIAS_FS("akfs");

/**
 * @brief __create_file 
 *   创建文件或在目录
 */
static struct dentry *__create_file(const char *name, umode_t mode,
        struct dentry *parent, void *data,
        const struct file_operations *fops)
{
    struct dentry *dentry = NULL;
    int error;


    error = simple_pin_fs(&akfs_fs_type, &akfs_mount,
            &akfs_mount_count);
    assert_error(!error ,NULL);

    //如果没有指定父目录，则挂载到根上。
    assert_void(parent ,parent = akfs_mount->mnt_root;);

    mutex_lock(&parent->d_inode->i_mutex);

    dentry = lookup_one_len(name, parent, strlen(name));
    if (!IS_ERR(dentry)) {
        switch (mode & S_IFMT) {
            case S_IFDIR:
                error = akfs_mkdir(parent->d_inode, dentry, mode);

                break;
            default:
                error = akfs_create(parent->d_inode, dentry, mode,
                        data, fops);
                break;
        }
        dput(dentry);
    }else{
        error = PTR_ERR(dentry);
    }

    mutex_unlock(&parent->d_inode->i_mutex);

    if (error) {
        dentry = NULL;
        simple_release_fs(&akfs_mount, &akfs_mount_count);
    }

    return dentry;
}


/**
 * @brief akfs_create_file 
 *   创建文件
 */
struct dentry *akfs_create_file(const char *name, umode_t mode,
        struct dentry *parent, void *data,
        const struct file_operations *fops)
{
    switch (mode & S_IFMT) {
        case S_IFREG:
        case 0:
            break;
        default:
            BUG();
    }

    return __create_file(name, mode, parent, data, fops);
}

/**
 * @brief akfs_create_dir 
 *   创建目录
 */
struct dentry *akfs_create_dir(const char *name, struct dentry *parent)
{
    return __create_file(name, S_IFDIR | S_IRWXU | S_IRUGO | S_IXUGO,
            parent, NULL, NULL);
}

/**
 * @brief __akfs_remove 
 *   根据dentry移除对应的文件/目录
 */
static int __akfs_remove(struct dentry *dentry, struct dentry *parent)
{
    int ret = 0;

    if (akfs_positive(dentry)) {
        if (dentry->d_inode) {
            dget(dentry);
            switch (dentry->d_inode->i_mode & S_IFMT) {
                case S_IFDIR:
                    ret = simple_rmdir(parent->d_inode, dentry);
                    break;
                case S_IFLNK:
                    kfree(dentry->d_inode->i_private);
                default:
                    simple_unlink(parent->d_inode, dentry);
                    break;
            }
            if (!ret)
                d_delete(dentry);
            dput(dentry);
        }
    }
    return ret;
}

/**
 * @brief akfs_remove 
 */
void akfs_remove(struct dentry *dentry)
{
    struct dentry *parent;
    int ret;

    if (IS_ERR_OR_NULL(dentry))
        return;

    parent = dentry->d_parent;
    if (!parent || !parent->d_inode)
        return;

    mutex_lock(&parent->d_inode->i_mutex);
    ret = __akfs_remove(dentry, parent);
    mutex_unlock(&parent->d_inode->i_mutex);

    if (!ret){
        simple_release_fs(&akfs_mount, &akfs_mount_count);
    }
}

/**
 * @brief akfs_init 
 *   注册文件系统
 */
static int __akfs_init(void){
    assert_error(!akfs_capability_init() ,-EACCES);
    return register_filesystem(&akfs_fs_type);
}

/**
 * @brief akfs_exit 
 *   卸载文件系统
 */
static void __akfs_exit(void){
    unregister_filesystem(&akfs_fs_type);
}

module_init(__akfs_init);
module_exit(__akfs_exit);
MODULE_LICENSE("GPL");

