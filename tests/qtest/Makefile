CC=gcc
LINK=gcc

TEST=build/test
LIBRARY=build/memory.so

TESTOBJS=qtest.o headers.o queue.o memory.o stack.o

LIBS=-lm
CFLAGS=-fPIC -D WINDOWS=0 -D LOGGING=1 -Wall #-Werror -Wextra -pedantic
BFLAGS=-O0 -ggdb3
LFLAGS=-ggdb3 -O0
OFLAGS=-fPIC -O0 -rdynamic

all:${TEST}

all : CFLAGS += ${BFLAGS}

${TEST} : ${TESTOBJS}
	mkdir -p build
	${CC} ${OFLAGS} -o $@ $^

qtest.o: qtest.c
memory.o: memory.c memory.h
headers.o: headers.c headers.h
queue.o: queue.c queue.h
stack.o: stack.c stack.h

.PHONY : clean

clean:
	rm -rf ${TEST} core*
	rm -rf *.o core*
	rm -rf build
