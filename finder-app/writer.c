/*
X Accepts the following arguments: the first argument is a full path to a file
    (including filename) on the filesystem, referred to below as writefile; 
    the second argument is a text string which will be written within this file, 
    referred to below as writestr

X Exits with value 1 error and print statements if any of the arguments above 
    were not specified

- Creates a new file with name and path writefile with content writestr, 
    overwriting any existing file. Exits with value 1 and error print statement 
    if the file could not be created.

X Setup syslog logging for your utility using the LOG_USER facility.

X Use the syslog capability to write a message “Writing <string> to <file>” 
    where <string> is the text string written to file (second argument) and 
    <file> is the file created by the script.  This should be written with 
    LOG_DEBUG level.

X Use the syslog capability to log any unexpected errors with LOG_ERR level.
*/

#include <syslog.h>
#include <stdio.h>



int main (int argc, char **argv) {
    openlog (argv [0], LOG_PERROR, LOG_USER);
    setlogmask (LOG_UPTO (LOG_DEBUG));

    if (argc != 3) {
        syslog (LOG_WARNING, "Incorrect amount of parameters passed.");
        syslog (LOG_WARNING, "Parameters passed is %d where 2 are required.", 
            argc - 1);
        return 1;
    }

    const char *filename = argv [1];
    const char *string = argv [2];
    FILE *file = fopen (filename, "w");

    if (file == NULL) {
        syslog (LOG_ERR, "Could not open file %s.", filename);
        return 1;
    }

    fprintf (file, string);
    syslog (LOG_DEBUG, "Writing %s to %s", string, filename);

    return 0;
}