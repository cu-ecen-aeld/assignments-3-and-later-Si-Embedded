#include "socket_functions.h"
#include "connection_handler.h" /* For thread target and argument struct. */
#include "file_io.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <stdio.h>



int open_socket (FileDescriptor *socket_fd) {
    struct addrinfo hints = {0};
    struct addrinfo *server_info = NULL;
    int status;

    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags    = AI_PASSIVE;
    hints.ai_protocol = 0;

    int server_socket = socket (PF_INET, SOCK_STREAM, 0);
    if (server_socket < 0) {
        syslog (LOG_ERR, "Failed to create a socket. Exit code: %d. %s.",
            server_socket, gai_strerror (server_socket));
        return -1;
    }

    if (setsockopt (server_socket, 
                    SOL_SOCKET, 
                    SO_REUSEADDR, 
                    &(int){1}, 
                    sizeof (int)))
    {
        syslog (LOG_WARNING, "Unable to set socket with SO_REUSEADDR option.");
    }

    status = getaddrinfo (NULL, "9000", &hints, &server_info);
    if (status != 0) {
        syslog (LOG_ERR, "Failed to get address info. Exit code: %d. %s.", 
            status, gai_strerror (status));
        return -1;
    }

    status = bind  (server_socket, 
        server_info->ai_addr, server_info->ai_addrlen);

    freeaddrinfo (server_info);
    if (status != 0) {
        syslog (LOG_ERR, "Failed to bind the server socket.");
        return -1;
    }

    syslog (LOG_DEBUG, "Successfully opened the server socket.");
    *socket_fd = server_socket;
    return 0;
}


Status wait_for_new_connection (FileDescriptor server_socket, 
    ThreadData *thread_data)
{
    /* These arguments are always freed when the thread cleaned up by 
     * th_clean_up_stored_thread_data (). 
     */
    ConnectionHandlerArguments* connection = malloc 
        (sizeof (ConnectionHandlerArguments));
    if (connection == NULL) {
        syslog (LOG_ERR, "Could not allocate memory for connection "
            "parameters.");
        return -1;
    }
    int status                                 = 0;
    socklen_t length                         = sizeof (&connection->details);

    syslog (LOG_DEBUG, "Listening for a new connection.");
    status = listen (server_socket, 1);
    if (status) {
        syslog (LOG_ERR, "Failed to listen on given socket. \
            Exit code: %d. %s.",status, gai_strerror (status));
        return -1;
    }

    connection->socket = accept (server_socket, 
        (struct sockaddr*) &connection->details, &length);
    if (connection->socket == -1) {
        if (errno == EINTR) {
            syslog (LOG_DEBUG, "Accept call interrupted by signal. Exiting.");
        }
        else {
            syslog (LOG_ERR, "Failed to accept the new socket. Exit code: %d. "
                "%s.", errno, gai_strerror (errno));
        }

        free (connection);
        connection = NULL;
        return -1;
    }

    syslog (LOG_INFO, "Accepted connection from %s:%d", 
        inet_ntoa (connection->details.sin_addr), 
        connection->details.sin_port);

    thread_data->thread_target              = &ch_handle_new_connection;
    thread_data->thread_arguments           = connection;
    thread_data->thread_graceful_exit       = &ch_shutdown_connection;
    thread_data->thread_argument_destructor = &ch_free_arguments;

    return 0;
}


int receive_data (FileDescriptor client, ReceiveBuffer *buffer) {
    int bytes_received = 0;
    const int internal_buffer_size = 1024;
    char internal_buffer [internal_buffer_size];
    char *reallocated_buffer = NULL;

    while (1) {
        bytes_received = recv (client, internal_buffer, 
            internal_buffer_size, 0);

        if (bytes_received == 0) {
            syslog (LOG_INFO, "No bytes received. The connection to the client"
                " is assumed terminated.");
            return 0;
        }
        else if (bytes_received == -1) {
            syslog (LOG_ERR, "Unable to receive bytes from the given socket.");
            return -2;
        }

        reallocated_buffer = realloc (buffer->data, 
            buffer->size + bytes_received);
        if (reallocated_buffer == NULL) {
            syslog (LOG_WARNING, "Could not reallocate memory for storing "
                " newly received data.");
            return -4;
        }

        buffer->data = reallocated_buffer;
        memcpy (buffer->data + buffer->size, internal_buffer, bytes_received);
        buffer->size += bytes_received;

        if (bytes_received < internal_buffer_size) {
            break;
        }
    }

    return bytes_received;
}


int newline_is_detected (ReceiveBuffer buffer) {
    return buffer.data [buffer.size - 1] == '\n';
}


int echo_entire_file (FileDescriptor socket) {
    syslog (LOG_DEBUG, "Echoing back to client.");

    const size_t internal_buffer_size = 4196;
    ssize_t bytes_read    = 0;
    ssize_t bytes_sent    = 0;
    char buffer [internal_buffer_size];
    const char *file_path = wr_get_file_path ();
    
    FILE *file = fopen (file_path, "r");
    if (file == NULL) {
        syslog (LOG_ERR, "Unable to read from file %s.", file_path);
        return -1;
    }

    syslog (LOG_DEBUG, "Attempting to read from %s.", file_path);
    while (true) {
        bytes_read = fread (buffer, 1, internal_buffer_size, file);
        if (!bytes_read) {break;}

        bytes_sent = send (socket, buffer, bytes_read, 0);
        if (bytes_sent == -1) {
            syslog (LOG_ERR, "Error sending bytes to client. No bytes send.");
            return -2;
        }
        else if (bytes_sent != bytes_read) {
            syslog (LOG_WARNING, "Only %ld out of the intended %ld bytes have "
                "been sent.", bytes_sent, bytes_read);
            return -3;
        }
    
        syslog (LOG_DEBUG, "Sent %ld bytes back.", bytes_sent);
    }
    fclose (file);
    
    return 0;
}
