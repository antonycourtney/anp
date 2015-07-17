/*
 * client.c -- a simple client to investigate select() with pthreads
 *
 * Antony Courtney,	14/4/95
 */

#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

#include "anp_wrappers.h"
#include "anp_test.h"

/* just use loopback interface: */

extern unsigned short serv_port;

#define BUFSIZE 256

void dump_bytes(char *buf,int nbytes)
{
     char *s;
     char *nilp;

     /* find next NUL, if any */
     s=buf;
     nilp=buf;
     while (nilp < buf + nbytes) {
	  while ((nilp < buf + nbytes) && (*nilp!='\0'))
	       nilp++;
	  if (nilp < buf + nbytes) {
	       printf("[%s] ",s);
	       s=nilp+1;
	       nilp++;
	  }
     }
     printf("\n");
}

/* client_loop() -- simple loop around a select() checking for data to read
 * on the connection to the server
 */
void client_loop(int fd)
{
     static char buf[BUFSIZE];
     int nbytes;
     int old_flags;
     struct timeval timeout={3600, 0}; 

     while (1) {
	  if ((nbytes=read(fd,buf,BUFSIZE)) < 0) {
	       err_sys("client read");
	  } else {
	       if (nbytes==0) {
		    printf("server closed connection.\n");
		    close(fd);
		    return;
	       }
	       printf("client read %d bytes from server: ",nbytes);
	       dump_bytes(buf,nbytes);
	  }
     }
}

void clitest(char *dst_addr)
{
     int sockfd;
     struct sockaddr_in serv_addr;
     unsigned short portno;

     if (port_num==0) {
	  /* loopback test */
	  portno=(short) serv_port;
     } else {
	  /* take port number from command line */
	  portno=(short) port_num;
     }

     printf("client: attempting to connect to server %s:%d\n", dst_addr,portno);
     memset(&serv_addr,0,sizeof(serv_addr));
     serv_addr.sin_family = AF_INET;
     serv_addr.sin_addr.s_addr=inet_addr(dst_addr);
     serv_addr.sin_port=htons(portno);

     if ((sockfd=socket(AF_INET,SOCK_STREAM,0)) < 0) {
	  err_sys("client socket");
     }
     
     /* connect to server */
     if (connect(sockfd,(struct sockaddr *) &serv_addr,
		 sizeof(serv_addr)) < 0) {
	  err_sys("client connect");
     }
     printf("client: server contacted, entering read loop...\n");

     client_loop(sockfd);
}
     
