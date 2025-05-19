#include "connection_handler.h"
#include "receive_buffer.h"
#include "threads.h"
#include "signal_handler.h"     /* For graceful_exit_is_not_requested flag. */
#include "socket_functions.h"
#include "file_io.h"

#include <stdio.h>
#include <syslog.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <arpa/inet.h>



void *ch_handle_new_connection (void *thread_data) {
    ReceiveBuffer buffer  = {.data = NULL, .size = 0};
    int bytes_received    = 0;
    int status            = 0;

    if (thread_data == NULL) {return NULL;}
    ThreadData *data = thread_data;
    ConnectionHandlerArguments *connection = data->thread_arguments;

    syslog (LOG_INFO, "Handling connection %s:%d in a thread.", 
        inet_ntoa (connection->details.sin_addr), 
        connection->details.sin_port);

    while (graceful_exit_is_not_requested) {
        /* Receive data. */
        bytes_received = receive_data (connection->socket, &buffer);
        if (bytes_received <= 0) {
            syslog (LOG_INFO, "Closed connection with %s:%d", 
                inet_ntoa (connection->details.sin_addr), 
                connection->details.sin_port);
            close (connection->socket);
            status = 0; 
            break;
        }

        syslog (LOG_DEBUG, "Received %d bytes.", bytes_received);

        /* Echo and save data. */
        if (newline_is_detected (buffer)) {
            syslog (LOG_DEBUG, "Newline detected.");
            
            wr_write (buffer.data, buffer.size);
            
            echo_entire_file (connection->socket);
            free (buffer.data);
            buffer.data = NULL; 
        }
    }

    syslog (LOG_DEBUG, "Cleaning up data inside of thread %lu handler "
        "and marking it completed.", data->thread);

    free (buffer.data);
    if (status != 0) {
        close (connection->socket);
    }

    data->has_completed = true;

    return NULL;
}


void ch_shutdown_connection (void *thread_data) {
    ThreadData *data = thread_data;
    ConnectionHandlerArguments *arguments = data->thread_arguments;

    int status = shutdown (arguments->socket, SHUT_RD);
    if (status) {
        syslog (LOG_ERR, "Error while shutting down socket %d in "
            "thread %lu. %s.",
            arguments->socket, 
            data->thread,
            strerror (errno));
    }
}


void ch_free_arguments (void *thread_data) {
    ThreadData *data = thread_data;
    free (data->thread_arguments);
    data->thread_arguments = NULL;
}