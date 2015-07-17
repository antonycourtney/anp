
include ../Makedefs.bsd

OBJS =  sys_socket.o uipc_mbuf.o uipc_socket.o uipc_syscalls.o uipc_domain.o \
	uipc_socket2.o kern_subr.o kern_sysctl.o tty_subr.o

all:	$(OBJS)

clean:	FRC
	rm -f *.o

FRC:
