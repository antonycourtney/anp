#ifndef CALLOUT_H
#define CALLOUT_H 1
/*
 * callout.h -- data structure for representing the callout queue used
 *		in timers.c
 *		based on ideas from the 4.3 kernel
 *
 * Antony Courtney,	4/15/96
 */

/* A callout queue entry */
typedef struct cqueue_s {
     struct cqueue_s *next;
     void (*func)(void *arg);	/* function to call, and argument */
     void *arg;
     int ticks;			/* number of ticks until timer expires --
				 * stored as a difference from previous
				 * event on the queue
				 */
} cqueue_t;

#endif /* CALLOUT_H */
