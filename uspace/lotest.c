/*
 * main.c -- initialization of user-space kernel simulator
 *
 * Antony Courtney,	4/12/96
 *
 */

#include <stdio.h>
#include <pthread.h>

#include "timers.h"
#include "mt.h"
#include "anp_test.h"

int servflag=0, cliflag=0, port_num=0;

void timeo(void *arg)
{
     printf("timeout function called w/ arg: [%s]\n",(char *) arg);
}

void *sleeper_thread(void *arg)
{
     int retcode;
     
     MU_LOCK(&kern_lock);
     printf("sleeper_thread started...\n");
     printf("sleeper_thread: about to call tsleep(&sleeper_thread,0,NULL,20); \n");
     retcode=tsleep(&sleeper_thread,0,NULL,20);
     printf("sleeper_thread: tsleep() returned: %d\n", retcode);
     MU_UNLOCK(&kern_lock);
}

/* test of timer and synchronization functionality, now antiquated: */
static void timer_tests(void)
{
     pthread_t tid;

     /* start a couple of timers: */
     MU_LOCK(&kern_lock);
     printf("starting timer A (to wakeup in 20 ticks)\n");
     timeout(timeo,"timer A",20);
     printf("starting timer B (to wakeup in 30 ticks)\n");
     timeout(timeo,"timer B",30);
     printf("starting timer C (to wakeup in 10 ticks)\n");
     timeout(timeo,"timer C",10);
     MU_UNLOCK(&kern_lock);

     /* test sleep and wakeup: */
     /* test 1: a sleep where wakeup really happens */
     (void) pthread_create(&tid,NULL,sleeper_thread,NULL);
     printf("main thread: sleeper_thread forked (#1)\n");
     sleep(1);
     MU_LOCK(&kern_lock);
     printf("main thread: calling wakeup(&sleeper_thread);\n");
     wakeup(&sleeper_thread);
     printf("main thread: sleeper_thread woken...\n");
     MU_UNLOCK(&kern_lock);

     /* test 2: create another sleeper thread, but this time let it time
      * out
      */
     (void) pthread_create(&tid,NULL,sleeper_thread,NULL);
     printf("main thread: sleeper_thread forked (#2)\n");
}
     
void *client_thread(void *arg)
{
     clitest(LO_IP_ADDR);
}

void *server_thread(void *arg)
{
     servtest();
}

static void start_tests(void)
{
     pthread_t tid;

     nettest1();

     (void) pthread_create(&tid,NULL,server_thread,NULL);
     sleep(2);
     (void) pthread_create(&tid,NULL,client_thread,NULL);
}

int main(int argc,char *argv[])
{
     /* initialize the kernel simulator: */
     anp_init();

     /* run some tests */
     start_tests();

     pause();
     exit(0);
}
