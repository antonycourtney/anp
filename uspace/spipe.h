#ifndef SPIPE_H
#define SPIPE_H 1
/*
 * spipe.h -- interface to routines for stream-pipes, from Stevens
 *
 * Antony Courtney,	4/22/96
 */

int serv_listen(const char *path);
int serv_accept(int listenfd);

int cli_conn(const char *path);

int spipe_start_input_thread(int pfd,void *tty_ptr);

#endif /* SPIPE_H */
