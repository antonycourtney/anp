/*
 * anp_syscall.c -- system call wrappers (called by client code) which
 *		    dispatch into internal kernel routines
 *
 * (These routines should be compiled w/ BSD headers) 
 *
 * Antony Courtney,	4/17/96
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/filedesc.h>
#include <sys/ioctl.h>
#include <sys/file.h>
#include <sys/proc.h>
#include <sys/socketvar.h>
#include <sys/uio.h>
#include <sys/kernel.h>
#include <sys/stat.h>
#include <sys/malloc.h>
#ifdef KTRACE
#include <sys/ktrace.h>
#endif

#include "anp_fdtbl.h"
#include "mt_bsd.h"


/* A replacement for errno, for error reporting: */
int anp_errno=0;

/* generic calls (originally in kern/sys_generic.c): */

/*
 * Read system call.
 */
int anp_read(int fd,char *buf,unsigned int nbytes)
{
     register struct socket *sock;
     struct uio auio;
     struct iovec aiov;
     long cnt, error = 0;

     MU_LOCK(kern_lock_p);
     sock=anp_fdfind(fd);

     aiov.iov_base = (caddr_t) buf;
     aiov.iov_len = nbytes;
     auio.uio_iov = &aiov;
     auio.uio_iovcnt = 1;
     auio.uio_resid = nbytes;
     auio.uio_rw = UIO_READ;
     auio.uio_segflg = UIO_USERSPACE;

     cnt = nbytes;
     if (error = (soo_read)(sock, &auio))
	  if (auio.uio_resid != cnt && (error == ERESTART ||
					error == EINTR || error == EWOULDBLOCK))
	       error = 0;
     cnt -= auio.uio_resid;

     anp_errno=error;
     MU_UNLOCK(kern_lock_p);
     if (anp_errno!=0) {
	  return -1;
     } else {
	  return cnt;
     }
}

/*
 * Write system call
 */
int anp_write(int fd,char *buf,unsigned int nbytes)
{
     struct socket *sock;
     struct uio auio;
     struct iovec aiov;
     long cnt, error = 0;

     MU_LOCK(kern_lock_p);
     sock=anp_fdfind(fd);
     aiov.iov_base = (caddr_t) buf;
     aiov.iov_len = nbytes;
     auio.uio_iov = &aiov;
     auio.uio_iovcnt = 1;
     auio.uio_resid = nbytes;
     auio.uio_rw = UIO_WRITE;
     auio.uio_segflg = UIO_USERSPACE;

     cnt = nbytes;
     if (error = soo_write(sock, &auio)) {
	  if (auio.uio_resid != cnt && (error == ERESTART ||
					error == EINTR || error == EWOULDBLOCK))
	       error = 0;
#ifdef NOPE	/* signals not supported */
	  if (error == EPIPE)
	       psignal(p, SIGPIPE);
#endif
     }
     cnt -= auio.uio_resid;

     anp_errno=error;
     MU_UNLOCK(kern_lock_p);
     if (anp_errno!=0) {
	  return -1;
     } else {
	  return cnt;
     }
}

/*
 * Ioctl system call
 */
