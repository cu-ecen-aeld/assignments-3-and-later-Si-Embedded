/* Provides a synchronised and reentrant way to 
 * write to the defined file. The module intends
 * to write to a single compiletime defined file.
 */

#ifndef FILE_IO_H
#define FILE_IO_H

#include <stdio.h>



int wr_init (char *file_path);
int wr_cleanup (void);
const char *wr_get_file_path (void);
int wr_write (const char *data, int size);
int wr_delete_file ();


#endif