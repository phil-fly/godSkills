#include "symbol.h"

extern akfs_operation_t *__akfs_ops;
extern akfs_module_t __net; 
static DEFINE_SPINLOCK(__akfs_chan_lock_net_ptr);

/**
* @brief __put_ring 
*   往ring丢数据，并唤醒
*/
static void __put_ring(akfs_net_t *n){
    spin_lock(&__akfs_chan_lock_net_ptr);
    __akfs_ops->put(&__net.c_args ,n ,PAGE_SIZE);
    spin_unlock(&__akfs_chan_lock_net_ptr);
}

/**
 * @brief __padding_id 
 *   填充id字段
 */
static void __padding_id(akfs_net_t *n ,struct task_struct *task ,
        unsigned int type)
{
    n->data_type = type;

    //获取相应id值
    n->pid = task->pid;
    n->ns = akfs_get_nspid(task);
    n->uid = akfs_get_uid(task);
    n->gid = akfs_get_gid(task);

    //填充时间戳
    n->timestamp = __akfs_ops->get_unixts();

    //填充进程全路径
    __akfs_ops->get_tpath(task, n->exec_file ,sizeof(n->exec_file));
}


void aknet_tcp_v4_connect_hook(struct sock *sk)
{

    struct inet_sock *inet   = inet_sk(sk);
    akfs_net_t *net 	 = NULL;

    assert_ret(sk);

    net = (akfs_net_t *)__akfs_ops->alloc(__net.c_ring ,1);
    assert_ret(net);

    __padding_id(net ,current ,NET_TYPE_TCP_CONNECT);
    net->srcip	= inet->inet_saddr;
    net->dstip	= inet->inet_daddr;
    net->src_port	= inet->inet_sport;
    net->dst_port	= inet->inet_dport;
 
    __put_ring(net);

    __akfs_ops->free(__net.c_ring, net);	
}

void aknet_inet_csk_accept_hook(struct sock *sk)
{

    struct inet_sock *inet   = inet_sk(sk);
    akfs_net_t *net          = NULL;

    assert_ret(sk);

    net = (akfs_net_t *)__akfs_ops->alloc(__net.c_ring ,1);
    assert_ret(net);

    __padding_id(net ,current ,NET_TYPE_TCP_ACCEPT);
    net->srcip      = inet->inet_saddr;
    net->dstip      = inet->inet_daddr;
    net->src_port   = inet->inet_sport;
    net->dst_port   = inet->inet_dport;
 
    __put_ring(net);

    __akfs_ops->free(__net.c_ring, net);
}

void aknet_tcp_close_hook(struct sock *sk)
{

    struct inet_sock *inet   = inet_sk(sk);
    akfs_net_t *net          = NULL;

    assert_ret(sk);

    net = (akfs_net_t *)__akfs_ops->alloc(__net.c_ring ,1);
    assert_ret(net);

    __padding_id(net ,current ,NET_TYPE_TCP_CLOSE);
    net->srcip      = inet->inet_saddr;
    net->dstip      = inet->inet_daddr;
    net->src_port   = inet->inet_sport;
    net->dst_port   = inet->inet_dport;

    __put_ring(net);

    __akfs_ops->free(__net.c_ring, net);
}

void aknet_udp_sendmsg_hook(struct kiocb *iocb, struct sock *sk, struct msghdr *msg,size_t len)
{
    struct inet_sock *inet = inet_sk(sk);
    akfs_net_t *net          = NULL;

    assert_ret(sk); 

    printk("dport:%d\n", ntohs(inet->inet_dport));
    if (inet->inet_dport != htons(53)) {
        return;
    }


    net = (akfs_net_t *)__akfs_ops->alloc(__net.c_ring ,1);
    assert_ret(net);

    __padding_id(net ,current ,NET_TYPE_DNS);
    net->srcip      = inet->inet_saddr;
    net->dstip      = inet->inet_daddr;
    net->src_port   = inet->inet_sport;
    net->dst_port   = inet->inet_dport;

    __put_ring(net);
    __akfs_ops->free(__net.c_ring, net);
}
