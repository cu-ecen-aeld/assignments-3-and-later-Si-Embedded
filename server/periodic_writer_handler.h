#ifndef PERIODIC_WRITER_HANDLER_H
#define PERIODIC_WRITER_HANDLER_H



void pw_spawn_and_store_periodic_file_writer_thread (void);


void* pw_periodically_write_to_file (void *args);



#endif