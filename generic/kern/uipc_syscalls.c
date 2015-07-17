/*
 * Copyright (c) 1982, 1986, 1989, 1990, 1993
 *	The Regents of the University of California.  All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. All advertising materials mentioning features or use of this software
 *    must display the following acknowledgement:
 *	This product includes software developed by the University of
 *	California, Berkeley and its contributors.
 * 4. Neither the name of the University nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE REGENTS AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE REGENTS OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 *	@(#)uipc_syscalls.c	8.4 (Berkeley) 2/21/94
 */

#include <sys/param.h>
#include <sys/filedesc.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/buf.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#ifdef KTRACE
#include <sys/ktrace.h>
#endif

#include "anp_fdtbl.h"

/* AC:
 * modified 4/10/96, to add anp_sys_ prefix to all system calls, through use
 * of SYSCALL_PREFIX() macro:
 */
#define SYSCALL_PREFIX(nm)	anp_sys_ ## nm


/*
 * System call interface to the socket abstraction.
 */
#if defined(COMPAT_43) || defined(COMPAT_SUNOS)
#define COMPAT_OLDSOCK
#endif

extern	struct fileops socketops;

struct socket_args {
	int	domain;
	int	type;
	int	protocol;
};
SYSCALL_PREFIX(socket)(uap, retval)
	register struct socket_args *uap;
	int *retval;
{
	struct socket *so;
	int fd, error;

	error = socreate(uap->domain, &so, uap->type, uap->protocol);
	if (!error) {
	     fd=anp_fdalloc(so);
	     *retval=fd;
	}
	return (error);
}

struct bind_args {
	int	s;
	caddr_t	name;
	int	namelen;
};
/* ARGSUSED */
SYSCALL_PREFIX(bind)(uap, retval)
	register struct bind_args *uap;
	int *retval;
{
     struct socket *sock;
     struct mbuf *nam;
     int error;

     sock=anp_fdfind(uap->s);
     if (error = sockargs(&nam, uap->name, uap->namelen, MT_SONAME))
	  return (error);
     error = sobind(sock, nam);
     m_freem(nam);
     return (error);
}

struct listen_args {
     int	s;
     int	backlog;
};
/* ARGSUSED */
SYSCALL_PREFIX(listen)(uap, retval)
register struct listen_args *uap;
int *retval;
{
     struct socket *sock;
     int error;

     sock=anp_fdfind(uap->s);
     return (solisten(sock, uap->backlog));
}

struct accept_args {
     int	s;
     caddr_t	name;
     int	*anamelen;
};

SYSCALL_PREFIX(accept)(uap, retval)
register struct accept_args *uap;
int *retval;
{
     struct mbuf *nam;
     int namelen, error, s;
     register struct socket *so;

     if (uap->name && (error = copyin((caddr_t)uap->anamelen,
				      (caddr_t)&namelen, sizeof (namelen))))
	  return (error);
     so=anp_fdfind(uap->s);
     s = splnet();
     if ((so->so_options & SO_ACCEPTCONN) == 0) {
	  splx(s);
	  return (EINVAL);
     }
     if ((so->so_state & SS_NBIO) && so->so_qlen == 0) {
	  splx(s);
	  return (EWOULDBLOCK);
     }
     while (so->so_qlen == 0 && so->so_error == 0) {
	  if (so->so_state & SS_CANTRCVMORE) {
	       so->so_error = ECONNABORTED;
	       break;
	  }
	  if (error = tsleep((caddr_t)&so->so_timeo, PSOCK | PCATCH,
			     netcon, 0)) {
	       splx(s);
	       return (error);
	  }
     }
     if (so->so_error) {
	  error = so->so_error;
	  so->so_error = 0;
	  splx(s);
	  return (error);
     }
     { struct socket *aso = so->so_q;
     if (soqremque(aso, 1) == 0)
	  panic("accept");
     so = aso;
     }
     *retval=anp_fdalloc(so);
     nam = m_get(M_WAIT, MT_SONAME);
     (void) soaccept(so, nam);
     if (uap->name) {
#ifdef COMPAT_OLDSOCK
	  if (uap->compat_43)
	       mtod(nam, struct osockaddr *)->sa_family =
	       mtod(nam, struct sockaddr *)->sa_family;
#endif
	  if (namelen > nam->m_len)
	       namelen = nam->m_len;
	  /* SHOULD COPY OUT A CHAIN HERE */
	  if ((error = copyout(mtod(nam, caddr_t), (caddr_t)uap->name,
			       (u_int)namelen)) == 0)
	       error = copyout((caddr_t)&namelen,
			       (caddr_t)uap->anamelen, sizeof (*uap->anamelen));
     }
     m_freem(nam);
     splx(s);
     return (error);
}

