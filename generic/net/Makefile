
include ../Makedefs.bsd

EXTRA_CFLAGS = -DNSL=1

OBJS = 	bpf.o bpf_filter.o if.o if_ethersubr.o if_loop.o if_sl.o \
	radix.o raw_cb.o raw_usrreq.o route.o rtsock.o slcompress.o

all:	$(OBJS)

clean:	FRC
	rm -f *.o

FRC:
