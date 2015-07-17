/*
 * sltest.c -- test of SLIP interface support in ANP, using stream pipes
 *
 * Antony Courtney,	4/22/96
 */

#include <stdio.h>
#include <pthread.h>

#include "timers.h"
#include "mt.h"
#include "anp_test.h"

int servflag=0;
int cliflag=0;
int port_num=0;

void usage(char *pname)
{
     fprintf(stderr,"usage: %s [-s] [-c] [-p portnum]\n");
     exit(1);
}

void *client_thread(void *arg)
{
     clitest(SERV_IP_ADDR);
}

void *server_thread(void *arg)
{
     servtest();
}

static void start_tests(void)
{
     pthread_t tid;

     if (servflag) {
	  (void) pthread_create(&tid,NULL,server_thread,NULL);
     }
     if (cliflag) {
	  (void) pthread_create(&tid,NULL,client_thread,NULL);
     }
}

int main(int argc,char *argv[])
{
     int c;
     extern int optind;
     extern char *optarg;

     while ((c = getopt(argc,argv,"scp:"))!=EOF) {
	  switch (c) {
	  case 's':
	       if (cliflag) {
		    usage(argv[0]);
	       }
	       servflag++;
	       break;
	  case 'c':
	       if (servflag) {
		    usage(argv[0]);
	       }
	       cliflag++;
	       break;
	  case 'p':
	       port_num=atoi(optarg);
	       break;
	  case '?':
	  default:
	       usage(argv[0]);
	  }
     }

     /* initialize the kernel simulator: */
     anp_init();

     /* initialize the psuedo serial line */
     ser_init();

     /* ifconfig up the SLIP sl0 interface: */
     do_ifconfig();

     /* run some tests */
     start_tests();

     pause();
     exit(0);
}

