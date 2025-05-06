CC:=gcc
CFLAGS:=-Wall -Werror -Wextra


all: aesdsocket
	
aesdsocket: aesdsocket.c socket_functions.o
	$(CC) $(CFLAGS) $^ -o $@

socket_functions.o: socket_functions.c
	$(CC) $(CFLAGS) -c $^ -o $@

test-client: aesdsocket
	nc 127.0.0.1 9000

valgrind: aesdsocket
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
		--verbose --log-file=/tmp/valgrind-out.txt ./aesdsocket

clean:
	rm -rf aesdsocket *.o

.PHONY: all clean test-client valgrind