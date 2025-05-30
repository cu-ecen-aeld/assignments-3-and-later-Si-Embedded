#ifndef SOCKET_FUNCTIONS_H
#define SOCKET_FUNCTIONS_H

#include <netdb.h>

#include "receive_buffer.h"
#include "threads.h"


typedef int FileDescriptor;
typedef int Status;


/* Important: the returned struct must be freed by freeaddrinfo. */
int open_socket (FileDescriptor *socket_fd);


/* Waits for a new connection by calling listen () and accept (). */
Status wait_for_new_connection (FileDescriptor server_socket, 
    ThreadData *thread_data);


int receive_data (FileDescriptor client, ReceiveBuffer *buffer);


int newline_is_detected (ReceiveBuffer buffer);


int echo_entire_file (FileDescriptor socket);



#endif