struct connect_args {
     int	s;
     caddr_t	name;
     int	namelen;
};
/* ARGSUSED */
SYSCALL_PREFIX(connect)(uap, retval)
register struct connect_args *uap;
int *retval;
{
     register struct socket *so;
     struct mbuf *nam;
     int error, s;

     so=anp_fdfind(uap->s);
     if ((so->so_state & SS_NBIO) && (so->so_state & SS_ISCONNECTING))
	  return (EALREADY);
     if (error = sockargs(&nam, uap->name, uap->namelen, MT_SONAME))
	  return (error);
     error = soconnect(so, nam);
     if (error)
	  goto bad;
     if ((so->so_state & SS_NBIO) && (so->so_state & SS_ISCONNECTING)) {
	  m_freem(nam);
	  return (EINPROGRESS);
     }
     s = splnet();
     while ((so->so_state & SS_ISCONNECTING) && so->so_error == 0)
	  if (error = tsleep((caddr_t)&so->so_timeo, PSOCK | PCATCH,
			     netcon, 0))
	       break;
     if (error == 0) {
	  error = so->so_error;
	  so->so_error = 0;
     }
     splx(s);
bad:
     so->so_state &= ~SS_ISCONNECTING;
     m_freem(nam);
     if (error == ERESTART)
	  error = EINTR;
     return (error);
}

struct socketpair_args {
     int	domain;
     int	type;
     int	protocol;
     int	*rsv;
};
SYSCALL_PREFIX(socketpair)(uap, retval)
register struct socketpair_args *uap;
int retval[];
{
     struct socket *so1, *so2;
     int fd, error, sv[2];

     /* TODO: really should hold on to both fd's, so that we can call
      * anp_fdfree() when/if an error occurs
      */
     if (error = socreate(uap->domain, &so1, uap->type, uap->protocol))
	  return (error);
     if (error = socreate(uap->domain, &so2, uap->type, uap->protocol))
	  goto free1;
     fd=anp_fdalloc(so1);
     sv[0] = fd;
     fd=anp_fdalloc(so2);
     sv[1] = fd;
     if (error = soconnect2(so1, so2))
	  goto free4;
     if (uap->type == SOCK_DGRAM) {
	  /*
	   * Datagram socket connection is asymmetric.
	   */
	  if (error = soconnect2(so2, so1))
	       goto free4;
     }
     error = copyout((caddr_t)sv, (caddr_t)uap->rsv, 2 * sizeof (int));
     retval[0] = sv[0];		/* XXX ??? */
     retval[1] = sv[1];		/* XXX ??? */
     return (error);
free4:
free3:
free2:
     (void)soclose(so2);
free1:
(void) soclose(so1);
return (error);
}

struct sendto_args {
     int	s;
     caddr_t	buf;
     size_t	len;
     int	flags;
     caddr_t	to;
     int	tolen;
};
SYSCALL_PREFIX(sendto)(uap, retval)
register struct sendto_args *uap;
int *retval;
{
     struct msghdr msg;
     struct iovec aiov;

     msg.msg_name = uap->to;
     msg.msg_namelen = uap->tolen;
     msg.msg_iov = &aiov;
     msg.msg_iovlen = 1;
     msg.msg_control = 0;
#ifdef COMPAT_OLDSOCK
     msg.msg_flags = 0;
#endif
     aiov.iov_base = uap->buf;
     aiov.iov_len = uap->len;
     return (sendit(uap->s, &msg, uap->flags, retval));
}

struct sendmsg_args {
     int	s;
     caddr_t	msg;
     int	flags;
};
SYSCALL_PREFIX(sendmsg)(uap, retval)
register struct sendmsg_args *uap;
int *retval;
{
     struct msghdr msg;
     struct iovec aiov[UIO_SMALLIOV], *iov;
     int error;

     if (error = copyin(uap->msg, (caddr_t)&msg, sizeof (msg)))
	  return (error);
     if ((u_int)msg.msg_iovlen >= UIO_SMALLIOV) {
	  if ((u_int)msg.msg_iovlen >= UIO_MAXIOV)
	       return (EMSGSIZE);
	  MALLOC(iov, struct iovec *,
		 sizeof(struct iovec) * (u_int)msg.msg_iovlen, M_IOV,
		 M_WAITOK);
     } else
	  iov = aiov;
     if (msg.msg_iovlen &&
	 (error = copyin((caddr_t)msg.msg_iov, (caddr_t)iov,
			 (unsigned)(msg.msg_iovlen * sizeof (struct iovec)))))
	  goto done;
     msg.msg_iov = iov;
#ifdef COMPAT_OLDSOCK
     msg.msg_flags = 0;
#endif
     error = sendit(uap->s, &msg, uap->flags, retval);
done:
     if (iov != aiov)
	  FREE(iov, M_IOV);
     return (error);
}

