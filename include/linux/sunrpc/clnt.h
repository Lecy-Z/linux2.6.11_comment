/*
 *  linux/include/linux/sunrpc/clnt.h
 *
 *  Declarations for the high-level RPC client interface
 *
 *  Copyright (C) 1995, 1996, Olaf Kirch <okir@monad.swb.de>
 */

#ifndef _LINUX_SUNRPC_CLNT_H
#define _LINUX_SUNRPC_CLNT_H

#include <linux/sunrpc/msg_prot.h>
#include <linux/sunrpc/sched.h>
#include <linux/sunrpc/xprt.h>
#include <linux/sunrpc/auth.h>
#include <linux/sunrpc/stats.h>
#include <linux/sunrpc/xdr.h>
#include <linux/sunrpc/timer.h>
#include <asm/signal.h>

/*
 * This defines an RPC port mapping
 */
struct rpc_portmap {
	__u32			pm_prog;
	__u32			pm_vers;
	__u32			pm_prot;
	__u16			pm_port;
	unsigned char		pm_binding : 1;	/* doing a getport() */
	struct rpc_wait_queue	pm_bindwait;	/* waiting on getport() */
};

struct rpc_inode;

/*
 * The high-level client handle
 */
/**
 * RPC�ͻ�������Ϣ
 */
struct rpc_clnt {
	atomic_t		cl_count;	/* Number of clones */
	/**
	 * ���ô��� 
	 */
	atomic_t		cl_users;	/* number of references */
	/**
	 * �����ӿ�
	 */
	struct rpc_xprt *	cl_xprt;	/* transport */
	/**
	 * Զ�̹��� 
	 */
	struct rpc_procinfo *	cl_procinfo;	/* procedure info */
	/**
	 * ���Զ�̹�����Ŀ 
	 */
	u32			cl_maxproc;	/* max procedure number */

	/**
	 * Server������ 
	 */
	char *			cl_server;	/* server machine name */
	/**
	 * Զ�̳����� 
	 */
	char *			cl_protname;	/* protocol name */
	/**
	 * ��֤�ӿ� 
	 */
	struct rpc_auth *	cl_auth;	/* authenticator */
	/**
	 * ͳ������ 
	 */
	struct rpc_stat *	cl_stats;	/* statistics */

	/**
	 * ����ʱ 
	 */
	unsigned int		cl_softrtry : 1,/* soft timeouts */
	/**
	 * �Ƿ���ж� 
	 */
				cl_intr     : 1,/* interruptible */
	/**
	 * �Ƿ���ϸ˵�� 
	 */
				cl_chatty   : 1,/* be verbose */
	/**
	 * �Ƿ���ö˿�ӳ�� 
	 */
				cl_autobind : 1,/* use getport() */
				cl_droppriv : 1,/* enable NFS suid hack */
				cl_oneshot  : 1,/* dispose after use */
				cl_dead     : 1;/* abandoned */

	/**
	 * �ͻ������� 
	 */
	struct rpc_rtt *	cl_rtt;		/* RTO estimator data */
	/**
	 * �˿�ӳ��ӿ�
	 */
	struct rpc_portmap *	cl_pmap;	/* port mapping */

	/** 
	 * ��㳤�� 
	 */
	int			cl_nodelen;	/* nodename length */
	char 			cl_nodename[UNX_MAXNODENAME];
	char			cl_pathname[30];/* Path in rpc_pipe_fs */
	struct dentry *		cl_dentry;	/* inode */
	struct rpc_clnt *	cl_parent;	/* Points to parent of clones */
	struct rpc_rtt		cl_rtt_default;
	struct rpc_portmap	cl_pmap_default;
	char			cl_inline_name[32];
};
#define cl_timeout		cl_xprt->timeout
#define cl_prog			cl_pmap->pm_prog
#define cl_vers			cl_pmap->pm_vers
#define cl_port			cl_pmap->pm_port
#define cl_prot			cl_pmap->pm_prot

/*
 * General RPC program info
 */
