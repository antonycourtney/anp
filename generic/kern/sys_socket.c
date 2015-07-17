/*
 * Copyright (c) 1982, 1986, 1990, 1993
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
 *	@(#)sys_socket.c	8.1 (Berkeley) 6/10/93
 */

#include <sys/param.h>
#include <sys/systm.h>
#include <sys/proc.h>
#include <sys/file.h>
#include <sys/mbuf.h>
#include <sys/protosw.h>
#include <sys/socket.h>
#include <sys/socketvar.h>
#include <sys/ioctl.h>
#include <sys/stat.h>

#include <net/if.h>
#include <net/route.h>

#ifdef NOPE /* not used anymore */
struct	fileops socketops =
    { soo_read, soo_write, soo_ioctl, soo_select, soo_close };
#endif


/* ARGSUSED */
int soo_read(struct socket *sock, struct uio *uio)
{

     return (soreceive(sock, (struct mbuf **)0, uio,
		       (struct mbuf **)0, (struct mbuf **)0, (int *)0));
}

/* ARGSUSED */
int soo_write(struct socket *sock, struct uio * uio)
{
     return (sosend(sock, (struct mbuf *)0, uio, (struct mbuf *)0,
		    (struct mbuf *)0, 0));
}

int soo_ioctl(struct socket *so,int cmd,caddr_t data)
{
     switch (cmd) {

     case FIONBIO:
	  if (*(int *)data)
	       so->so_state |= SS_NBIO;
	  else
	       so->so_state &= ~SS_NBIO;
	  return (0);

     case FIOASYNC:
	  if (*(int *)data) {
	       so->so_state |= SS_ASYNC;
	       so->so_rcv.sb_flags |= SB_ASYNC;
	       so->so_snd.sb_flags |= SB_ASYNC;
	  } else {
	       so->so_state &= ~SS_ASYNC;
	       so->so_rcv.sb_flags &= ~SB_ASYNC;
	       so->so_snd.sb_flags &= ~SB_ASYNC;
	  }
	  return (0);

     case FIONREAD:
	  *(int *)data = so->so_rcv.sb_cc;
     return (0);

     case SIOCSPGRP:
	  so->so_pgid = *(int *)data;
	  return (0);

     case SIOCGPGRP:
	  *(int *)data = so->so_pgid;
     return (0);

     case SIOCATMARK:
	  *(int *)data = (so->so_state&SS_RCVATMARK) != 0;
     return (0);
     }
     /*
      * Interface/routing/protocol specific ioctls:
      * interface and routing ioctls should have a
      * different entry since a socket's unnecessary
      */
     if (IOCGROUP(cmd) == 'i')
	  return (ifioctl(so, cmd, data));
     if (IOCGROUP(cmd) == 'r')
	  return (rtioctl(cmd, data));
     return ((*so->so_proto->pr_usrreq)(so, PRU_CONTROL, 
					(struct mbuf *)cmd, (struct mbuf *)data, (struct mbuf *)0));
}


int soo_select(struct socket *so,int which)
{
#ifdef LATER /* We don't support select() yet in ANP */
     register int s = splnet();

     switch (which) {

     case FREAD:
	  if (soreadable(so)) {
	       splx(s);
	       return (1);
	  }
	  selrecord(p, &so->so_rcv.sb_sel);
	  so->so_rcv.sb_flags |= SB_SEL;
	  break;

     case FWRITE:
	  if (sowriteable(so)) {
	       splx(s);
	       return (1);
	  }
	  selrecord(p, &so->so_snd.sb_sel);
	  so->so_snd.sb_flags |= SB_SEL;
	  break;

     case 0:
	  if (so->so_oobmark || (so->so_state & SS_RCVATMARK)) {
	       splx(s);
	       return (1);
	  }
	  selrecord(p, &so->so_rcv.sb_sel);
	  so->so_rcv.sb_flags |= SB_SEL;
	  break;
     }
     splx(s);
     return (0);
#endif	/* LATER */
     panic("soo_select: unimplemented");
}

int soo_stat(so, ub)
	register struct socket *so;
	register struct stat *ub;
{

	bzero((caddr_t)ub, sizeof (*ub));
	ub->st_mode = S_IFSOCK;
	return ((*so->so_proto->pr_usrreq)(so, PRU_SENSE,
	    (struct mbuf *)ub, (struct mbuf *)0, 
	    (struct mbuf *)0));
}

/* ARGSUSED */
int soo_close(struct socket *sock)
{
     return soclose(sock);
}
