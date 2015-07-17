/*
 * anp_ucred.c -- faked interface to credentials functions
 *
 * Antony Courtney,	4/18/96
 */

/* anp_sys_suser() -- returns true if calling thread has super-user
 * priveleges, or EPERM if not
 */
int anp_sys_suser(void)
{
     return 0;
}
