#include <syslog.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>

#include <arpa/inet.h>

#include <signal.h>

#include "socket_functions.h"


static uint8_t graceful_exit_is_not_requested = 1;


static int append_to_file (FILE *file, const char *buffer, int size);


static void signal_handler (int signal_number);


static int setup_signal_handler ();


static int delete_file (const char *filepath);


static void reset (ReceiveBuffer *buffer);


int main (int argc, char **argv) {
    openlog (NULL, LOG_PERROR| LOG_PID, LOG_USER);
    
    int status                   = 0;
    int server_socket            = 0;
    int client_socket            = 0;
    struct sockaddr_in client_details;

    FILE *file            = NULL;
    const char *file_path = "/var/tmp/aesdsocketdata";
    int bytes_received    = 0;
    ReceiveBuffer buffer  = {.data = NULL, .size = 0};

    if (setup_signal_handler ()) {return -1;}
    
    /* Set up server socket to allow users to connect. */
    if (argc == 2 && strncmp (argv [1], "-d", 2) == 0) {
        status = open_socket (&server_socket, true);
    } else {
        status = open_socket (&server_socket, false);
    }
    if (status) {return 1;}
    syslog (LOG_INFO, "Successfully opened the server socket.");

    while (graceful_exit_is_not_requested) {
        /* Wait for a new connection by a client. */
        syslog (LOG_DEBUG, "Listening for a new connection.");
        status = listen_and_accept (server_socket, &client_socket, 
            &client_details);
        if (status) {break;}

        syslog (LOG_INFO, "Accepted connection from %s:%d", 
            inet_ntoa (client_details.sin_addr), client_details.sin_port);

        while (graceful_exit_is_not_requested) {
            /* Receive data. */
            bytes_received = receive_data (client_socket, &buffer);
            if (bytes_received <= 0) {
                syslog (LOG_INFO, "Closed connection with %s:%d", 
                    inet_ntoa (client_details.sin_addr), 
                        client_details.sin_port);
                close (client_socket);
                status = 0; 
                break;
            }
    
            syslog (LOG_DEBUG, "Received %d bytes.", bytes_received);
    
            /* Echo and save data. */
            if (newline_is_detected (buffer)) {
                syslog (LOG_DEBUG, "Newline detected.");
                
                file = fopen (file_path, "w");
                if (file == NULL) {
                    syslog (LOG_ERR, "Could not open file at %s.", file_path);
                    break;
                }

                append_to_file (file, buffer.data, buffer.size);
                fclose (file);
                
                echo (client_socket, buffer);
                reset (NULL);
            }
        }
    }

    /* Cleanup file descriptors and free memory. */
    syslog (LOG_INFO, "Running clean up.");
    delete_file (file_path);
    free (buffer.data);
    close (client_socket);
    close (server_socket);
    closelog ();

    return status;
}


int append_to_file (FILE *file, const char *buffer, int size) {
    int bytes_written = 0;

    syslog (LOG_DEBUG, "Attempting to save %d bytes to /var/tmp.",
        size);

    bytes_written = fwrite (buffer, 1, size, file);
    if (bytes_written != size) {
        syslog (LOG_ERR, "An error occured while writing the received data"
            " to the file.");
        return -1;
    }

    syslog (LOG_DEBUG, "Succesfully written %d bytes of %d bytes to "
        "/var/tmp.", bytes_written, size);
    return bytes_written;
}


void signal_handler (int signal_number) {
    switch (signal_number) {
    case SIGINT:
    case SIGTERM:
        graceful_exit_is_not_requested = 0;
        syslog (LOG_INFO, "Caught signal, exiting.");
    break;

    default:
        syslog (LOG_INFO, "Signal number not recognised.");
    }
}


int setup_signal_handler () {
    struct sigaction signal_action = {0};
    signal_action.sa_handler = signal_handler;

    if (sigaction (SIGINT, &signal_action, NULL)) {
        syslog (LOG_ERR, "Could not register new signal hangler for SIGINT.");
        return -1;
    }
    if (sigaction (SIGTERM, &signal_action, NULL)) {
        syslog (LOG_ERR, "Could not register new signal hangler for SIGTERM.");
        return -1;
    }

    return 0;
}


static int delete_file (const char *filepath) {
    int status = 0;

    syslog (LOG_DEBUG, "Deleting %s.", filepath);
    status = unlink (filepath);
    if (status != 0) {
        syslog (LOG_WARNING, "Error deleting file. %s.", strerror (errno));
    }
    else {
        syslog (LOG_DEBUG, "Successfully deleted file %s.", filepath);
    }

    return status;
}


void reset (ReceiveBuffer *buffer) {
    if (buffer == NULL) {return;}

    free (buffer->data);
    buffer->data = NULL;
    buffer->size = 0;
}