sendit(s, mp, flags, retsize)
int s;
register struct msghdr *mp;
int flags, *retsize;
{
     struct sock *sock;
     struct uio auio;
     register struct iovec *iov;
     register int i;
     struct mbuf *to, *control;
     int len, error;
#ifdef KTRACE
     struct iovec *ktriov = NULL;
#endif
	
     sock=anp_fdfind(s);
     auio.uio_iov = mp->msg_iov;
     auio.uio_iovcnt = mp->msg_iovlen;
     auio.uio_segflg = UIO_USERSPACE;
     auio.uio_rw = UIO_WRITE;
     auio.uio_offset = 0;			/* XXX */
     auio.uio_resid = 0;
     iov = mp->msg_iov;
     for (i = 0; i < mp->msg_iovlen; i++, iov++) {
	  if (iov->iov_len < 0)
	       return (EINVAL);
	  if ((auio.uio_resid += iov->iov_len) < 0)
	       return (EINVAL);
     }
     if (mp->msg_name) {
	  if (error = sockargs(&to, mp->msg_name, mp->msg_namelen,
			       MT_SONAME))
	       return (error);
     } else
	  to = 0;
     if (mp->msg_control) {
	  if (mp->msg_controllen < sizeof(struct cmsghdr)
#ifdef COMPAT_OLDSOCK
	      && mp->msg_flags != MSG_COMPAT
#endif
	       ) {
	       error = EINVAL;
	       goto bad;
	  }
	  if (error = sockargs(&control, mp->msg_control,
			       mp->msg_controllen, MT_CONTROL))
	       goto bad;
#ifdef COMPAT_OLDSOCK
	  if (mp->msg_flags == MSG_COMPAT) {
	       register struct cmsghdr *cm;

	       M_PREPEND(control, sizeof(*cm), M_WAIT);
	       if (control == 0) {
		    error = ENOBUFS;
		    goto bad;
	       } else {
		    cm = mtod(control, struct cmsghdr *);
		    cm->cmsg_len = control->m_len;
		    cm->cmsg_level = SOL_SOCKET;
		    cm->cmsg_type = SCM_RIGHTS;
	       }
	  }
#endif
     } else
	  control = 0;
#ifdef KTRACE
     if (KTRPOINT(p, KTR_GENIO)) {
	  int iovlen = auio.uio_iovcnt * sizeof (struct iovec);

	  MALLOC(ktriov, struct iovec *, iovlen, M_TEMP, M_WAITOK);
	  bcopy((caddr_t)auio.uio_iov, (caddr_t)ktriov, iovlen);
     }
#endif
     len = auio.uio_resid;
     if (error = sosend(sock, to, &auio,
			(struct mbuf *)0, control, flags)) {
	  if (auio.uio_resid != len && (error == ERESTART ||
					error == EINTR || error == EWOULDBLOCK))
	       error = 0;
/* AC: 4/18/96 */
#ifdef USE_SIGNALS
	  if (error == EPIPE)
	       psignal(p, SIGPIPE);
#endif
     }
     if (error == 0)
	  *retsize = len - auio.uio_resid;
#ifdef KTRACE
     if (ktriov != NULL) {
	  if (error == 0)
	       ktrgenio(p->p_tracep, s, UIO_WRITE,
			ktriov, *retsize, error);
	  FREE(ktriov, M_TEMP);
     }
#endif
bad:
     if (to)
	  m_freem(to);
     return (error);
}

struct recvfrom_args {
     int	s;
     caddr_t	buf;
     size_t	len;
     int	flags;
     caddr_t	from;
     int	*fromlenaddr;
};

