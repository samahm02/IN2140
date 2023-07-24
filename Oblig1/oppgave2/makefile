CFLAGS = -g -Wall -Wextra -std=gnu11
WFLAGS = --leak-check=full --show-leak-kinds=all --track-origins=yes --malloc-fill=0x40 --free-fill=0x23

compile: tests.c stringoperations.c
	gcc ${CFLAGS} tests.c stringoperations.c -o stringoperations

run: stringoperations
	./stringoperations

valgrind: 
	valgrind ${WFLAGS} ./stringoperations