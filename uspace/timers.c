/*
 * timers.c -- implementation of high and low priority clock processing
 * in kernel simulator
 *
 * Antony Courtney,	4/10/96
 */

#include <sys/types.h>
#include <sys/time.h>
#include <time.h>
#include <stddef.h>

#include "timers.h"
#include "callout.h"
#include "mt.h"
#include "util.h"

/* clock_cond -- condition variable used by hard-clock to signal the
 * softclock thread that it has work to do
 */
static pthread_cond_t clock_cond;

/* the callout queue itself */
static cqueue_t *cq_head;

/*
 * Bump a timeval by a small number of usec's.
 * (liberated from kern_clock.c in BSD sources)
 */
#define BUMPTIME(t, usec) { \
	register volatile struct timeval *tp = (t); \
	register long us; \
 \
	tp->tv_usec = us = tp->tv_usec + (usec); \
	if (us >= 1000000) { \
		tp->tv_usec = us - 1000000; \
		tp->tv_sec++; \
	} \
}

/* from kern_globals.c: */
extern struct timeval boottime;
extern volatile struct timeval anp_sys_time;
extern const int tick;
extern const int hz;

/* number of ticks since boot time: */
int ticks;

/* tv_diff() -- compute difference between two timeval's */
static struct timeval tv_diff(const volatile struct timeval *tp1,
			      const volatile struct timeval *tp2)
{
     struct timeval td;

     if (tp2->tv_usec > tp1->tv_usec) {
	  td.tv_usec=1000000 + tp1->tv_usec - tp2->tv_usec;
	  td.tv_sec=tp1->tv_sec - tp2->tv_sec - 1;
     } else {
	  td.tv_usec = tp1->tv_usec - tp2->tv_usec;
	  td.tv_sec = tp1->tv_sec - tp2->tv_sec;
     }

     return td;
}

/* tv_ctime_r() -- convert a timeval to a string form */
static char *tv_ctime_r(const volatile struct timeval *tv,char *buf,int buflen)
{
#ifdef OLDEN
     extern char *ctime_r(time_t *,char *,int);
     time_t clock;

     clock=tv->tv_sec;
     clock += (tv->tv_usec / 1000000);
     return ctime_r(&clock,buf,buflen);
#endif
     struct timeval delta;	/* difference between now and boot time */
     int mins, secs, msec;

     delta=tv_diff(tv,&boottime);
     mins=(delta.tv_sec / 60);
     if (mins > 0) {
	  delta.tv_sec -= mins * 60;
     }
     secs=delta.tv_sec;
     msec=delta.tv_usec / 1000;

     sprintf(buf,":%02d:%02d.%03d",mins,secs,msec);

     return buf;
}

/* credit to W. Richard Stevens: */
void sleep_us(unsigned int nusecs)
{
     struct timeval tv;

     tv.tv_sec=nusecs / 1000000;
     tv.tv_usec=nusecs % 1000000;
     (void) select(0, NULL, NULL, NULL, &tv);
}

/* cq_alloc() -- allocate and initialize a callout queue entry */
static cqueue_t *cq_alloc(void (*func)(void *),void *arg,int ticks)
{
     cqueue_t *cq;

     cq=safe_malloc(sizeof(cqueue_t));
     cq->next=NULL;
     cq->func=func;
     cq->arg=arg;
     cq->ticks=ticks;

     return cq;
}

/* timeout() -- implementation of timeout() service provided by a 4.3 kernel
 */
void timeout(void (*func)(void *),void *arg,int ticks)
{
     cqueue_t *cq, *cq_iter, *cq_prev;

     cq=cq_alloc(func,arg,ticks);

     /* just linear search through our callout queue until we find the
      * appropriate place to insert this entry
      */
     if (cq_head==NULL) {
	  cq_head=cq;
	  return;
     }
     if (cq_head->ticks > ticks) {
	  cq->next=cq_head;
	  cq_head->ticks -= ticks;
	  cq_head=cq;
	  return;
     }
     /* we step through list using cq_prev and cq_iter, until we reach the
      * point where cq should be placed between cq_prev and cq_iter
      * (N.B. conditional uses short-circuit evaluation)
      * Note that we decrement our tick count by the number of ticks in
      * the node we have just passed on each iteration, to obtain a relative
      * difference.
      */
     for (cq_prev=cq_head, cq_iter=cq_head->next, ticks -= cq_head->ticks;
	  (cq_iter!=NULL) && (cq_iter->ticks <= ticks);
	  cq_iter=cq_iter->next, cq_prev=cq_prev->next) {

	  ticks -= cq_iter->ticks;

     }

     cq->ticks=ticks;
     cq_prev->next=cq;
     cq->next=cq_iter;
}

/* untimeout() -- cancels a timer by searching for a matching (function,arg)
 * pair in callout queue
 */
void untimeout(void (*func)(void *),void *arg)
{
     cqueue_t *cq, *pcq;

     if (cq_head==NULL)
	  return;
     cq=cq_head->next;
     if ((cq_head->func==func) && (cq_head->arg==arg)) {
	  if (cq!=NULL) {
	       cq->ticks += cq_head->ticks;
	  }
	  free(cq_head);
	  cq_head=cq;
	  return;
     }
     for (pcq=cq_head; cq!=NULL && ((cq->func!=func) || (cq->arg!=arg));
	  cq=cq->next, pcq=pcq->next)
	  ;
     if (cq==NULL) {
	  return;
     }
     if (cq->next!=NULL) {
	  cq->next->ticks += cq->ticks;
     }
     pcq->next=cq->next;
     free(cq);
}     
	  
	  
/* hardclock() -- hard clock processing */
static void hardclock(void)
{
     static char buf[256];
     int needsoft=0;
     cqueue_t *cq;
     
#ifdef HARDCLOCK_DEBUG
     printf("hardclock() invoked at sim. time: %s\n",
	    tv_ctime_r(&anp_sys_time,buf,256));
#endif
     /* decrement delta for items at front of callout queue */
     for (cq=cq_head; cq!=NULL; cq=cq->next) {
	  if (--(cq->ticks) > 0) {
	       break;
	  }
	  needsoft=1;
	  if (cq->ticks==0)
	       break;
     }

     ticks++;
     BUMPTIME(&anp_sys_time,tick);
     if (needsoft) {
	  COND_SIGNAL(&clock_cond);
     }
}

