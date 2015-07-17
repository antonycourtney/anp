/*
 * ksync.c -- kernel synchronization primitives (sleep and wakeup) implemented
 *	      using mutexes and condition variables
 *
 * Antony Courtney,	4/15/96
 */

#include <assert.h>
#include "mt.h"

/* taken from the Net/3 kernel errno.h: */
#undef EWOULDBLOCK
#define EWOULDBLOCK	35

#define HTSIZE 8
#define CHASH(wchan)	((unsigned long) (wchan) % HTSIZE)

/* we implement tsleep() and wakeup() through an array of condition
 * variables; we choose which condition variable to use by hashing the
 * sleep address
 */

static pthread_cond_t slpqueue[HTSIZE];

static void sleep_timefunc(void *arg)
{
     pthread_cond_t **timoarg_p=(pthread_cond_t **) arg;
     pthread_cond_t *cond;

     cond=*timoarg_p;
     *timoarg_p=NULL;
     COND_BCAST(cond);
}

int tsleep(void *chan,int pri,char *wmesg,int timo)
{
     pthread_cond_t *timoarg=&slpqueue[CHASH(chan)];

     if (timo > 0) {
	  /* start a timer running */
	  timeout(sleep_timefunc,&timoarg,timo);
     }
     COND_WAIT(&slpqueue[CHASH(chan)],&kern_lock);
     if (timoarg==NULL) {
	  /* must have been set to NULL by timeout routine */
	  return(EWOULDBLOCK);
     } else {
	  /* really awakened, so cancel timeout */
          untimeout(sleep_timefunc,&timoarg);
     }
     return 0;
}

void wakeup(void *chan)
{
     COND_BCAST(&slpqueue[CHASH(chan)]);
}

/* unsleep() -- should never be called, because we don't provide any
 * mechanism in ANP for a process to end up in the stopped state;
 * provided as a placeholder until we rewrite select() in a non-UNIX
 * way
 */
void unsleep(void *p)
{
     assert(0);
}
