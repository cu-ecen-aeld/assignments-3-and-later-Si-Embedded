#include "file_io.h"

#include <pthread.h>
#include <syslog.h>
#include <string.h> 
#include <errno.h>
#include <unistd.h> /* For unlink (). */
#include <fcntl.h>



/* For internal use only with the current implementation. */
#define file_path_length 256
typedef struct {
    char path [file_path_length];
    int descriptor;
    pthread_mutex_t lock;
} SynchronisedFile;


static SynchronisedFile file = {0};


int wr_init (char *file_path) {
    size_t string_length = strnlen (file_path, file_path_length);
    if (string_length == file_path_length) {
        syslog (LOG_WARNING, "File path is as long as or longer than the "
            "buffer storing the file path. Please update the buffer size.");
    }

    strncpy (file.path, file_path, string_length);

    /* Open the file to prevent open close overhead. */
    file.descriptor = open (file.path, O_WRONLY | O_APPEND | O_CREAT, 0755);

    if (file.descriptor == 1) {
        syslog (LOG_ERR, "Error opening file %s. %s.", 
            file_path, strerror (errno));
        return -1;
    }

    syslog (LOG_DEBUG, "Opened %s with file descriptor %d.",
        file.path, file.descriptor);

    return pthread_mutex_init (&file.lock, NULL);
}


int wr_cleanup () {
    return close (file.descriptor);
}


const char *wr_get_file_path () {
    if (strnlen (file.path, file_path_length) == 0) {
        syslog (LOG_WARNING, "File path is not set.");
    }
    return file.path;
}


int wr_write (const char *data, int size) {
    int status = 0;
    int bytes_written = write (file.descriptor, data, size);

    if (bytes_written == -1) {
        syslog (LOG_ERR, "Failed to write to %s. %s.", 
            file.path, strerror (errno));
        status = -1;
    }

    syslog (LOG_DEBUG, "Written %d bytes out of the requested %d bytes to %s.",
        bytes_written, size, file.path);
   
    if (bytes_written != size) {
        syslog (LOG_WARNING, "Not all requested bytes were written.");
    }

    return status;
}


int wr_delete_file () {
    int status = 0;

    if (strnlen (file.path, file_path_length) == 0) {
        syslog (LOG_WARNING, "Trying to delete a file path that is not set. "
            "Returning");
        return -1;
    }

    syslog (LOG_DEBUG, "Deleting %s.", file.path);
    status = unlink (file.path);
    if (status != 0) {
        syslog (LOG_WARNING, "Error deleting file. %s.", strerror (errno));
    }
    else {
        syslog (LOG_DEBUG, "Successfully deleted file %s.", 
            file.path);
    }

    return status;
}
