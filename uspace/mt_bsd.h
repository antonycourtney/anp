#ifndef MT_BSD_H
#define MT_BSD_H 1
/*
 * mt_bsd.h -- A version of mt.h which can be used w/ BSD header files
 *
 * Antony Courtney,	4/19/96
 */

extern void *kern_lock_p;

int pthread_mutex_lock(void *);
int pthread_mutex_unlock(void *);

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

#endif