SYSCALL_PREFIX(recvfrom)(uap, retval)
	register struct recvfrom_args *uap;
	int *retval;
{
	struct msghdr msg;
	struct iovec aiov;
	int error;

	if (uap->fromlenaddr) {
		if (error = copyin((caddr_t)uap->fromlenaddr,
		    (caddr_t)&msg.msg_namelen, sizeof (msg.msg_namelen)))
			return (error);
	} else
		msg.msg_namelen = 0;
	msg.msg_name = uap->from;
	msg.msg_iov = &aiov;
	msg.msg_iovlen = 1;
	aiov.iov_base = uap->buf;
	aiov.iov_len = uap->len;
	msg.msg_control = 0;
	msg.msg_flags = uap->flags;
	return (recvit(uap->s, &msg, (caddr_t)uap->fromlenaddr, retval));
}

struct recvmsg_args {
	int	s;
	struct	msghdr *msg;
	int	flags;
};
SYSCALL_PREFIX(recvmsg)(uap, retval)
	register struct recvmsg_args *uap;
	int *retval;
{
	struct msghdr msg;
	struct iovec aiov[UIO_SMALLIOV], *uiov, *iov;
	register int error;

	if (error = copyin((caddr_t)uap->msg, (caddr_t)&msg, sizeof (msg)))
		return (error);
	if ((u_int)msg.msg_iovlen >= UIO_SMALLIOV) {
		if ((u_int)msg.msg_iovlen >= UIO_MAXIOV)
			return (EMSGSIZE);
		MALLOC(iov, struct iovec *,
		       sizeof(struct iovec) * (u_int)msg.msg_iovlen, M_IOV,
		       M_WAITOK);
	} else
		iov = aiov;
#ifdef COMPAT_OLDSOCK
	msg.msg_flags = uap->flags &~ MSG_COMPAT;
#else
	msg.msg_flags = uap->flags;
#endif
	uiov = msg.msg_iov;
	msg.msg_iov = iov;
	if (error = copyin((caddr_t)uiov, (caddr_t)iov,
	    (unsigned)(msg.msg_iovlen * sizeof (struct iovec))))
		goto done;
	if ((error = recvit(uap->s, &msg, (caddr_t)0, retval)) == 0) {
		msg.msg_iov = uiov;
		error = copyout((caddr_t)&msg, (caddr_t)uap->msg, sizeof(msg));
	}
done:
	if (iov != aiov)
		FREE(iov, M_IOV);
	return (error);
}

recvit(s, mp, namelenp, retsize)
int s;
register struct msghdr *mp;
caddr_t namelenp;
int *retsize;
{
     struct socket *sock;
     struct uio auio;
     register struct iovec *iov;
     register int i;
     int len, error;
     struct mbuf *from = 0, *control = 0;
#ifdef KTRACE
     struct iovec *ktriov = NULL;
#endif
	
     sock = anp_fdfind(s);
     auio.uio_iov = mp->msg_iov;
     auio.uio_iovcnt = mp->msg_iovlen;
     auio.uio_segflg = UIO_USERSPACE;
     auio.uio_rw = UIO_READ;
     auio.uio_offset = 0;			/* XXX */
     auio.uio_resid = 0;
     iov = mp->msg_iov;
     for (i = 0; i < mp->msg_iovlen; i++, iov++) {
	  if (iov->iov_len < 0)
	       return (EINVAL);
	  if ((auio.uio_resid += iov->iov_len) < 0)
	       return (EINVAL);
     }
#ifdef KTRACE
     if (KTRPOINT(p, KTR_GENIO)) {
	  int iovlen = auio.uio_iovcnt * sizeof (struct iovec);

	  MALLOC(ktriov, struct iovec *, iovlen, M_TEMP, M_WAITOK);
	  bcopy((caddr_t)auio.uio_iov, (caddr_t)ktriov, iovlen);
     }
#endif
     len = auio.uio_resid;
     if (error = soreceive(sock, &from, &auio,
			   (struct mbuf **)0, mp->msg_control ? &control : (struct mbuf **)0,
			   &mp->msg_flags)) {
	  if (auio.uio_resid != len && (error == ERESTART ||
					error == EINTR || error == EWOULDBLOCK))
	       error = 0;
     }
#ifdef KTRACE
     if (ktriov != NULL) {
	  if (error == 0)
	       ktrgenio(p->p_tracep, s, UIO_READ,
			ktriov, len - auio.uio_resid, error);
	  FREE(ktriov, M_TEMP);
     }
#endif
     if (error)
	  goto out;
     *retsize = len - auio.uio_resid;
     if (mp->msg_name) {
	  len = mp->msg_namelen;
	  if (len <= 0 || from == 0)
	       len = 0;
	  else {
#ifdef COMPAT_OLDSOCK
	       if (mp->msg_flags & MSG_COMPAT)
		    mtod(from, struct osockaddr *)->sa_family =
		    mtod(from, struct sockaddr *)->sa_family;
#endif
	       if (len > from->m_len)
		    len = from->m_len;
	       /* else if len < from->m_len ??? */
	       if (error = copyout(mtod(from, caddr_t),
				   (caddr_t)mp->msg_name, (unsigned)len))
		    goto out;
	  }
	  mp->msg_namelen = len;
	  if (namelenp &&
	      (error = copyout((caddr_t)&len, namelenp, sizeof (int)))) {
#ifdef COMPAT_OLDSOCK
	       if (mp->msg_flags & MSG_COMPAT)
		    error = 0;	/* old recvfrom didn't check */
	       else
#endif
		    goto out;
	  }
     }
     if (mp->msg_control) {
#ifdef COMPAT_OLDSOCK
	  /*
	   * We assume that old recvmsg calls won't receive access
	   * rights and other control info, esp. as control info
	   * is always optional and those options didn't exist in 4.3.
	   * If we receive rights, trim the cmsghdr; anything else
	   * is tossed.
	   */
	  if (control && mp->msg_flags & MSG_COMPAT) {
	       if (mtod(control, struct cmsghdr *)->cmsg_level !=
		   SOL_SOCKET ||
		   mtod(control, struct cmsghdr *)->cmsg_type !=
		   SCM_RIGHTS) {
		    mp->msg_controllen = 0;
		    goto out;
	       }
	       control->m_len -= sizeof (struct cmsghdr);
	       control->m_data += sizeof (struct cmsghdr);
	  }
#endif
	  len = mp->msg_controllen;
	  if (len <= 0 || control == 0)
	       len = 0;
	  else {
	       if (len >= control->m_len)
		    len = control->m_len;
	       else
		    mp->msg_flags |= MSG_CTRUNC;
	       error = copyout((caddr_t)mtod(control, caddr_t),
			       (caddr_t)mp->msg_control, (unsigned)len);
	  }
	  mp->msg_controllen = len;
     }
out:
     if (from)
	  m_freem(from);
     if (control)
	  m_freem(control);
     return (error);
}

