#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <signal.h>


/* Defined by signal_handler.c. Only ever changed by signal_handler. */
extern volatile sig_atomic_t graceful_exit_is_not_requested;


void signal_handler (int signal_number);
int setup_signal_handler ();



#endif