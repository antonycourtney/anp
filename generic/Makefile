#
# Makefile for a link test to figure out functions used but not defined
# in the Net/3 TCP/IP code


include Makedefs.bsd

SUBDIRS = libkern kern net netinet

all:	libanpgen.a

# AC: libkern/mcount.o left out because we don't have defn's of MCOUNT_ENTER
# and MCOUNT_EXIT macros

OBJS = 	kern/sys_socket.o kern/uipc_domain.o kern/uipc_mbuf.o \
	kern/uipc_socket.o kern/uipc_socket2.o kern/tty_subr.o \
	kern/uipc_syscalls.o kern/kern_subr.o kern/kern_sysctl.o net/bpf.o \
	net/bpf_filter.o net/if.o net/if_ethersubr.o \
	net/if_loop.o net/if_sl.o net/radix.o net/raw_cb.o \
	net/raw_usrreq.o net/route.o net/rtsock.o net/slcompress.o \
	netinet/if_ether.o netinet/igmp.o netinet/in.o netinet/in_cksum.o \
	netinet/in_pcb.o netinet/in_proto.o netinet/ip_icmp.o \
	netinet/ip_input.o netinet/ip_mroute.o netinet/ip_output.o \
	netinet/raw_ip.o netinet/tcp_debug.o netinet/tcp_input.o \
	netinet/tcp_output.o netinet/tcp_subr.o netinet/tcp_timer.o \
	netinet/tcp_usrreq.o netinet/udp_usrreq.o libkern/adddi3.o \
	libkern/anddi3.o libkern/ashldi3.o libkern/ashrdi3.o \
	libkern/bcmp.o libkern/cmpdi2.o libkern/divdi3.o \
	libkern/ffs.o libkern/iordi3.o libkern/locc.o \
	libkern/lshldi3.o libkern/lshrdi3.o \
	libkern/moddi3.o libkern/muldi3.o libkern/negdi2.o \
	libkern/notdi2.o libkern/qdivrem.o libkern/random.o \
	libkern/rindex.o libkern/scanc.o libkern/skpc.o \
	libkern/strcat.o libkern/strcmp.o libkern/strcpy.o \
	libkern/strlen.o libkern/strncpy.o libkern/subdi3.o \
	libkern/ucmpdi2.o libkern/udivdi3.o libkern/umoddi3.o \
	libkern/xordi3.o

syminfo:	buildsyms
	./showsyms > syms
	@echo "Make of syminfo complete -- symbol analsysis in ./syms"
	
allsyminfo:	buildsyms
	./showallsyms > allsyms
	@echo "Make of syminfo complete -- symbol analsysis in ./allsyms"
	
# buildsyms: target which places list of all undefined symbols in /tmp/syms
buildsyms:	FRC
	-make linktest > make.log 2>&1
	cat make.log |sed -n '/^\ symbol*/,/ld:*/p' |grep -v '^\ symbol' |grep -v '^main' |grep -v '^ld:' |grep -v '^\.' | sort | uniq > /tmp/syms


# linktest: not a REAL target; just used to try and produce link errors
# to figure out what required functions are missing

linktest:	FRC
	for i in $(SUBDIRS) ; do \
		cd $$i; make;  cd ..; \
	done
	$(LD) -o linktest $(OBJS)

libanpgen.a:	FRC
	for i in $(SUBDIRS) ; do \
		cd $$i; make;  cd ..; \
	done
	ar rv libanpgen.a $(OBJS)
	

clean:	FRC
	for i in $(SUBDIRS) ; do \
		cd $$i; make clean; cd ..; \
	done

FRC:
