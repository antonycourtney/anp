/*
 * kspl.c -- dummy routines for setting processor priority level
 *
 * N.B. must be compiled w/ BSD headers
 *
 * Since we have a non-preemptive multithreading scenario, these routines
 * are all NOPs
 *
 * Antony Courtney,	4/17/96
 */

#include <net/netisr.h>

/* just for fun, we can keep track of the level */
static int pl=0;

#define pl_0		0
#define pl_softclock	1
#define pl_net		2
#define pl_tty		3
#define pl_bio		4
#define pl_imp		5
#define pl_clock	6
#define pl_high		7

int spl0(void)
{
     int oldpl=pl;

     pl=pl_0;
     return oldpl;
}

int splsoftclock(void)
{
     int oldpl=pl;

     pl=pl_softclock;
     return oldpl;
}

int splnet(void)
{
     int oldpl=pl;

     pl=pl_net;
     return oldpl;
}

int spltty(void)
{
     int oldpl=pl;

     pl=pl_tty;
     return oldpl;
}

int splbio(void)
{
     int oldpl=pl;

     pl=pl_bio;
     return oldpl;
}

int splimp(void)
{
     int oldpl=pl;

     pl=pl_bio;
     return oldpl;
}

int splclock(void)
{
     int oldpl=pl;

     pl=pl_clock;
     return oldpl;
}

int splhigh(void)
{
     int oldpl=pl;

     pl=pl_high;
     return oldpl;
}

void splx(int s)
{
     pl=s;
}


/* AC: setsoftnet() is not commented anywhere, but basically appears to
 * schedule a software interrupt for network processing.
 *
 * The variable netisr apparently indicates which network processing needs
 * to happen
 */
void setsoftnet(void)
{
     signal_sw_intr();
}

/* So we can use SPARC machdep header files: */
void ienab_bis(int x)
{
     signal_sw_intr();
}

/* do_sw_intr() -- do software interrupt processing */
void do_sw_intr(void)
{
     int n, s;

     s = splhigh();
     n = netisr;
     netisr = 0;
     splx(s);
     if (n & (1 << NETISR_ARP))
	  arpintr();
     if (n & (1 << NETISR_IP))
	  ipintr();
}