/* implementation of microtime() for kernel routines */
void microtime(struct timeval *tvp)
{
     *tvp=anp_sys_time;
}

/* clock_thread() -- high priority clock processing */
void clock_thread(void *arg)
{
     while (1) {
	  sleep_us(TICK_TIME);

	  MU_LOCK(&kern_lock);
	  hardclock();
	  MU_UNLOCK(&kern_lock);
     }
}


/* callout_thread() -- low priority clock processing */
void callout_thread(void *arg)
{
     cqueue_t *cq;
     
     while (1) {
	  while ((cq_head==NULL) || (cq_head->ticks > 0)) {
	       /* no work to do, so sleep until we are awakened by hardclock */
	       COND_WAIT(&clock_cond,&kern_lock);
	  }
#ifdef CALLOUT_DEBUG
	  printf("callout_thread: softclock activity needed.\n");
#endif
	  while ((cq_head!=NULL) && (cq_head->ticks <= 0)) {
	       /* timer event has expired -- process callout queue entry: */
	       cq=cq_head;
	       cq_head=cq_head->next;
	       (cq->func)(cq->arg);
	       free(cq);
	       /* allow for context switch: */
	       MU_UNLOCK(&kern_lock);
	       MU_LOCK(&kern_lock);
	  }
     }
}


/* start_timers() -- start the timer threads running */
void start_timers(void)
{
     char buf[256];
     /* TODO: really aught to run the clock_thread at high priority */
     pthread_t tid;
     struct sched_param sched_param;

     /* initialize global timers using system call timer: */
     if (gettimeofday(&boottime,NULL) < 0) {
	  perror("gettimeofday");
	  exit(1);
     }
     anp_sys_time=boottime;

     ticks=0;

     printf("timers: kernel timekeeping started: %s\n",
	    tv_ctime_r(&boottime,buf,256));
     printf("simulation: 1 sec. system time elapses every %.2f sec.\n",
	    (double) TICK_TIME/tick);

     if (pthread_cond_init(&clock_cond,NULL) < 0) {
	  perror("pthread_cond_init");
	  exit(1);
     }

     /* start soft-clock thread at default priority */
     pthread_create(&tid, NULL, (void *(*)(void *)) callout_thread, NULL);

     /* start hard-clock thread at high priority */
     pthread_create(&tid, NULL, (void *(*)(void *)) clock_thread, NULL);

}     

/* utility routines taken from elsewhere in Net/3 kernel sources:
 */
/* from kern/kern_clock.c: */
/*
 * Compute number of hz until specified time.  Used to
 * compute third argument to timeout() from an absolute time.
 */
int
hzto(tv)
	struct timeval *tv;
{
	register long ticks, sec;
	int s;

	/*
	 * If number of milliseconds will fit in 32 bit arithmetic,
	 * then compute number of milliseconds to time and scale to
	 * ticks.  Otherwise just compute number of hz in time, rounding
	 * times greater than representible to maximum value.
	 *
	 * Delta times less than 25 days can be computed ``exactly''.
	 * Maximum value for any timeout in 10ms ticks is 250 days.
	 */
	s = splhigh();
	sec = tv->tv_sec - anp_sys_time.tv_sec;
	if (sec <= 0x7fffffff / 1000 - 1000)
		ticks = ((tv->tv_sec - anp_sys_time.tv_sec) * 1000 +
			(tv->tv_usec - anp_sys_time.tv_usec) / 1000) / (tick / 1000);
	else if (sec <= 0x7fffffff / hz)
		ticks = sec * hz;
	else
		ticks = 0x7fffffff;
	splx(s);
	return (ticks);
}

/* from kern/kern_time.c: */
/*
 * Check that a proposed value to load into the .it_value or
 * .it_interval part of an interval timer is acceptable, and
 * fix it to have at least minimal value (i.e. if it is less
 * than the resolution of the clock, round it up.)
 */
itimerfix(tv)
	struct timeval *tv;
{

/* from BSD <sys/errno.h>: */
#define EINVAL	22

	if (tv->tv_sec < 0 || tv->tv_sec > 100000000 ||
	    tv->tv_usec < 0 || tv->tv_usec >= 1000000)
		return (EINVAL);
	if (tv->tv_sec == 0 && tv->tv_usec != 0 && tv->tv_usec < tick)
		tv->tv_usec = tick;
	return (0);
}

/*
 * Add and subtract routines for timevals.
 * N.B.: subtract routine doesn't deal with
 * results which are before the beginning,
 * it just gets very confused in this case.
 * Caveat emptor.
 */
timevaladd(t1, t2)
	struct timeval *t1, *t2;
{

	t1->tv_sec += t2->tv_sec;
	t1->tv_usec += t2->tv_usec;
	timevalfix(t1);
}

timevalfix(t1)
	struct timeval *t1;
{

	if (t1->tv_usec < 0) {
		t1->tv_sec--;
		t1->tv_usec += 1000000;
	}
	if (t1->tv_usec >= 1000000) {
		t1->tv_sec++;
		t1->tv_usec -= 1000000;
	}
}
