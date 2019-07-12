#ifndef __LIBFAS_H__
#define __LIBFAS_H__

#include <sys/types.h>
#include <sys/stat.h>

int fas_open(char* pathname, int flags, mode_t mode);

#endif
