#include "libfas.h"
#include "fas.h"

#include <sys/ioctl.h>

static int fas_dev_fd;

int fas_init() {

  fas_dev_fd = open(FAS_FILE_NAME, 0);
  return fas_dev_fd;
}

int fas_open(char* pathname, int flags, mode_t mode) {

  struct fas_open_args args = {
    pathname,
    flags,
    mode
  };
  
  return ioctl(fas_dev_fd, FAS_IOCTL_OPEN, &args);
}
