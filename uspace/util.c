/*
 * util.c -- implementations of utility routines used through the kernel
 *	     simulator
 *
 * Antony Courtney,	4/15/96
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdio.h>
#include "util.h"

void *safe_malloc(unsigned long nbytes)
{
     void *p;

     if ((p=malloc(nbytes))==NULL) {
	  perror("malloc");
	  exit(1);
     }
     return p;
}

extern char *anp_sys_errlist[];

void err_sys(char *msg)
{
     extern int anp_errno;
     
     fprintf(stderr,"%s: %s\n",msg,anp_sys_errlist[anp_errno]);
     exit(1);
}

#define LBUFSIZE 256
#define RINGSIZE 8
 
/* str_addr() -- convert an address to a string in internet dot notation */
char *str_addr(void *sap)
{
     struct sockaddr_in *sa=sap;
     static char ring_buffer[RINGSIZE][LBUFSIZE];
     static int ringpos=0;
     unsigned char *addptr;
     unsigned short portno=ntohs(sa->sin_port);
     char *buf;
 
     buf=ring_buffer[ringpos];
     addptr=(unsigned char *) &(sa->sin_addr);
     sprintf(buf,"%d.%d.%d.%d:%d",addptr[0],addptr[1],addptr[2],addptr[3],
             portno);
     ringpos = (ringpos + 1) % RINGSIZE;
 
     return buf;
}





