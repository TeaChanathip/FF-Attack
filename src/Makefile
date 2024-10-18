FILES = \
	test-clflush.c \
	ff-simulate.c

prefix=/usr/local
exec_prefix=${prefix}

FTARGETS=$(FILES:.c=)
FOBJS=${FILES:.c=.o}
CFLAGS=-g -O0   -I..
LDFLAGS=-L../src/ -g
LDLIBS=-lmastik

TARGETS=${FTARGETS}
OBJS=${FOBJS}

all: ${TARGETS}

install:

${FTARGETS}: %: %.o 
	${CC} ${LDFLAGS} -o $@ $@.o ${LDLIBS} ${CFLAGS}

clean:
	rm -f ${TARGETS} ${OBJS}
