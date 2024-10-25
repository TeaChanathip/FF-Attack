FILES= test-clflush.c

FTARGETS=${FILES:.c=}
FOBJS=${FILES:.c=.o}
CFLAGS=-g -O2
LDLIBS=-lmastik

TARGETS=${FTARGETS} ff-aes-ttable ff-simulate

all: aes_core ${TARGETS}

aes_core:
	cc -O2 -c ../tests/aes_core.c -o ../tests/aes_core.o

${FTARGETS}: %: %.o
	gcc ${CFLAGS} -o $@ $@.o ${LDLIBS}

ff-aes-ttable:
	g++ ${CFLAGS} -c ff-aes-ttable.cpp
	g++ ${CFLAGS} -o ff-aes-ttable ff-aes-ttable.o ../tests/aes_core.o -I../tests/

ff-simulate:
	gcc -O2 -o ../tests/dummy.o -c ../tests/dummy.c
	gcc ${CFLAGS} -o ff-simulate ff-simulate.c ../tests/dummy.o -lmastik -I../tests/

clean:
	rm -f ${FTARGETS} ${FOBJS}
	rm -f ff-aes-ttable ff-aes-ttable.o
	rm -f ../tests/aes_core.o
	rm -f ff-simulate ff-aes-ttable.o
	rm -f ../tests/dummy.o