int anp_ioctl(int fd,int ucom,caddr_t udata)
{
     struct socket *sock;
     register int com, error;
     register u_int size;
     caddr_t data, memp;
     int tmp;
#define STK_PARAMS	128
     char stkbuf[STK_PARAMS];

     MU_LOCK(kern_lock_p);
     sock=anp_fdfind(fd);

     com=ucom;
/* AC: since we don't support exec() in sim. environment, we don't support
 * these ioctl's either...
 */
#ifdef NOPE
     switch (com = ucom) {
     case FIONCLEX:
	  fdp->fd_ofileflags[uap->fd] &= ~UF_EXCLOSE;
	  MU_UNLOCK(kern_lock_p);
	  return (0);
     case FIOCLEX:
	  fdp->fd_ofileflags[uap->fd] |= UF_EXCLOSE;
	  MU_UNLOCK(kern_lock_p);
	  return (0);
     }
#endif

     /*
      * Interpret high order word to find amount of data to be
      * copied to/from the user's address space.
      */
     size = IOCPARM_LEN(com);
     if (size > IOCPARM_MAX) {
	  anp_errno=ENOTTY;
	  MU_UNLOCK(kern_lock_p);
	  return -1;
     }
     memp = NULL;
     if (size > sizeof (stkbuf)) {
	  memp = (caddr_t)anp_sys_malloc((u_long)size, M_IOCTLOPS, M_WAITOK);
	  data = memp;
     } else
	  data = stkbuf;
     if (com&IOC_IN) {
	  if (size) {
	       error = copyin(udata, data, (u_int)size);
	       if (error) {
		    if (memp)
			 anp_sys_free(memp, M_IOCTLOPS);
		    anp_errno=error;
		    MU_UNLOCK(kern_lock_p);
		    return -1;
	       }
	  } else
	       *(caddr_t *)data = udata;
     } else if ((com&IOC_OUT) && size)
	  /*
	   * Zero the buffer so the user always
	   * gets back something deterministic.
	   */
	  bzero(data, size);
     else if (com&IOC_VOID)
	  *(caddr_t *)data = udata;

     switch (com) {

     case FIONBIO:
     case FIOASYNC:
	  tmp = *((int *) data);
	  error = (soo_ioctl)(sock, com, (caddr_t)&tmp);
	  break;
#ifdef NOPE
     case FIOSETOWN:
	  tmp = *((int *)data);
	  if (fp->f_type == DTYPE_SOCKET) {
	       ((struct socket *)fp->f_data)->so_pgid = tmp;
	       error = 0;
	       break;
	  }
	  if (tmp <= 0) {
	       tmp = -tmp;
	  } else {
	       struct proc *p1 = pfind(tmp);
	       if (p1 == 0) {
		    error = ESRCH;
		    break;
	       }
	       tmp = p1->p_pgrp->pg_id;
	  }
	  error = (*fp->f_ops->fo_ioctl)
	       (fp, (int)TIOCSPGRP, (caddr_t)&tmp, p);
	  break;

     case FIOGETOWN:
	  if (fp->f_type == DTYPE_SOCKET) {
	       error = 0;
	       *(int *)data = ((struct socket *)fp->f_data)->so_pgid;
	       break;
	  }
	  error = (*fp->f_ops->fo_ioctl)(fp, (int)TIOCGPGRP, data, p);
	  *(int *)data = -*(int *)data;
	  break;

#endif
     default:
	  error = soo_ioctl(sock, com, data);
	  /*
	   * Copy any data to user, size was
	   * already set and checked above.
	   */
	  if (error == 0 && (com&IOC_OUT) && size)
	       error = copyout(data, udata, (u_int)size);
	  break;
     }
     if (memp)
	  anp_sys_free(memp, M_IOCTLOPS);
     anp_errno=error;
     MU_UNLOCK(kern_lock_p);
     if (anp_errno!=0) {
	  return -1;
     } else {
	  return 0;
     }
}

/* close system call: */
int anp_close(int fd)
{
     struct socket *sock;

     MU_LOCK(kern_lock_p);
     sock=anp_fdfind(fd);
     soclose(sock);
     anp_fdfree(fd);
     MU_UNLOCK(kern_lock_p);
     return 0;
}


#ifdef LATER /* AC: implement select() system call at some later date... */
int	selwait, nselcoll;

/*
 * Select system call.
 */
struct select_args {
	u_int	nd;
	fd_set	*in, *ou, *ex;
	struct	timeval *tv;
};
SYSCALL_PREFIX(select)(p, uap, retval)
	register struct proc *p;
	register struct select_args *uap;
	int *retval;
{
	fd_set ibits[3], obits[3];
	struct timeval atv;
	int s, ncoll, error = 0, timo;
	u_int ni;

	bzero((caddr_t)ibits, sizeof(ibits));
	bzero((caddr_t)obits, sizeof(obits));
	if (uap->nd > FD_SETSIZE)
		return (EINVAL);
	if (uap->nd > p->p_fd->fd_nfiles)
		uap->nd = p->p_fd->fd_nfiles;	/* forgiving; slightly wrong */
	ni = howmany(uap->nd, NFDBITS) * sizeof(fd_mask);

#define	getbits(name, x) \
	if (uap->name && \
	    (error = copyin((caddr_t)uap->name, (caddr_t)&ibits[x], ni))) \
		goto done;
	getbits(in, 0);
	getbits(ou, 1);
	getbits(ex, 2);
#undef	getbits

	if (uap->tv) {
		error = copyin((caddr_t)uap->tv, (caddr_t)&atv,
			sizeof (atv));
		if (error)
			goto done;
		if (itimerfix(&atv)) {
			error = EINVAL;
			goto done;
		}
		s = splclock();
		timevaladd(&atv, (struct timeval *)&anp_sys_time);
		timo = hzto(&atv);
		/*
		 * Avoid inadvertently sleeping forever.
		 */
		if (timo == 0)
			timo = 1;
		splx(s);
	} else
		timo = 0;
retry:
	ncoll = nselcoll;
	p->p_flag |= P_SELECT;
	error = selscan(p, ibits, obits, uap->nd, retval);
	if (error || *retval)
		goto done;
	s = splhigh();
	/* this should be timercmp(&anp_sys_time, &atv, >=) */
	if (uap->tv && (anp_sys_time.tv_sec > atv.tv_sec ||
	    anp_sys_time.tv_sec == atv.tv_sec && anp_sys_time.tv_usec >= atv.tv_usec)) {
		splx(s);
		goto done;
	}
	if ((p->p_flag & P_SELECT) == 0 || nselcoll != ncoll) {
		splx(s);
		goto retry;
	}
	p->p_flag &= ~P_SELECT;
	error = tsleep((caddr_t)&selwait, PSOCK | PCATCH, "select", timo);
	splx(s);
	if (error == 0)
		goto retry;
done:
	p->p_flag &= ~P_SELECT;
	/* select is not restarted after signals... */
	if (error == ERESTART)
		error = EINTR;
	if (error == EWOULDBLOCK)
		error = 0;
#define	putbits(name, x) \
	if (uap->name && \
	    (error2 = copyout((caddr_t)&obits[x], (caddr_t)uap->name, ni))) \
		error = error2;
	if (error == 0) {
		int error2;

		putbits(in, 0);
		putbits(ou, 1);
		putbits(ex, 2);
#undef putbits
	}
	return (error);
}

