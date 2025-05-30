#ifndef DAEMON_RUNNER_H
#define DAEMON_RUNNER_H



/* Forks the calling process (program) to a new session,
 * then forks again so the session becomes unreachable.
 */
void run_this_as_daemon (void);



#endif