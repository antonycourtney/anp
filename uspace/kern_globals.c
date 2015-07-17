/*
 * kern_globals.c -- global variables exported by the kernel;  This file
 *		     should be compiled with BSD headers
 *
 * Antony Courtney,	4/10/96
 */

#include <sys/param.h>
#include <sys/time.h>
#include <sys/kernel.h>

/* Global variables exported by the kernel */

/*
 * TODO: provide an initialization routine which sets these appropriately
 */
 
#define HOSTID 0xDEADBEEF
#define HOSTNAME "bobo.frob.org"

/* 1.1 */
long hostid=HOSTID;
char hostname[MAXHOSTNAMELEN]=HOSTNAME;
int hostnamelen=sizeof(HOSTNAME);
 
/* 1.2 */
volatile struct timeval mono_time;
struct timeval boottime;
struct timeval runtime;
volatile struct timeval anp_sys_time;
struct timezone tz;                      /* XXX */
 
int tick=10000;                        /* usec per tick (1000000 / hz) */
int hz=100;                          /* system clock's frequency */
int stathz=100;                      /* statistics clock's frequency */
int profhz=100;                      /* profiling clock's frequency */
int lbolt;                       /* once a second sleep address */

