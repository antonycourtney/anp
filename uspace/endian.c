/*
 * endian.c -- call-through routines for byte-order conversions done in
 * the kernel
 *
 * Antony Courtney,	4/18/96
 */

#include <sys/types.h>
#include <netinet/in.h>

unsigned long anp_sys_htonl(unsigned long l)
{
     return htonl(l);
}

unsigned short anp_sys_htons(unsigned short s)
{
     return htons(s);
}

unsigned long anp_sys_ntohl(unsigned long l)
{
     return ntohl(l);
}

unsigned long anp_sys_ntohs(unsigned short s)
{
     return ntohs(s);
}