selscan(p, ibits, obits, nfd, retval)
	struct proc *p;
	fd_set *ibits, *obits;
	int nfd, *retval;
{
	register struct filedesc *fdp = p->p_fd;
	register int msk, i, j, fd;
	register fd_mask bits;
	struct file *fp;
	int n = 0;
	static int flag[3] = { FREAD, FWRITE, 0 };

	for (msk = 0; msk < 3; msk++) {
		for (i = 0; i < nfd; i += NFDBITS) {
			bits = ibits[msk].fds_bits[i/NFDBITS];
			while ((j = ffs(bits)) && (fd = i + --j) < nfd) {
				bits &= ~(1 << j);
				fp = fdp->fd_ofiles[fd];
				if (fp == NULL)
					return (EBADF);
				if ((*fp->f_ops->fo_select)(fp, flag[msk], p)) {
					FD_SET(fd, &obits[msk]);
					n++;
				}
			}
		}
	}
	*retval = n;
	return (0);
}

/*ARGSUSED*/
seltrue(dev, flag, p)
	dev_t dev;
	int flag;
	struct proc *p;
{

	return (1);
}

/*
 * Record a select request.
 */
void
selrecord(selector, sip)
	struct proc *selector;
	struct selinfo *sip;
{
	struct proc *p;
	pid_t mypid;

	mypid = selector->p_pid;
	if (sip->si_pid == mypid)
		return;
	if (sip->si_pid && (p = pfind(sip->si_pid)) &&
	    p->p_wchan == (caddr_t)&selwait)
		sip->si_flags |= SI_COLL;
	else
		sip->si_pid = mypid;
}

/*
 * Do a wakeup when a selectable event occurs.
 */
void
selwakeup(sip)
	register struct selinfo *sip;
{
	register struct proc *p;
	int s;

	if (sip->si_pid == 0)
		return;
	if (sip->si_flags & SI_COLL) {
		nselcoll++;
		sip->si_flags &= ~SI_COLL;
		wakeup((caddr_t)&selwait);
	}
	p = pfind(sip->si_pid);
	sip->si_pid = 0;
	if (p != NULL) {
		s = splhigh();
		if (p->p_wchan == (caddr_t)&selwait) {
			if (p->p_stat == SSLEEP)
				setrunnable(p);
			else
				unsleep(p);
		} else if (p->p_flag & P_SELECT)
			p->p_flag &= ~P_SELECT;
		splx(s);
	}
}

#endif /* LATER */

/* socket-specific system calls (dispatch routines to call into
 * kern/uipc_syscalls.c
 */

struct socket_args {
	int	domain;
	int	type;
	int	protocol;
};
int anp_socket(int domain,int type,int protocol)
{
     struct socket_args sargs;
     int retval=0;

     MU_LOCK(kern_lock_p);
     sargs.domain=domain;
     sargs.type=type;
     sargs.protocol=protocol;
     
     if (anp_errno=anp_sys_socket(&sargs,&retval)) {
	  MU_UNLOCK(kern_lock_p);
	  return -1;
     }
     MU_UNLOCK(kern_lock_p);
     return retval;
}

