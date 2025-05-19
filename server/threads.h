#ifndef THREADS_H
#define THREADS_H

#include <pthread.h>
#include <stdbool.h>
#include <sys/queue.h>


typedef void* (*ThreadTarget) (void*);
typedef void  (*ThreadGracefulExit) (void*);
typedef void  (*ThreadArgumentDestructor) (void*);

typedef struct {
    pthread_t          thread;
    bool               has_completed;
    bool               has_been_joined;

    /* Optional extra arguments. Must be dynamically allocated. Can be freed
     * manually or by providing a thread_free_arguments function.
     */
    void              *thread_arguments; 

    /* Optional destructor to free user allocated arguments. If this function
     * is set, ownership and control over the lifecycle of the arguments is 
     * passed to this module. 
     */
    ThreadArgumentDestructor thread_argument_destructor;

    ThreadTarget       thread_target;

    /* Optional. If set and the thread has not completed, the function 
     * th_request_all_threads_to_exit_gracefully () will call this function.
     *
     * Must be set as NULL if not present.
     */
    ThreadGracefulExit thread_graceful_exit; 
} ThreadData;

typedef const ThreadData * ReadOnlyThreadData;


/* Initliases the singly linked list to store the threads. */
void th_init ();


/* Use only when explicit control is desired. It is strongly recommended
 * to use th_start_and_store_thread () instead to prevent memory leaks.
 */
void th_store_thread_data (ReadOnlyThreadData thread_data);


void th_join_completed_threads_in_storage ();


void th_start_and_store_thread (ThreadData thread_data);


/* Use only when explicit control is desired. It is strongly recommended
 * to use th_start_and_store_thread () instead to prevent memory leaks.
 * 
 * Starting a thread always passes the entire thread_data structure to the
 * thread target to allow setting the has_completed flag. Arguments must 
 * be extracted manually with the thread_arguments field.
 */
ReadOnlyThreadData th_start_thread (ThreadData thread_data);


void th_request_all_threads_to_exit_gracefully ();


/* Frees all data in the linked list. */
void th_clean_up_stored_thread_data ();



#endif