struct shutdown_args {
	int	s;
	int	how;
};
/* ARGSUSED */
SYSCALL_PREFIX(shutdown)(uap, retval)
     register struct shutdown_args *uap;
     int *retval;
{
     struct socket *sock;
     int error;

     sock=anp_fdfind(uap->s);
     return (soshutdown(sock, uap->how));
}

struct setsockopt_args {
	int	s;
	int	level;
	int	name;
	caddr_t	val;
	int	valsize;
};
/* ARGSUSED */
SYSCALL_PREFIX(setsockopt)(uap, retval)
     register struct setsockopt_args *uap;
     int *retval;
{
     struct socket *sock;
     struct mbuf *m = NULL;
     int error;

     sock=anp_fdfind(uap->s);
     if (uap->valsize > MLEN)
	  return (EINVAL);
     if (uap->val) {
	  m = m_get(M_WAIT, MT_SOOPTS);
	  if (m == NULL)
	       return (ENOBUFS);
	  if (error = copyin(uap->val, mtod(m, caddr_t),
			     (u_int)uap->valsize)) {
	       (void) m_free(m);
	       return (error);
	  }
	  m->m_len = uap->valsize;
     }
     return (sosetopt(sock, uap->level,uap->name, m));
}

struct getsockopt_args {
	int	s;
	int	level;
	int	name;
	caddr_t	val;
	int	*avalsize;
};
/* ARGSUSED */
SYSCALL_PREFIX(getsockopt)(uap, retval)
     register struct getsockopt_args *uap;
     int *retval;
{
     struct socket *sock;
     struct mbuf *m = NULL;
     int valsize, error;

     sock=anp_fdfind(uap->s);

     if (uap->val) {
	  if (error = copyin((caddr_t)uap->avalsize, (caddr_t)&valsize,
			     sizeof (valsize)))
	       return (error);
     } else
	  valsize = 0;
     if ((error = sogetopt(sock, uap->level,uap->name, &m)) == 0 &&
	 uap->val && valsize && m != NULL) {
	  if (valsize > m->m_len)
	       valsize = m->m_len;
	  error = copyout(mtod(m, caddr_t), uap->val, (u_int)valsize);
	  if (error == 0)
	       error = copyout((caddr_t)&valsize,
			       (caddr_t)uap->avalsize, sizeof (valsize));
     }
     if (m != NULL)
	  (void) m_free(m);
     return (error);
}

