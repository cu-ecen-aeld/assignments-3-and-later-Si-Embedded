#include "signal_handler.h"
#include <syslog.h>

#include <string.h> /* For NULL pointer. */


volatile sig_atomic_t graceful_exit_is_not_requested = 1;


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

