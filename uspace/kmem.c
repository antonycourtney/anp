/*
 * kmem.c -- kernel memory allocation / deallocation primitives
 *
 * Antony Courtney,	4/17/96
 */

#include <stdlib.h>

/*
 * implementations of renamed kernel malloc/free routines
 */
void *anp_sys_malloc(unsigned long size,int type,int flags)
{
#ifdef NOPE
     return malloc(size);
#endif
     return valloc(size);
}

void anp_sys_free(void *addr,int type)
{
     free(addr);
}

/* copyin() and copyout() are just call-throughs to memcpy(), since we don't
 * actually have any address space boundaries...
 */
int copyin(void *src,void *dst,unsigned long size)
{
     memcpy(dst,src,size);
     return 0;
}

int copyout(void *src,void *dst,unsigned long size)
{
     memcpy(dst,src,size);
     return 0;
}

/* ovbcopy() -- call-through to bcopy(), which we (hopefully) have on the
 * system which handles overlapping strings correctly...
 */
void ovbcopy(void *src,void *dst,unsigned long nbytes)
{
     bcopy(src,dst,nbytes);
}