struct pipe_args {
	int	dummy;
};
/* ARGSUSED */
SYSCALL_PREFIX(pipe)(uap, retval)
	struct pipe_args *uap;
	int retval[];
{
     return (ENOSYS);
/* AC: 4/18/96: no pipes yet, because we don't support unix domain sockets */
#ifdef LATER
	struct socket *rso, *wso;
	int fd, error;

	if (error = socreate(AF_UNIX, &rso, SOCK_STREAM, 0))
		return (error);
	if (error = socreate(AF_UNIX, &wso, SOCK_STREAM, 0))
		goto free1;
	
	fd=anp_fdalloc(rso);
	retval[0] = fd;
	fd=anp_fdalloc(wso);
	retval[1] = fd;
	if (error = unp_connect2(wso, rso))
		goto free4;
	return (0);
free4:
free3:
free2:
	(void)soclose(wso);
free1:
	(void)soclose(rso);
	return (error);
#endif
}

/*
 * Get socket name.
 */
struct getsockname_args {
	int	fdes;
	caddr_t	asa;
	int	*alen;
};

/* ARGSUSED */
SYSCALL_PREFIX(getsockname)(uap, retval)
	register struct getsockname_args *uap;
	int *retval;
{
	register struct socket *so;
	struct mbuf *m;
	int len, error;

	so=anp_fdfind(uap->fdes);
	if (error = copyin((caddr_t)uap->alen, (caddr_t)&len, sizeof (len)))
		return (error);
	m = m_getclr(M_WAIT, MT_SONAME);
	if (m == NULL)
		return (ENOBUFS);
	if (error = (*so->so_proto->pr_usrreq)(so, PRU_SOCKADDR, 0, m, 0))
		goto bad;
	if (len > m->m_len)
		len = m->m_len;
	error = copyout(mtod(m, caddr_t), (caddr_t)uap->asa, (u_int)len);
	if (error == 0)
		error = copyout((caddr_t)&len, (caddr_t)uap->alen,
		    sizeof (len));
bad:
	m_freem(m);
	return (error);
}

/*
 * Get name of peer for connected socket.
 */
struct getpeername_args {
	int	fdes;
	caddr_t	asa;
	int	*alen;
};

/* ARGSUSED */
SYSCALL_PREFIX(getpeername)(uap, retval)
	register struct getpeername_args *uap;
	int *retval;
{
	register struct socket *so;
	struct mbuf *m;
	int len, error;

	so = anp_fdfind(uap->fdes);
	if ((so->so_state & (SS_ISCONNECTED|SS_ISCONFIRMING)) == 0)
		return (ENOTCONN);
	if (error = copyin((caddr_t)uap->alen, (caddr_t)&len, sizeof (len)))
		return (error);
	m = m_getclr(M_WAIT, MT_SONAME);
	if (m == NULL)
		return (ENOBUFS);
	if (error = (*so->so_proto->pr_usrreq)(so, PRU_PEERADDR, 0, m, 0))
		goto bad;
	if (len > m->m_len)
		len = m->m_len;
	if (error = copyout(mtod(m, caddr_t), (caddr_t)uap->asa, (u_int)len))
		goto bad;
	error = copyout((caddr_t)&len, (caddr_t)uap->alen, sizeof (len));
bad:
	m_freem(m);
	return (error);
}

sockargs(mp, buf, buflen, type)
	struct mbuf **mp;
	caddr_t buf;
	int buflen, type;
{
	register struct sockaddr *sa;
	register struct mbuf *m;
	int error;

	if ((u_int)buflen > MLEN) {
#ifdef COMPAT_OLDSOCK
		if (type == MT_SONAME && (u_int)buflen <= 112)
			buflen = MLEN;		/* unix domain compat. hack */
		else
#endif
		return (EINVAL);
	}
	m = m_get(M_WAIT, type);
	if (m == NULL)
		return (ENOBUFS);
	m->m_len = buflen;
	error = copyin(buf, mtod(m, caddr_t), (u_int)buflen);
	if (error)
		(void) m_free(m);
	else {
		*mp = m;
		if (type == MT_SONAME) {
			sa = mtod(m, struct sockaddr *);

#if defined(COMPAT_OLDSOCK) && BYTE_ORDER != BIG_ENDIAN
			if (sa->sa_family == 0 && sa->sa_len < AF_MAX)
				sa->sa_family = sa->sa_len;
#endif
			sa->sa_len = buflen;
		}
	}
	return (error);
}

