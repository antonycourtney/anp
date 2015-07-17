/*
 * config.c -- takes the place of "ifconfig" usually run at boot time
 *
 * Antony Courtney,	4/19/96
 *
 * (must be compiled w/ BSD headers)
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sockio.h>
#include <sys/errno.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <string.h>

#define size_t __frob
#include "host/stdio.h"
#include "anp_wrappers.h"
#include "anp_test.h"

/* sl_ifconfig() -- configure SLIP interface */
void sl_ifconfig(char *local_addr,char *remote_addr)
{
     const char *netmask="255.255.255.0";
     struct ifreq ifr;
     struct sockaddr_in sin;
     int s;

     if ((s=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
	  err_sys("ifconfig socket");
     }

     memset(&ifr,0,sizeof(ifr));
     strncpy(ifr.ifr_name,"sl0",sizeof(ifr.ifr_name));

     /* let's try deleting an interface address and then adding one: */
     if (ioctl(s,SIOCDIFADDR,&ifr) < 0) {
	  if (anp_errno==EADDRNOTAVAIL) {
	       /* means there was no previous address for this interface */
	  } else {
	       err_sys("ioctl (SIOCDIFADDR)");
	  }
     }

     /* set local IP address: */
     memset(&sin,0,sizeof(sin));
     sin.sin_family=AF_INET;
     sin.sin_len=sizeof(sin);
     sin.sin_addr.s_addr=inet_addr(local_addr);
     memcpy(&ifr.ifr_addr,&sin,sizeof(sin));
     if (ioctl(s,SIOCAIFADDR,&ifr) < 0) {
	  err_sys("ioctl (SIOCAIFADDR)");
     }

     /* now let's try and set the destination IP address and netmask */
     memset(&sin,0,sizeof(sin));
     sin.sin_family=AF_INET;
     sin.sin_len=sizeof(sin);
     sin.sin_addr.s_addr=inet_addr(remote_addr);
     memcpy(&ifr.ifr_addr,&sin,sizeof(sin));
     if (ioctl(s,SIOCSIFDSTADDR,&ifr) < 0) {
	  err_sys("ioctl (SIOCSIFDSTADDR");
     }

     memset(&sin,0,sizeof(sin));
     sin.sin_family=AF_INET;
     sin.sin_len=sizeof(sin);
     sin.sin_addr.s_addr=inet_addr(netmask);
     memcpy(&ifr.ifr_addr,&sin,sizeof(sin));
     if (ioctl(s,SIOCSIFNETMASK,&ifr) < 0) {
	  err_sys("ioctl (SIOCSIFNETMASK");
     }

     /* now we do a SIOCSIFADDR ioctl, just to force an update to routing
      * table entry
      */
     memset(&sin,0,sizeof(sin));
     sin.sin_family=AF_INET;
     sin.sin_len=sizeof(sin);
     sin.sin_addr.s_addr=inet_addr(local_addr);
     memcpy(&ifr.ifr_addr,&sin,sizeof(sin));
     if (ioctl(s,SIOCSIFADDR,&ifr) < 0) {
	  err_sys("ioctl (SIOCSIFADDR)");
     }
     close(s);

     printf("ifconfig sl0: local: %s, remote: %s, netmask: %s\n",
	    local_addr,remote_addr,netmask);

}

void do_ifconfig(void)
{
     struct ifreq ifr;
     struct sockaddr_in sin;
     int s;

     if ((s=socket(AF_INET,SOCK_DGRAM,0)) < 0) {
	  err_sys("ifconfig socket");
     }
     memset(&sin,0,sizeof(sin));
     sin.sin_family=AF_INET;
     sin.sin_len=sizeof(sin);
     sin.sin_addr.s_addr=inet_addr(LO_IP_ADDR);

     memset(&ifr,0,sizeof(ifr));
     strncpy(ifr.ifr_name,"lo0",sizeof(ifr.ifr_name));

     /* let's try deleting an interface address and then adding one: */
     if (ioctl(s,SIOCDIFADDR,&ifr) < 0) {
	  if (anp_errno==EADDRNOTAVAIL) {
	       /* means there was no previous address for this interface */
	  } else {
	       err_sys("ioctl (SIOCDIFADDR)");
	  }
     }
     memcpy(&ifr.ifr_addr,&sin,sizeof(sin));

     if (ioctl(s,SIOCAIFADDR,&ifr) < 0) {
	  err_sys("ioctl (SIOCAIFADDR)");
     }

     close(s);

     printf("ifconfig lo0: IP address: %s\n", LO_IP_ADDR);

     /* now configure sl0 interface, if appropriate */
     if (cliflag) {
	  sl_ifconfig(CLI_IP_ADDR,SERV_IP_ADDR);
     }
     if (servflag) {
	  sl_ifconfig(SERV_IP_ADDR,CLI_IP_ADDR);
     }

}
	  
