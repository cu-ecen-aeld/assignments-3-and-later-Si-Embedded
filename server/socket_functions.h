#ifndef SOCKET_FUNCTIONS_H
#define SOCKET_FUNCTIONS_H

#include <netdb.h>
#include <stdbool.h>



typedef struct {
    char *data;
    int32_t size;
} ReceiveBuffer;

typedef int FileDescriptor;
typedef int Status;


/* Important: the returned struct must be freed by freeaddrinfo. */
int open_socket (FileDescriptor *socket_fd, bool intended_as_daemon);


Status listen_and_accept (FileDescriptor server_socket, 
    FileDescriptor *new_connection, struct sockaddr_in *new_connection_details);


int receive_data (FileDescriptor client, ReceiveBuffer *buffer);


int newline_is_detected (ReceiveBuffer buffer);


int echo (FileDescriptor socket, ReceiveBuffer buffer);



#endif