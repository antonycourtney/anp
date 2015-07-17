#ifndef ANP_WRAPPERS_H
#define ANP_WRAPPERS_H 1
/*
 * anp_wrappers.h -- redefinitions of all socket-related system calls which
 *		     are provided by ANP
 *
 * (This include file only to be included by test-case modules, as it will
 *  prevent the true system call on the host system from being called)
 *
 * Antony Courtney,	4/18/96
 */

#define socket			anp_socket
#define bind			anp_bind
#define listen			anp_listen
#define accept			anp_accept
#define connect			anp_connect
#define socketpair		anp_socketpair
#define sendto			anp_sendto
#define sendmsg			anp_sendmsg
#define recvfrom		anp_recvfrom
#define recvmsg			anp_recvmsg
#define shutdown		anp_shutdown
#define setsockopt		anp_setsockopt
#define getsockopt		anp_getsockopt
#define pipe			anp_pipe
#define getsockname		anp_getsockname
#define getpeername		anp_getpeername

#define read			anp_read
#define write			anp_write
#define ioctl			anp_ioctl
#define close			anp_close

#define htonl			anp_sys_htonl
#define htons			anp_sys_htons
#define ntohl			anp_sys_ntohl
#define ntohs			anp_sys_ntohs

extern int anp_errno;

#endif /* ANP_WRAPPERS_H */
