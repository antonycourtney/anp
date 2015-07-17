/*
 * aclist.h -- generic, simple drop-in replacement for clists, without weird
 *	       pointer masking trickery
 *
 * Antony Courtney,	4/22/96
 */

#define ACBUFSIZE	56

struct acblock {
     struct acblock *ac_next;	/* pointer to next block */
     unsigned short ac_start;	/* index of first character in block */
     unsigned short ac_len;	/* length of block */
     char ac_data[ACBUFSIZE];	/* data itself */
};
