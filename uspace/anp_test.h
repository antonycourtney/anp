/*
 * anp_test.h -- header file with global definitions for ANP testing
 *
 * Antony Courtney,	4/23/96
 */

/* IP addresses for SLIP link: */

#define CLI_IP_ADDR	"129.135.107.32"
#define SERV_IP_ADDR	"129.135.107.64"
#define LO_IP_ADDR	"127.0.0.1"

/* global variables exported from main module: */
extern int cliflag, servflag, port_num;
