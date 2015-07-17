/*
 * init.c -- general initialization for ANP library and user-space kernel
 *	     simulator
 *
 * Antony Courtney,	4/22/96
 */

#include <stdio.h>
#include <pthread.h>

#include "timers.h"
#include "mt.h"


/* taken from kern/init_main.c: */
struct pdevinit {
     void (*pdev_attach)(int);	/* attach function */
     int pdev_count;		/* number of devices */
};

extern void slattach(int);
extern void loopattach(int);

struct pdevinit pdevinit[]={
     { slattach, 1},
     { loopattach, 1},
     { 0, 0}
};

void anp_init(void)
{
     struct pdevinit *pdev;
     int s;

     /* do required setup for thread package (including global locks): */
     mt_init();

     /* start callout_thread and clock_thread */
     start_timers();

     /* kernel initialization: */

     MU_LOCK(&kern_lock);
     /* initialize mbuf module: */
     mbinit();

     /* initialise psuedo-devices (e.g. SLIP and loopback) */
     for (pdev=pdevinit; pdev->pdev_attach!=NULL; pdev++) {
	  (*pdev->pdev_attach)(pdev->pdev_count);
     }

     /* initialize protocols; block reception of incoming packets until
      * everything is ready
      */
     s=splimp();
     ifinit();		/* initialize network interfaces */
     domaininit();	/* initialize protocol domains */
     splx(s);

     MU_UNLOCK(&kern_lock);

     /* ifconfig up the network interfaces: */
     do_ifconfig();

     
}     

