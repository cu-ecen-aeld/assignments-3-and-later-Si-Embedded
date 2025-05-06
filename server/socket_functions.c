#include "socket_functions.h"

#include <sys/types.h>
#include <sys/socket.h>
#include <syslog.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>



static void run_as_daemon ();


int open_socket (FileDescriptor *socket_fd, bool intended_as_daemon) {
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

    if (intended_as_daemon) {
        run_as_daemon ();
    }

    *socket_fd = server_socket;
    return 0;
}


Status listen_and_accept (FileDescriptor server_socket, 
    FileDescriptor *new_connection, struct sockaddr_in *new_connection_details)
{
    int status = 0;
    socklen_t length = sizeof (new_connection_details);

    status = listen (server_socket, 1);
    if (status) {
        syslog (LOG_ERR, "Failed to listen on given socket. \
            Exit code: %d. %s.",status, gai_strerror (status));
        return -1;
    }

    *new_connection = accept (server_socket, 
        (struct sockaddr*) new_connection_details, &length);
    if (*new_connection == -1) {
        if (errno == EINTR) {
            syslog (LOG_INFO, "Accept call interrupted by signal. Exiting.");
        }
        else {
            syslog (LOG_ERR, "Failed to accept the new socket. Exit code: %d. "
                "%s.", errno, gai_strerror (errno));
        }
        return -1;
    }

    return 0;
}


int receive_data (FileDescriptor client, ReceiveBuffer *buffer) {
    int bytes_received = 0;
    const int internal_buffer_size = 1024;
    char internal_buffer [internal_buffer_size];
    char *reallocated_buffer = NULL;

    bytes_received = recv (client, internal_buffer, internal_buffer_size, 0);
    if (bytes_received == 0) {
        syslog (LOG_INFO, "No bytes received. The connection to the client "
            "is assumed terminated.");
        return 0;
    }
    else if (bytes_received == -1) {
        syslog (LOG_ERR, "Unable to receive bytes from the given socket.");
        return -2;
    }

    reallocated_buffer = realloc (buffer->data, buffer->size + bytes_received);
    if (reallocated_buffer == NULL) {
        syslog (LOG_WARNING, "Could not reallocate memory for storing newly \
            received data.");
        return -4;
    }

    buffer->data = reallocated_buffer;
    memcpy (buffer->data + buffer->size, internal_buffer, bytes_received);
    buffer->size += bytes_received;

    if (bytes_received == internal_buffer_size) {
        return bytes_received + receive_data (client, buffer);
    }

    return bytes_received;
}


int newline_is_detected (ReceiveBuffer buffer) {
    return buffer.data [buffer.size - 1] == '\n';
}


int echo (FileDescriptor socket, ReceiveBuffer buffer) {
    syslog (LOG_DEBUG, "Echoing back to client.");
    int bytes_send = send (socket, buffer.data, buffer.size, 0);
    if (bytes_send == -1) {
        syslog (LOG_ERR, "Error sending bytes to client. No bytes send.");
        return -2;
    }
    else if (bytes_send != buffer.size) {
        syslog (LOG_WARNING, "Only %d out of the intended %d bytes have been "
            "sent.", bytes_send, buffer.size);
        return -1;
    }

    syslog (LOG_DEBUG, "Sent %d bytes back.", bytes_send);
    return 0;
}


static void run_as_daemon () {
    pid_t pid = fork ();

    if (pid == -1) {syslog (LOG_ERR, "%s", strerror (errno));}
    else if (pid) {
        syslog (LOG_INFO, "Child started with PID %d. Parent "
            "exiting.", pid);
        exit (EXIT_SUCCESS);
    }
    else {
        syslog (LOG_INFO, "Child started.");
        if (setsid () == -1) {
            syslog (LOG_ERR, "%s", strerror (errno));
            return;
        }
        syslog (LOG_INFO, "Started a new session.");

        /* Fork twice to avoid reattaching a terminal. */
        pid = fork ();
        if (pid == -1) {syslog (LOG_ERR, "%s", strerror (errno));}
        else if (pid) {
            syslog (LOG_INFO, "Child started with PID %d. Parent "
                "exiting.", pid);
            syslog (LOG_INFO, "Terminating main session process.");
            exit (EXIT_SUCCESS);
        }
        else {
            syslog (LOG_INFO, "Continuing as daemon.");
        }
    }
}