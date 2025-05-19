/* Intended as thread target for handling new connections. */

#ifndef CONNECTION_HANDLER_H
#define CONNECTION_HANDLER_H

#include "threads.h"

#include <netinet/in.h> /* For sockaddr_in. */


typedef struct {
    struct sockaddr_in details;
    int                socket;
} ConnectionHandlerArguments;


void *ch_handle_new_connection (void *thread_data);


void ch_shutdown_connection (void *thread_data);


void ch_free_arguments (void *thread_data);


#endif