#ifndef __AKNET_SYMBOL_H__
#define __AKNET_SYMBOL_H__

#include "akfs.h"
#include "lds.h"
#include <net/tcp.h>
#include <net/udp.h>
#include <linux/types.h>
#include <linux/sched.h>
#include <linux/kernel.h>

#define NIPQUAD(addr) \
    ((unsigned char *)&addr)[0], \
    ((unsigned char *)&addr)[1], \
    ((unsigned char *)&addr)[2], \
    ((unsigned char *)&addr)[3] 


enum akfs_net_type {
	NET_TYPE_TCP_CONNECT = 3001,
	NET_TYPE_TCP_ACCEPT,
	NET_TYPE_TCP_CLOSE,
	NET_TYPE_DNS = 3010,
};

#define AKNET_FILE_PATH_MAX_LEN	256

typedef struct akfs_net_s 		akfs_net_t;

struct akfs_net_s {
    uint64_t   timestamp;
    unsigned int data_type;
    pid_t pid;
    uid_t uid;
    pid_t gid;
    unsigned int   ns;
    unsigned int 	srcip;
    unsigned int 	dstip;
    unsigned short  src_port;
    unsigned short  dst_port;
    char exec_file[AKNET_FILE_PATH_MAX_LEN];
};


void aknet_tcp_v4_connect_hook(struct sock *sk);
void aknet_inet_csk_accept_hook(struct sock *sk);
void aknet_tcp_close_hook(struct sock *sk);
void aknet_udp_sendmsg_hook(struct kiocb *iocb, struct sock *sk, struct msghdr *msg,size_t len);

#endif

