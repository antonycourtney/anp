/*
 * serline.c -- A psuedo-serial line over a SLIP link
 *
 * N.B. Should be compiled with BSD headers
 */

#include <stdlib.h>
#include <sys/types.h>
#include <sys/tty.h>
#include <sys/aclist.h>

#include "spipe.h"
#include "util.h"
#include "mt_bsd.h"

#define SERPATH	"/tmp/Stest"

extern int servflag, cliflag, portnum;

/* serial link file descriptor: */
static int ser_fd;

/* begin output on serial link (called from slstart) */
void ser_oproc(struct tty *tp)
{
     struct acblock *acb;
     int nbytes;
     
     /* (just write all of the bytes in tp->t_outq) on to the stream
      * pipe;  We really want to do better than this in a real implementation,
      * where blocking other processes is unacceptable
      */
     while (tp->t_outq.c_cc > 0) {
	  acb=(struct acblock *) tp->t_outq.c_cf;
	  if ((nbytes=write(ser_fd,acb->ac_data + acb->ac_start,
			    acb->ac_len)) < 0) {
	       perror("write");
	       exit(1);
	  }
	  acb->ac_len -= nbytes;
	  acb->ac_start += nbytes;
	  if (acb->ac_len==0) {
	       if (acb->ac_next==NULL) {
		    tp->t_outq.c_cf=tp->t_outq.c_cl=NULL;
	       } else {
		    tp->t_outq.c_cf=(char *) acb->ac_next;
	       }
	       free(acb);
	  }
	  tp->t_outq.c_cc -= nbytes;
     }
}

void ser_init(void)
{
     struct tty *tp;
     int servfd;
     extern int anp_errno;
     int error;

     if (servflag) {
	  servfd=serv_listen(SERPATH);
	  printf("sltest: ser_init() awaiting connection on %s\n", SERPATH);
	  ser_fd=serv_accept(servfd);
	  printf("sltest: ser_init() accepted connection.\n");
     } else {
	  printf("sltest: ser_init(): attempting to connect to %s\n", SERPATH);
	  ser_fd=cli_conn(SERPATH);
	  printf("sltest: ser_init(): serial link connected.\n");
     }

     /* now, wrap our ser_fd in a fake tty structure: */
     tp=safe_malloc(sizeof(struct tty));
     memset(tp,0,sizeof(*tp));

     tp->t_sc=NULL;
     tp->t_ospeed=38400;
     tp->t_state=TS_CARR_ON;
     tp->t_oproc=ser_oproc;

     /* and emulate what would happen as result of slattach attaching SLIP
      * line discipline:
      */
     if ((error=slopen(NULL,tp))!=0) {
	  anp_errno=error;
	  panic("slopen failed");
     }
     printf("sltest: ser_init(): SLIP line discipline attached.\n");

     /* start a background thread to deal with serial device */
     spipe_start_input_thread(ser_fd,tp);
}
     
     
