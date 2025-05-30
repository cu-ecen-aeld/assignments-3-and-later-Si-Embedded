#include "file_io.h"

#include <pthread.h>
#include <syslog.h>
#include <string.h> /* For strerror (). */
#include <errno.h>
#include <unistd.h> /* For unlink (). */



static pthread_mutex_t lock;
static char *internal_file_path;

static int write_to_file (const char *buffer, int size);


int wr_init (char *file_path) {
    internal_file_path = file_path;
    return pthread_mutex_init (&lock, NULL);
}


int wr_destroy () {
    return pthread_mutex_destroy (&lock);
}


const char *wr_get_file_path () {
    if (internal_file_path == NULL) {
        syslog (LOG_WARNING, "Internal file path requested but it has not "
            "been set. Returning null.");
    }
    return internal_file_path;
}


int wr_write (const char *data, int size) {
    int status = 0;

    status = pthread_mutex_lock (&lock);
    if (status) {
        syslog (LOG_ERR, "Failed to lock mutex. "
            "Error code %d. %s.", status, strerror (status));
        return -1;
    }

    write_to_file (data, size);

    status = pthread_mutex_unlock (&lock);
    if (status) {
        syslog (LOG_ERR, "Error code %d. %s.", status, strerror (status));
        return -1;
    }

    return 0;
}


int wr_delete_file () {
    int status = 0;

    syslog (LOG_DEBUG, "Deleting %s.", internal_file_path);
    status = unlink (internal_file_path);
    if (status != 0) {
        syslog (LOG_WARNING, "Error deleting file. %s.", strerror (errno));
    }
    else {
        syslog (LOG_DEBUG, "Successfully deleted file %s.", 
            internal_file_path);
    }

    return status;
}


static int write_to_file (const char *buffer, int size) {
    int bytes_written = 0;
    FILE *file = NULL;

    if (buffer == NULL) {
        syslog (LOG_WARNING, "Trying to write an empty buffer to the file.");
        return 0;
    }

    syslog (LOG_DEBUG, "Saving %d bytes to %s.", size, 
        internal_file_path);

    file = fopen (internal_file_path, "a+");
    if (file == NULL) {
        syslog (LOG_ERR, "Could not open file %s.", internal_file_path);
        return -1;
    }

    bytes_written = fwrite (buffer, 1, size, file);
    if (bytes_written != size) {
        syslog (LOG_ERR, "An error occured while writing the data"
            " to the file.");
        return -1;
    }

    fclose (file);
    return 0;
}
