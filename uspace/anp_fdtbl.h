#ifndef ANP_FDTBL_H
#define ANP_FDTBL_H 1
/*
 * anp_fdtbl.h -- interface to routines for mapping (thread,fd) tuples to
 *		  (struct socket *)s.
 *		(This replaces the more generic per-process file descriptor
 *		 table stored in the proc structure on UN*X systems)
 *
 * Antony Courtney,	4/18/96
 */

/* anp_fdalloc() -- allocate an entry in the current thread's fd table,
 * and return the index into the table
 */
int anp_fdalloc(void *sock);

/* anp_find() -- lookup the socket stored at a given index in the current
 * thread's fd table
 */
void *anp_fdfind(int fd);

/* anp_fdfree() -- release the entry for a particular descriptor in the
 * current thread's descriptor table
 */
void anp_fdfree(int fd);


#endif /* ANP_FDTBL_H */
