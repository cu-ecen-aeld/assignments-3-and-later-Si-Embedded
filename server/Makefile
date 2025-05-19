CC:=gcc
CFLAGS:=-Wall -Werror -Wextra


all: aesdsocket
	
aesdsocket: aesdsocket.c socket_functions.o connection_handler.o file_io.o \
			signal_handler.o threads.o periodic_writer_handler.o
	$(CC) $(CFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) -c $^ -o $@

test-client: aesdsocket
	nc 127.0.0.1 9000

valgrind: aesdsocket
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
		--verbose --log-file=/tmp/valgrind-out.txt ./aesdsocket

clean:
	rm -rf aesdsocket *.o

.PHONY: all clean test-client valgrind