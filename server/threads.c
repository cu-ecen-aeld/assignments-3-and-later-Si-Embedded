#include "threads.h"

#include <syslog.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


struct Node {
    ThreadData *thread_data;
    SLIST_ENTRY (Node) entries;
};

SLIST_HEAD (slisthead, Node) head;


void th_init () {
    SLIST_INIT (&head);
}


void th_store_thread_data (ReadOnlyThreadData thread_data) {
    if (thread_data == NULL) {
        syslog (LOG_WARNING, "Attempting to store empty (null) thread data.");
        return;
    }
    ThreadData *element = (ThreadData*) thread_data;

    struct Node *node = malloc (sizeof (struct Node));
    if (node == NULL) {
        syslog (LOG_WARNING, "Unable to malloc."); 
        return;
    }

    syslog (LOG_DEBUG, "Storing thread %lu.", element->thread);

    /* Copy thread data into the node. */
    node->thread_data = element;

    /* Store the node. This module keeps ownership, 
     * freeing is done in th_clean_up_stored_thread_data ().
     */
    SLIST_INSERT_HEAD (&head, node, entries);
}


void th_join_completed_threads_in_storage () {
    struct Node *node = NULL;
    SLIST_FOREACH (node, &head, entries) {
        if (node->thread_data->has_completed
            && !node->thread_data->has_been_joined) 
        {
            pthread_join (node->thread_data->thread, NULL);
            syslog (LOG_DEBUG, "Joined with completed thread %lu.", 
                node->thread_data->thread);
            node->thread_data->has_been_joined = true;
        }
    }
}


ReadOnlyThreadData th_start_thread (ThreadData thread_data) {
    /* Ownership of internal_thread_data belongs to this module. 
     * This data is freed through clean up of the stored ThreadData 
     * in th_clean_up_stored_thread_data ().
     */
    ThreadData *internal_thread_data = malloc (sizeof (ThreadData));
    if (internal_thread_data == NULL) {
        syslog (LOG_WARNING, "Unable to malloc."); 
        return NULL;
    }

    *internal_thread_data = thread_data;
    int status = pthread_create (
        &internal_thread_data->thread, 
        NULL, 
        thread_data.thread_target,
        internal_thread_data);

    if (status) {
        syslog (LOG_ERR, "Unable to start a new thread with status code %d."
            " %s", status, strerror (status));
    } else {
        syslog (LOG_DEBUG, "Started a new thread with thread id %lu.",
            internal_thread_data->thread);
    }

    /* Return ThreadData in read only format, so it can be 
     * explicitly stored by the caller if that amount of control is desired.
     */
    return (ReadOnlyThreadData) internal_thread_data;
}


void th_request_all_threads_to_exit_gracefully () {
    struct Node *node = NULL;

    syslog (LOG_DEBUG, "Requesting all incompleted threads to exit "
        "gracefully.");

    if (SLIST_EMPTY (&head)) {
        syslog (LOG_INFO, "No threads to request shutdown of.");
        return;
    }

    SLIST_FOREACH (node, &head, entries) {
        if (node->thread_data == NULL) {
            syslog (LOG_WARNING, "Thread data is already null. Unable to "
                " request a shutdown.");
            continue;
        }

        if (!node->thread_data->has_completed 
            && node->thread_data->thread_graceful_exit != NULL) 
        {
            node->thread_data->thread_graceful_exit (node->thread_data);
        }
    }
}


void th_clean_up_stored_thread_data () {
    syslog (LOG_DEBUG, "Cleaning up stored threads.");

    while (!SLIST_EMPTY (&head)) {
        struct Node *node = SLIST_FIRST (&head);
        ThreadData *data = node->thread_data;

        if (data == NULL) {
            syslog (LOG_ERR, "Thread data is invalid, unable to join thread.");
        }
        else {
            syslog (LOG_DEBUG, "Cleaning up thread %lu.", 
                data->thread);
    
            if (!data->has_been_joined) {
                syslog (LOG_DEBUG, "Joining with thread %lu.", 
                    data->thread);
                pthread_join (data->thread, NULL);
            }
    
            if (data->thread_arguments) { 
                if (data->thread_argument_destructor) {
                    syslog (LOG_DEBUG, "Calling user provided argument "
                        "destructor for thread %lu.", data->thread);
                    data->thread_argument_destructor (data);
                }
                else {
                    syslog (LOG_WARNING, "Thread arguments found during clean"
                        " up but no destructor. Set the pointer to NULL after"
                        " freeing to supress this warning.");
                }
            }

            /* Clean up of allocated memory in start_thread () is done here. */
            free (node->thread_data);
            node->thread_data = NULL;
        }

        SLIST_REMOVE_HEAD (&head, entries);
        free (node);
        node = NULL;
    }

    syslog (LOG_DEBUG, "Cleaned up all threads and freed memory.");
}


void th_start_and_store_thread (ThreadData thread_data) {
    th_store_thread_data (th_start_thread (thread_data));
}
