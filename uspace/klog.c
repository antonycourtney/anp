/*
 * klog.c -- kernel logging facilities
 *
 * Antony Courtney,	4/17/96
 */

#include <stdio.h>
#include <stdarg.h>
#include <assert.h>

void anp_sys_log(int level,char *fmt,...)
{
     va_list ap;
     
     va_start(ap,fmt);
     vprintf(fmt,ap);
     putchar('\n');
     va_end(ap);
}

void panic(const char *fmt, ...)
{
     va_list ap;

     va_start(ap,fmt);
     printf("panic: ");
     vprintf(fmt,ap);
     putchar('\n');
     assert(0);
}
      
