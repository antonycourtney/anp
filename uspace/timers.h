#ifndef TIMERS_H
#define TIMERS_H 1
/*
 * timers.h -- interface to routines for managing timers and timer threads
 *	       in user-space kernel simulator
 *
 * Antony Courtney,	4/12/96
 */

/* TICK_TIME is measured in microseconds, and gives the true number of
 * microseconds we sleep for each tick;
 * note that this is completely independent of the simulated tick time
 * (given by hz and tick), and may be adjusted to speed up or slow down
 * the simulation as appropriate
 */
#define TICK_TIME	250000	/* .25 seconds */


/* start_timers() -- start the timer threads running */
void start_timers(void);


#endif	/* TIMERS_H */
