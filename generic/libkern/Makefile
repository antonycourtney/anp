
include ../Makedefs.bsd

# AC: removed mcount.o because we don't have defn's for MCOUNT_ENTER
# and MCOUNT_EXIT macros
OBJS = 	adddi3.o anddi3.o ashldi3.o ashrdi3.o bcmp.o cmpdi2.o \
	divdi3.o ffs.o iordi3.o locc.o lshldi3.o lshrdi3.o \
	moddi3.o muldi3.o negdi2.o notdi2.o qdivrem.o \
	random.o rindex.o scanc.o skpc.o strcat.o strcmp.o strcpy.o \
	strlen.o strncpy.o subdi3.o ucmpdi2.o udivdi3.o umoddi3.o xordi3.o

all:	$(OBJS)

clean:	FRC
	rm -f *.o

FRC:
