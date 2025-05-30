CC ?= gcc
CFLAGS +=-Wall -Werror -Wextra
LDFLAGS ?= -lpthread -lrt
TARGET ?= aesdsocket


all: $(TARGET)
	
$(TARGET): aesdsocket.c socket_functions.o connection_handler.o file_io.o \
			signal_handler.o threads.o periodic_writer_handler.o \
			daemon-runner.o
	$(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@

%.o: %.c
	$(CC) $(CFLAGS) $(LDFLAGS) -c $^ -o $@

test-client: $(TARGET)
	nc 127.0.0.1 9000

valgrind: $(TARGET)
	valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes \
		--verbose --log-file=/tmp/valgrind-out.txt ./aesdsocket

clean:
	rm -rf $(TARGET) *.o

.PHONY: all clean test-client valgrind