/*
 * server.c -- simple TCP socket server, which just binds to a socket on a
 * random port, accepts a connection and writes some data to that connection
 * every second...
 *
 * Antony Courtney,	14/4/95
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <sys/uio.h>

#include "anp_wrappers.h"

#define BUFSIZE 256

short serv_port=0;


void servtest(void)
{
     int servfd, clifd;	/* server and client fd's */
     struct sockaddr_in serv_addr, cli_addr;
     int clilen, servlen;
     char buf[BUFSIZE];
     int msgcount=0;
     struct iovec iov[2];

     printf("server started...\n");
     /* create a server socket */
     if ((servfd=socket(AF_INET,SOCK_STREAM,0)) < 0) {
	  err_sys("server socket");
     }

     /* bind our local address so */
     memset(&serv_addr,0,sizeof(serv_addr));
     serv_addr.sin_family=AF_INET;
     serv_addr.sin_addr.s_addr=htonl(INADDR_ANY);
     serv_addr.sin_port=0;

     if (bind(servfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0) {
	  err_sys("server bind");
     }
     
     /* find out which port we actually bound to */
     servlen=sizeof(serv_addr);
     if (getsockname(servfd,(struct sockaddr *) &serv_addr,
		     &servlen) < 0) {
	  err_sys("getsockname");
     }
     serv_port=ntohs(serv_addr.sin_port);
     printf("server started: bound to local address: %s\n", str_addr(&serv_addr));
     
     listen(servfd,5);
     
     while (1) {
	  clilen=sizeof(cli_addr);

	  if ((clifd=accept(servfd,(struct sockaddr *) &cli_addr,
			    &clilen)) < 0) {
	       err_sys("accept");
	  }
	  printf("server accepted connection from: %s\n", str_addr(&cli_addr));
	  while (1) {
	       sprintf(buf,"this is message %d",msgcount++);

	       printf("server sending message: [%s]\n",buf);
	       if (write(clifd,buf,strlen(buf)+1) < 0) {
		    printf("server: write() failed, closing connection...\n");
		    close(clifd);
		    break;
	       }
	       sleep(1);
	  }
     }
}
     



