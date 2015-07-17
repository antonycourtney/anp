
include ../Makedefs.bsd

OBJS =	if_ether.o igmp.o in.o in_cksum.o in_pcb.o in_proto.o \
	ip_icmp.o ip_input.o ip_mroute.o ip_output.o raw_ip.o \
	tcp_debug.o tcp_input.o tcp_output.o tcp_subr.o tcp_timer.o \
	tcp_usrreq.o udp_usrreq.o

all:	$(OBJS)

clean:	FRC
	rm -f *.o

FRC:
