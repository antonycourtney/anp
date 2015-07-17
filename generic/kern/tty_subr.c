/*-
 * Copyright (c) 1982, 1986, 1993
 *	The Regents of the University of California.  All rights reserved.
 * (c) UNIX System Laboratories, Inc.
 * All or some portions of this file are derived from material licensed
 * to the University of California by American Telephone and Telegraph
 * Co. or Unix System Laboratories, Inc. and are reproduced herein with
 * the permission of UNIX System Laboratories, Inc.
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
 *	from: @(#)tty_subr.c	8.2 (Berkeley) 9/5/93
 */

#include <sys/param.h>
#include <sys/ioctl.h>
#include <sys/tty.h>
#include <sys/aclist.h>

char cwaiting;
struct cblock *cfree, *cfreelist;
int cfreecount, nclist;

void
clist_init()
{
     /*
      * Body deleted.
      */
     return;
}

getc(a1)
struct clist *a1;
{

     /*
      * Body deleted.
      */
     abort();
     return ((char)0);
}

q_to_b(a1, a2, a3)
struct clist *a1;
char *a2;
int a3;
{
     /*
      * Body deleted.
      */
     abort();
     return (0);
}

ndqb(a1, a2)
struct clist *a1;
int a2;
{

     /*
      * Body deleted.
      */
     abort();
     return (0);
}

void
ndflush(a1, a2)
struct clist *a1;
int a2;
{
     /*
      * Body deleted.
      */
     abort();
     return;
}

/* putc(): put a character on a C-list */

putc(a1, a2)
char a1;
struct clist *a2;
{
     struct acblock *acb, *ncb;
     
     if (a2->c_cc==0) {
	  acb=(struct acblock *) safe_malloc(sizeof(struct acblock));
	  a2->c_cf=(char *) acb;
	  a2->c_cl=(char *) acb;
	  acb->ac_next=NULL;
	  acb->ac_start=acb->ac_len=0;
     } else {
	  /* is there space in acblock at end? */
	  acb=(struct acblock *) a2->c_cl;
	  if ((acb->ac_start + acb->ac_len) >= ACBUFSIZE ) {
	       /* nope, need another block */
	       ncb=(struct acblock *) safe_malloc(sizeof(struct acblock));
	       ncb->ac_next=NULL;
	       ncb->ac_start=ncb->ac_len=0;
	       acb->ac_next=ncb;
	       acb=ncb;
	       a2->c_cl=(char *) acb;
	  }
     }
     a2->c_cc++;
     acb->ac_data[(acb->ac_start) + acb->ac_len++]=a1;
     return (0);
}

b_to_q(buf,nbytes,cl)
char *buf;
int nbytes;
struct clist *cl;
{
     struct acblock *acb, *ncb;
     int ncopy;

     cl->c_cc += nbytes;
     while (nbytes > 0) {
	  /* still more characters to add to clist */

	  /* create new acblock with as many bytes from a1 as possible */
	  ncb=(struct acblock *) safe_malloc(sizeof(struct acblock));
	  ncb->ac_next=NULL;
	  ncb->ac_start=ncb->ac_len=0;

	  ncopy = (nbytes > ACBUFSIZE) ? ACBUFSIZE : nbytes;
	  memcpy(ncb->ac_data,buf,ncopy);
	  ncb->ac_len=ncopy;

	  nbytes -= ncopy;
	  buf += ncopy;

	  /* add cblock to clist */
	  if (cl->c_cl==NULL) {
	       cl->c_cf=cl->c_cl=(char *) ncb;
	  } else {
	       acb=(struct acblock *) cl->c_cl;
	       acb->ac_next=ncb;
	       cl->c_cl=(char *) ncb;
	  }
     }
     return 0;
}

char *
nextc(a1, a2, a3)
struct clist *a1;
char *a2;
int *a3;
{

     /*
      * Body deleted.
      */
     abort();
     return ((char *)0);
}

unputc(a1)
struct clist *a1;
{

     /*
      * Body deleted.
      */
     abort();
     return ((char)0);
}

void
catq(a1, a2)
struct clist *a1, *a2;
{

     /*
      * Body deleted.
      */
     abort();
     return;
}

/* AC: moved these here from tty.c just to get a link working */
void ttyflush(struct tty *tp,int flags)
{
}

int ttywflush(struct tty *tp)
{
}