#define RPC_MAXVERSION		4
/**
 * Զ�̳���
 */
struct rpc_program {
	/**
	 * Զ�̳����� 
	 */
	char *			name;		/* protocol name */
	/**
	 * ����� 
	 */
	u32			number;		/* program number */
	/**
	 * �汾���� 
	 */
	unsigned int		nrvers;		/* number of versions */
	/**
	 * ָ���и����汾��Ϣ������ָ�� 
	 */
	struct rpc_version **	version;	/* version array */
	/**
	 * ͳ����Ϣ 
	 */
	struct rpc_stat *	stats;		/* statistics */
	char *			pipe_dir_name;	/* path to rpc_pipefs dir */
};

/**
 * �汾��Ϣ 
 */
struct rpc_version {
	/**
	 * �汾�� 
	 */
	u32			number;		/* version number */
	/**
	 * Զ�̹�����
	 */
	unsigned int		nrprocs;	/* number of procs */
	/**
	 * Զ�̹������� 
	 */
	struct rpc_procinfo *	procs;		/* procedure array */
};

/*
 * Procedure information
 */
/**
 * Զ�̹�����Ϣ 
 */
struct rpc_procinfo {
	/**
	 * ���̺�
	 */
	u32			p_proc;		/* RPC procedure number */
	/**
	 * XDR���뺯�� 
	 */
	kxdrproc_t		p_encode;	/* XDR encode function */
	/**
	 * XDR ���뺯�� 
	 */
	kxdrproc_t		p_decode;	/* XDR decode function */
	/**
	 * ���󻺴��С 
	 */
	unsigned int		p_bufsiz;	/* req. buffer size */
	/**
	 * ������ 
	 */
	unsigned int		p_count;	/* call count */
	unsigned int		p_timer;	/* Which RTT timer to use */
};

#define RPC_CONGESTED(clnt)	(RPCXPRT_CONGESTED((clnt)->cl_xprt))
#define RPC_PEERADDR(clnt)	(&(clnt)->cl_xprt->addr)

#ifdef __KERNEL__

struct rpc_clnt *rpc_create_client(struct rpc_xprt *xprt, char *servname,
				struct rpc_program *info,
				u32 version, rpc_authflavor_t authflavor);
struct rpc_clnt *rpc_clone_client(struct rpc_clnt *);
int		rpc_shutdown_client(struct rpc_clnt *);
int		rpc_destroy_client(struct rpc_clnt *);
void		rpc_release_client(struct rpc_clnt *);
void		rpc_getport(struct rpc_task *, struct rpc_clnt *);
int		rpc_register(u32, u32, int, unsigned short, int *);

void		rpc_call_setup(struct rpc_task *, struct rpc_message *, int);

int		rpc_call_async(struct rpc_clnt *clnt, struct rpc_message *msg,
			       int flags, rpc_action callback, void *clntdata);
int		rpc_call_sync(struct rpc_clnt *clnt, struct rpc_message *msg,
			      int flags);
void		rpc_restart_call(struct rpc_task *);
void		rpc_clnt_sigmask(struct rpc_clnt *clnt, sigset_t *oldset);
void		rpc_clnt_sigunmask(struct rpc_clnt *clnt, sigset_t *oldset);
void		rpc_setbufsize(struct rpc_clnt *, unsigned int, unsigned int);

static __inline__
int rpc_call(struct rpc_clnt *clnt, u32 proc, void *argp, void *resp, int flags)
{
	struct rpc_message msg = {
		.rpc_proc	= &clnt->cl_procinfo[proc],
		.rpc_argp	= argp,
		.rpc_resp	= resp,
		.rpc_cred	= NULL
	};
	return rpc_call_sync(clnt, &msg, flags);
}
		
extern void rpciod_wake_up(void);

/*
 * Helper function for NFSroot support
 */
int		rpc_getport_external(struct sockaddr_in *, __u32, __u32, int);

#endif /* __KERNEL__ */
#endif /* _LINUX_SUNRPC_CLNT_H */