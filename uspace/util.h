#ifndef UTIL_H
#define UTIL_H 1
/*
 * util.h -- various utility routines for use throughout the kernel simulator
 *
 * Antony Courtney,	4/15/96
 */

void *safe_malloc(unsigned long nbytes);

/* N.B. sap is really a (struct sockaddr_in *) */
char *str_addr(void *sap);
#endif /* UTIL_H */
