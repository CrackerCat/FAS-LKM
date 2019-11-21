#ifndef __LIBFAS_H__
#define __LIBFAS_H__

#include <sys/stat.h>
#include <sys/types.h>

/* Initialize the FAS device. Put this in yout main() before starting any
   thread. Returns the file descriptor of the FAS device. */
int fas_init();

/* Open a session on a file. Unsupported flags are:
    - O_TMPFILE
    - O_PATH
    - O_CREAT */
int fas_open(const char* pathname, int flags);

#endif

