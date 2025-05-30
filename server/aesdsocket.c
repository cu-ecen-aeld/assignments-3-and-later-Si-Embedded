#include <syslog.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#include "socket_functions.h"
#include "signal_handler.h"     /* Holds graceful_exit flag. */
#include "file_io.h"
#include "connection_handler.h"
#include "threads.h"
#include "periodic_writer_handler.h"
#include "daemon-runner.h"



int main (int argc, char **argv) {
    openlog (NULL, LOG_PID, LOG_USER);
    
    int status                   = 0;
    int server_socket            = 0; 

    syslog (LOG_INFO, "======== Starting socket server ========");

    /* Run as daemon if requested with the -d parameter. */
    if ((argc == 2 && strncmp (argv [1], "-d", 2) == 0)) {
        syslog (LOG_INFO, "Requested to run as daemon.");
        run_this_as_daemon ();
    }

    if (setup_signal_handler () || wr_init ("/var/tmp/aesdsocketdata")) {
        return -1;
    }
    th_init ();

    pw_spawn_and_store_periodic_file_writer_thread ();    
    
    status = open_socket (&server_socket);
    if (status) {return 1;}

    while (graceful_exit_is_not_requested) {
        ThreadData thread_data = {0};
        th_join_completed_threads_in_storage ();

        status = wait_for_new_connection (server_socket, &thread_data);
        if (status) {continue;}

        th_start_and_store_thread (thread_data);
    }

    /* Clean up file descriptors and free memory. */
    syslog (LOG_INFO, "Running clean up.");
    th_request_all_threads_to_exit_gracefully ();
    th_clean_up_stored_thread_data ();
    wr_cleanup ();
    wr_delete_file ();
    close (server_socket);
    closelog ();

    return 0;
}

