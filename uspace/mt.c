/*
 * mt.c -- supporting routines for binding kernel simulator to threads
 *	   package (pthreads, in this case)
 *
 * Antony Courtney,	4/15/96
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mt.h"

/*
 * kern_lock -- global lock used to ensure that all kernel code runs in
 * mutual exclusion
 */
pthread_mutex_t kern_lock;
pthread_mutex_t *kern_lock_p=&kern_lock;


/* software interrupt thread: waits on condition sw_intr_cond, and then
 * invokes do_sw_intr() to perform software interrupt processing
 */
static pthread_cond_t sw_intr_cond;

void *sw_intr_thread(void *arg)
{
     MU_LOCK(&kern_lock);
     while (1) {
	  COND_WAIT(&sw_intr_cond,&kern_lock);
	  do_sw_intr();
     }
}

void signal_sw_intr(void)
{
     COND_SIGNAL(&sw_intr_cond);
}

/* initialise global MT-state: */
void mt_init(void)
{
     pthread_t tid;

     MT_CALL(pthread_mutex_init, (&kern_lock,NULL));
     (void) pthread_create(&tid,NULL,sw_intr_thread,NULL);
}

/* utility function for reporting multithreading errors: */
void mt_error(int code,char *fmt,...)
{
     va_list ap;

     va_start(ap, fmt);
     (void) vfprintf(stderr, fmt, ap);
     (void) fprintf(stderr,": %s\n", strerror(code));
     
}



