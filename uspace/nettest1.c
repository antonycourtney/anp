/*
 * nettest1.c -- A simple test of ANP networking
 *
 * Attempts to send a UDP datagram to our own daytime server via the
 * loopback interface
 *
 */

/* N.B. This file should be compiled w/ BSD headers */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#define size_t __frob
#include "host/stdio.h"
#include "anp_wrappers.h"

#define BUFFSIZE 256

void nettest1(void)
{
     struct sockaddr_in serv;
     char buff[BUFFSIZE]="(initial test data...)";
     int sockfd, n;

     if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
	  err_sys("socket");
     }
     printf("socket() returned: %d\n", sockfd);
     memset(&serv,0,sizeof(serv));
     serv.sin_family=AF_INET;
     serv.sin_addr.s_addr=inet_addr("127.0.0.1");
     serv.sin_port=htons(13);

     if (sendto(sockfd,buff,BUFFSIZE,0,
		(struct sockaddr *) &serv, sizeof(serv)) != BUFFSIZE) {
	  err_sys("sendto");
     }

     /* should finish this test someday... */
     printf("nettest1: completed succesfully...\n");
}