struct bind_args {
	int	s;
	caddr_t	name;
	int	namelen;
};
int anp_bind(int s,caddr_t name,int namelen)
{
     struct bind_args bargs;
     int retval=0;

     MU_LOCK(kern_lock_p);
     bargs.s=s;
     bargs.name=name;
     bargs.namelen=namelen;
     if (anp_errno=anp_sys_bind(&bargs,&retval)) {
	  MU_UNLOCK(kern_lock_p);
	  return -1;
     }
     MU_UNLOCK(kern_lock_p);
     return retval;
}

struct listen_args {
     int	s;
     int	backlog;
};
int anp_listen(int s,int backlog)
{
     struct listen_args largs;
     int retval=0;

     MU_LOCK(kern_lock_p);
     largs.s=s;
     largs.backlog=backlog;
     if (anp_errno=anp_sys_listen(&largs,&retval)) {
	  MU_UNLOCK(kern_lock_p);
	  return -1;
     }
     MU_UNLOCK(kern_lock_p);
     return retval;
}

struct accept_args {
     int	s;
     caddr_t	name;
     int	*anamelen;
};

int anp_accept(int s,caddr_t name,int *anamelen)
{
     struct accept_args aargs;
     int retval=0;

     MU_LOCK(kern_lock_p);
     aargs.s=s;
     aargs.name=name;
     aargs.anamelen=anamelen;

     if (anp_errno=anp_sys_accept(&aargs,&retval)) {
	  MU_UNLOCK(kern_lock_p);
	  return -1;
     }
     MU_UNLOCK(kern_lock_p);
     return retval;
}

struct connect_args {
     int	s;
     caddr_t	name;
     int	namelen;
};
int anp_connect(int s,caddr_t name,int namelen)
{
     struct connect_args cargs;
     int retval=0;

     MU_LOCK(kern_lock_p);
     cargs.s=s;
     cargs.name=name;
     cargs.namelen=namelen;
     if (anp_errno=anp_sys_connect(&cargs,&retval)) {
	  MU_UNLOCK(kern_lock_p);
	  return -1;
     }
     MU_UNLOCK(kern_lock_p);
     return retval;
}

struct socketpair_args {
     int	domain;
     int	type;
     int	protocol;
     int	*rsv;
};
int anp_socketpair(int domain,int type,int protocol,int *rsv)
{
     struct socketpair_args spargs;
     int retval;

     MU_LOCK(kern_lock_p);
     spargs.domain=domain;
     spargs.type=type;
     spargs.protocol=protocol;
     spargs.rsv=rsv;

     if (anp_errno=anp_sys_socketpair(&spargs,&retval)) {
	  MU_UNLOCK(kern_lock_p);
	  return -1;
     }
     MU_UNLOCK(kern_lock_p);
     return retval;
}

struct sendto_args {
     int	s;
     caddr_t	buf;
     size_t	len;
     int	flags;
     caddr_t	to;
     int	tolen;
};
int anp_sendto(int s,caddr_t buf,size_t len,int flags,caddr_t to,int tolen)
{
     struct sendto_args sargs;
     int retval=0;

     MU_LOCK(kern_lock_p);
     sargs.s=s;
     sargs.buf=buf;
     sargs.len=len;
     sargs.flags=flags;
     sargs.to=to;
     sargs.tolen=tolen;
     if (anp_errno=anp_sys_sendto(&sargs,&retval)) {
	  MU_UNLOCK(kern_lock_p);
	  return -1;
     }
     MU_UNLOCK(kern_lock_p);
     return retval;
}

struct sendmsg_args {
     int	s;
     caddr_t	msg;
     int	flags;
};
int anp_sendmsg(int s,caddr_t msg,int flags)
{
     struct sendmsg_args sargs;
     int retval=0;

     MU_LOCK(kern_lock_p);
     sargs.s=s;
     sargs.msg=msg;
     sargs.flags=flags;
     if (anp_errno=anp_sys_sendmsg(&sargs,&retval)) {
	  MU_UNLOCK(kern_lock_p);
	  return -1;
     }
     MU_UNLOCK(kern_lock_p);
     return retval;
}

