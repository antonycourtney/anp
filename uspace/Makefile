#
# Makefile for MT-based kernel simulator environment
#

CFLAGS = -g
LIBS = -lanpgen -lnsl -lpthread
CC = gcc
SIMROOT = ../generic
SIMCFLAGS = -g -I$(SIMROOT) -I$(SIMROOT)/include -DKERNEL -DINET
TSTCFLAGS = -g -I$(SIMROOT) -I$(SIMROOT)/include
LDFLAGS = -L$(SIMROOT)


TSTOBJS = nettest1.o config.o errlst.o servtest.o clitest.o
SIMOBJS = kern_globals.o anp_syscall.o kspl.o serline.o
HOSTOBJS = mt.o timers.o util.o ksync.o kmem.o klog.o \
	anp_fdtbl.o anp_ucred.o endian.o init.o spipe.o
LOTSTOBJ = lotest.o
SLTSTOBJ = sltest.o

all:	lotest sltest

lotest:	$(SIMOBJS) $(HOSTOBJS) $(TSTOBJS) $(LOTSTOBJ)
	$(CC) $(LDFLAGS) -o lotest $(SIMOBJS) $(HOSTOBJS) $(TSTOBJS) $(LOTSTOBJ) $(LIBS)

sltest:	$(SIMOBJS) $(HOSTOBJS) $(TSTOBJS) $(SLTSTOBJ)
	$(CC) $(LDFLAGS) -o sltest $(SIMOBJS) $(HOSTOBJS) $(TSTOBJS) $(SLTSTOBJ) $(LIBS)


# N.B:  need a target here for each entry in SIMOBJS:
kern_globals.o:	kern_globals.c
	$(CC) $(SIMCFLAGS) -c kern_globals.c

anp_syscall.o:	anp_syscall.c
	$(CC) $(SIMCFLAGS) -c anp_syscall.c

serline.o:	serline.c
	$(CC) $(SIMCFLAGS) -c serline.c

nettest1.o:	nettest1.c
	$(CC) $(TSTCFLAGS) -c nettest1.c

config.o:	config.c
	$(CC) $(TSTCFLAGS) -c config.c

errlst.o:	errlst.c
	$(CC) $(TSTCFLAGS) -c errlst.c

servtest.o:	servtest.c
	$(CC) $(TSTCFLAGS) -c servtest.c

clitest.o:	clitest.c
	$(CC) $(TSTCFLAGS) -c clitest.c

kspl.o:		kspl.c
	$(CC) $(SIMCFLAGS) -c kspl.c
