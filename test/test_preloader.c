#define _GNU_SOURCE
#include <stdio.h>
#include <fcntl.h>
#include <dlfcn.h>
#include <stdarg.h>
#include <string.h>
#include "libfas.h"
#include "fas.h"

int (*__libc_open)(const char *__file, int __oflag, ...);

__attribute__((constructor))
static void __init() {

  __libc_open = dlsym(RTLD_NEXT, "open");
  fas_init();

}

int open(const char *pathname, int flags, ...) {

  mode_t mode = 0;
  int fd;
  
  if (flags & (O_CREAT | O_TMPFILE)) {

    va_list arg;
    va_start(arg, flags);
    mode = va_arg(arg, int);
    va_end(arg);

  }
  
  struct stat info;
  stat(pathname, &info);
  
  if (S_ISCHR(info.st_mode) || S_ISBLK(info.st_mode) || (flags & (O_CREAT | O_PATH | O_TMPFILE))) {
    
    fd = __libc_open(pathname, flags, mode);
    fprintf(stderr, "FAS test_prelaoder: open(%s, %x, %x) = %d\n", pathname, flags, mode, fd);

  } else {
  
    fd = fas_open(pathname, flags);
    fprintf(stderr, "FAS test_prelaoder: fas_open(%s, %x) = %d\n", pathname, flags, fd);

  }

  return fd;

}