struct recvfrom_args {
     int	s;
     caddr_t	buf;
     size_t	len;
     int	flags;
     caddr_t	from;
     int	*fromlenaddr;
};
int anp_recvfrom(int s,caddr_t buf,size_t len,int flags,caddr_t from,
		 int *fromlenaddr)
{
     struct recvfrom_args rargs;
     int retval=0;

     MU_LOCK(kern_lock_p);
     rargs.s=s;
     rargs.buf=buf;
     rargs.len=len;
     rargs.flags=flags;
     rargs.from=from;
     rargs.fromlenaddr=fromlenaddr;
     if (anp_errno=anp_sys_recvfrom(&rargs,&retval)) {
	  MU_UNLOCK(kern_lock_p);
	  return -1;
     }
     MU_UNLOCK(kern_lock_p);
     return retval;
}

struct recvmsg_args {
	int	s;
	struct	msghdr *msg;
	int	flags;
};
int anp_recvmsg(int s,struct msghdr *msg,int flags)
{
     struct recvmsg_args rargs;
     int retval=0;

     MU_LOCK(kern_lock_p);
     rargs.s=s;
     rargs.msg=msg;
     rargs.flags=flags;
     if (anp_errno=anp_sys_recvmsg(&rargs,&retval)) {
	  MU_UNLOCK(kern_lock_p);
	  return -1;
     }
     MU_UNLOCK(kern_lock_p);
     return retval;
}

struct shutdown_args {
	int	s;
	int	how;
};
int anp_shutdown(int s,int how)
{
     struct shutdown_args sargs;
     int retval=0;

     MU_LOCK(kern_lock_p);
     sargs.s=s;
     sargs.how=how;
     if (anp_errno=anp_sys_shutdown(&sargs,&retval)) {
	  MU_UNLOCK(kern_lock_p);
	  return -1;
     }
     MU_UNLOCK(kern_lock_p);
     return retval;
}

struct setsockopt_args {
	int	s;
	int	level;
	int	name;
	caddr_t	val;
	int	valsize;
};
int anp_setsockopt(int s,int level,int name,caddr_t val,int valsize)
{
     struct setsockopt_args sargs;
     int retval=0;

     MU_LOCK(kern_lock_p);
     sargs.s=s;
     sargs.level=level;
     sargs.name=name;
     sargs.val=val;
     sargs.valsize=valsize;

     if (anp_errno=anp_sys_setsockopt(&sargs,&retval)) {
	  MU_UNLOCK(kern_lock_p);
	  return -1;
     }
     MU_UNLOCK(kern_lock_p);
     return retval;
}

struct getsockopt_args {
	int	s;
	int	level;
	int	name;
	caddr_t	val;
	int	*avalsize;
};
int anp_getsockopt(int s,int level,int name,caddr_t val,int *avalsize)
{
     struct getsockopt_args gargs;
     int retval=0;

     MU_LOCK(kern_lock_p);
     gargs.s=s;
     gargs.level=level;
     gargs.name=name;
     gargs.val=val;
     gargs.avalsize=avalsize;
     if (anp_errno=anp_sys_getsockopt(&gargs,&retval)) {
	  MU_UNLOCK(kern_lock_p);
	  return -1;
     }
     MU_UNLOCK(kern_lock_p);
     return retval;
}

int anp_pipe(int fildes[2])
{
     int retcode;

     MU_LOCK(kern_lock_p);
     if (anp_errno=anp_sys_pipe(NULL,fildes)) {
	  MU_UNLOCK(kern_lock_p);
	  return -1;
     }
     MU_UNLOCK(kern_lock_p);
     return 0;
}

struct getsockname_args {
	int	fdes;
	caddr_t	asa;
	int	*alen;
};
int anp_getsockname(int fdes,caddr_t asa,int *alen)
{
     struct getsockname_args gargs;
     int retval=0;

     MU_LOCK(kern_lock_p);
     gargs.fdes=fdes;
     gargs.asa=asa;
     gargs.alen=alen;
     if (anp_errno=anp_sys_getsockname(&gargs,&retval)) {
	  MU_UNLOCK(kern_lock_p);
	  return -1;
     }
     MU_UNLOCK(kern_lock_p);
     return retval;
}

struct getpeername_args {
	int	fdes;
	caddr_t	asa;
	int	*alen;
};
int anp_getpeername(int fdes,caddr_t asa,int *alen)
{
     struct getpeername_args gpargs;
     int retval=0;

     MU_LOCK(kern_lock_p);
     gpargs.fdes=fdes;
     gpargs.asa=asa;
     gpargs.alen=alen;
     if (anp_errno=anp_sys_getpeername(&gpargs,&retval)) {
	  MU_UNLOCK(kern_lock_p);
	  return -1;
     }
     MU_UNLOCK(kern_lock_p);
     return retval;
}


