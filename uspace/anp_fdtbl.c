/*
 * anp_fdtbl.c -- Implementations of routines for managing a table
 *		  which maps (thread,fd) --> (struct socket *)
 *		(This replaces the more generic per-process file descriptor
 *		 table stored in the proc structure on UN*X systems)
 *
 * Antony Courtney,	4/17/96
 */

#include <assert.h>
#include <stdlib.h>
#include "anp_fdtbl.h"

#define MAXFDS	256

/*
 * In a serious implementation, these routines will actually call through
 * to the threads package on the host system to obtain the caller's thread
 * ID;  For now, however, we just assume a single thread, and use a global
 * table
 * TODO: update this!
 */
static int virgin=1;

static void *fdtbl[MAXFDS];

static void fdtbl_init(void)
{
     int i;

     for (i=0; i < MAXFDS; i++) {
	  fdtbl[i]=NULL;
     }
}

/* anp_fdalloc() -- allocate an entry in the current thread's fd table,
 * and return the index into the table
 */
int anp_fdalloc(void *sock)
{
     int i;
     
     if (virgin) {
	  fdtbl_init();
	  virgin=0;
     }
     for (i=0; i < MAXFDS; i++) {
	  if (fdtbl[i]==NULL) {
	       fdtbl[i]=sock;
	       return i;
	  }
     }
     return -1;
}


/* anp_find() -- lookup the socket stored at a given index in the current
 * thread's fd table
 */
void *anp_fdfind(int fd)
{
     assert(fd < MAXFDS);
     assert(fdtbl[fd]!=NULL);
     return fdtbl[fd];
}

/* anp_fdfree() -- release the entry for a particular descriptor in the
 * current thread's descriptor table
 */
void anp_fdfree(int fd)
{
     assert(fdtbl[fd]!=NULL);
     fdtbl[fd]=NULL;
}
		
