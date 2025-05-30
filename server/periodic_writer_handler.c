#include "periodic_writer_handler.h"
#include "threads.h"
#include "file_io.h"
#include "signal_handler.h"     /* For graceful_exit flag. */

#include <stdio.h>      /* For NULL definition. */
#include <time.h>
#include <syslog.h>
#include <unistd.h>
#include <string.h>


#define BUFFER_CAPACTITY    128

static const char *prefix = "timestamp:";
typedef struct {
    char data [BUFFER_CAPACTITY];
    size_t size;
} Buffer;


/* RFC 2822 includes the shortened day of week, numerical date, three-letter 
 * month abbreviation, year, time, and time zone, displaying as 
 * 01 Jun 2016 14:31:46 -0700.
 */
static int compile_time_string_in_rfc_2822_format (Buffer *buffer);


/* Automatically exits on signal. */
static void wait_for_ten_seconds (void);


static void graceful_exit (void *thread_data);


void pw_spawn_and_store_periodic_file_writer_thread () {
    ThreadData thread_data           = {0};
    thread_data.thread_target        = &pw_periodically_write_to_file;
    thread_data.thread_graceful_exit = graceful_exit;

    th_start_and_store_thread (thread_data);
}



void* pw_periodically_write_to_file (void *args) {
    ThreadData *thread_data = args;
    
    while (graceful_exit_is_not_requested) {
        wait_for_ten_seconds ();
        Buffer buffer = {0};
        compile_time_string_in_rfc_2822_format (&buffer);
        wr_write (buffer.data, buffer.size);
    }

    thread_data->has_completed = true;

    return NULL;
}


int compile_time_string_in_rfc_2822_format (Buffer *buffer) {
    /* Prepare buffer with prefix. */
    memcpy (buffer->data, prefix, strlen (prefix));
    buffer->size += strlen (prefix);

    /* Get time. */
    time_t time_in_seconds = time (NULL);
    struct tm *temporary   = localtime (&time_in_seconds);
    if (temporary == NULL) {
        syslog (LOG_WARNING, "Unable to retrieve time.");
        return -1;
    }

    /* Convert time to RFC 2822 format  */
    char characters_written = strftime (
        &buffer->data [buffer->size], 
        BUFFER_CAPACTITY - buffer->size - 1,  /* -1 to leave room for \n. */
        "%d %b %Y %X",
        temporary);

    buffer->size += characters_written;
    buffer->data [buffer->size++] = '\n';

    syslog (LOG_DEBUG, "Compiled rfc2822 timestring: %s", buffer->data);

    return 0;
}


void wait_for_ten_seconds () {
    struct timespec current_time;
    if (clock_gettime (CLOCK_MONOTONIC, &current_time)) {
        syslog (LOG_ERR, "Unable to retrieve monotonic time.");
        return;
    }

    const struct timespec delay = {
        .tv_sec  = 10 + current_time.tv_sec,
        .tv_nsec = current_time.tv_nsec
    };
    clock_nanosleep (CLOCK_MONOTONIC, TIMER_ABSTIME, &delay, NULL);
}


static void graceful_exit (void *thread_data) {
    ThreadData *data = thread_data;
    data->has_completed = true;
    pthread_cancel (data->thread);
}
