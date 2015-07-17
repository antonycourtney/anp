/*
 * spipe.c -- code to simulate a serial device using a stream pipe
 *
 * Antony Courtney,	4/22/96
 */

/* These routines stolen outright from Stevens' "Advanced Programming in
 * the Unix Environment"
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <stropts.h>
#include <fcntl.h>
#include <stdio.h>
#include <pthread.h>

#include "spipe.h"
#include "mt.h"

#define FIFO_MODE	S_IRUSR|S_IWUSR

int serv_listen(const char *path)
{
     int tempfd, fd[2], len;

     unlink(path);
     if ((tempfd=creat(path,FIFO_MODE)) < 0) {
	  perror("creat");
	  exit(1);
     }
     if (close(tempfd) < 0) {
	  perror("close");
	  exit(1);
     }

     if (pipe(fd) < 0) {
	  perror("pipe");
	  exit(1);
     }

     if (ioctl(fd[1],I_PUSH,"connld") < 0) {
	  perror("I_PUSH ioctl");
	  exit(1);
     }
     if (fattach(fd[1],path) < 0) {
	  perror("fattach");
	  exit(1);
     }

     return fd[0];
}

/*
 * wait for a client connection to arrive, and accept it...:
 *
 */
int serv_accept(int listenfd)
{
     struct strrecvfd recvfd;

     if (ioctl(listenfd, I_RECVFD, &recvfd) < 0) {
	  perror("I_RECVFD ioctl");
	  exit(1);
     }

     return recvfd.fd;
}

/* connect to server on the given path: */
int cli_conn(const char *path)
{
     int fd;

     if ((fd=open(path,O_RDWR)) < 0) {
	  perror("open");
	  exit(1);
     }

     if (isastream(fd)==0) {
	  fprintf(stderr,"cli_conn: path \"%s\" is not a stream\n",path);
	  exit(1);
     }
     return fd;
}

static int in_fd;

void *spipe_input_thread(void *arg)
{
#define BUFSIZE 256
     
     unsigned char buf[BUFSIZE];
     int i,nbytes;

     while (1) {
	  if ((nbytes=read(in_fd,buf,BUFSIZE)) < 0) {
	       perror("read");
	       exit(1);
	  }
	  MU_LOCK(&kern_lock);
	  for (i=0; i < nbytes; i++) {
	       slinput((int) buf[i], arg);
	  }
	  MU_UNLOCK(&kern_lock);
     }
}

int spipe_start_input_thread(int pfd,void *tty_ptr)
{
     pthread_t tid;

     in_fd=pfd;
     (void) pthread_create(&tid,NULL,spipe_input_thread,tty_ptr);
     sleep(1);
     printf("spipe_start_input_thread: Input thread started...\n");
}

