
#include "symbol.h"

akfs_trace_declare(tcp_v4_connect);
static int akfs_trace_function(tcp_v4_connect)(struct sock *sk, struct sockaddr *uaddr, int addr_len)	
{
	int rv = 0;	

	int (*run)(struct sock *sk, struct sockaddr *uaddr, int addr_len) = (void *)akfs_trace_get_original(tcp_v4_connect);
	
	rv = run(sk, uaddr, addr_len);

	aknet_tcp_v4_connect_hook(sk);

	return rv;
}


akfs_trace_declare(inet_csk_accept);
static struct sock* akfs_trace_function(inet_csk_accept)(struct sock *sk, int flags, int *err)
{
	struct sock *newsk = NULL;

	struct sock* (*run)(struct sock *sk, int flags, int *err) = (void *)akfs_trace_get_original(inet_csk_accept);

	newsk = run(sk, flags, err);

	aknet_inet_csk_accept_hook(newsk);
	
	return newsk;

}

akfs_trace_declare(tcp_close);
static void akfs_trace_function(tcp_close)(struct sock *sk, long timeout)
{
	void (*run)(struct sock *sk, long timeout) = (void *)akfs_trace_get_original(tcp_close);
	
	run(sk, timeout);

	aknet_tcp_close_hook(sk);

	return;
}

akfs_trace_declare(udp_sendmsg);
static int akfs_trace_function(udp_sendmsg)(struct kiocb *iocb, struct sock *sk, struct msghdr *msg, size_t len)
{
	int rv = 0;

	int (*run)(struct kiocb *iocb, struct sock *sk, struct msghdr *msg, size_t len) = (void *)akfs_trace_get_original(udp_sendmsg);
	
	rv = run(iocb, sk, msg, len);

	aknet_udp_sendmsg_hook(iocb, sk, msg, len);

	return rv;
}
