#include "daemon-runner.h"

#include <syslog.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>



void run_this_as_daemon () {
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
