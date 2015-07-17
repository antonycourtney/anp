#ifndef MT_H
#define MT_H 1
/*
 * mt.h -- macros and definitions for interface to underlying threads system
 * N.B. This file can only be included by 
 *
 * Antony Courtney,	4/15/96
 */

#include <stdarg.h>
#include <pthread.h>

/*
 * kern_lock -- global lock used to ensure that all kernel code runs in
 * mutual exclusion
 */
extern pthread_mutex_t kern_lock;


/* initialise global MT-state: */
extern void mt_init(void);

/* utility function for reporting multithreading errors: */
void mt_error(int code,char *fmt,...);

#define MT_CALL(func,args) \
{ int mt_code; \
  if ((mt_code = func args) < 0) { \
     mt_error(mt_code,"\"%s\", line %d: " #func, __FILE__, __LINE__); \
     exit(1); \
  }		   \
}

/* macros for common MT operations: */
#define MU_LOCK(mu)	MT_CALL(pthread_mutex_lock, (mu))

#define MU_UNLOCK(mu)	MT_CALL(pthread_mutex_unlock, (mu))

#define COND_WAIT(cond,mu)	MT_CALL(pthread_cond_wait, (cond, mu))

#define COND_SIGNAL(cond)	MT_CALL(pthread_cond_signal, (cond))

#define COND_BCAST(cond)	MT_CALL(pthread_cond_broadcast, (cond))

#endif /* MT_H */
