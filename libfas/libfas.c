#include "libfas.h"
#include "fas.h"

#include <string.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

static int fas_dev_fd;

int fas_init() {

  fas_dev_fd = open(FAS_FILE_NAME, 0);
  return fas_dev_fd;

}

int fas_open(char* pathname, int flags, mode_t mode) {

  struct fas_open_args args;
  args.flags = flags;
  args.mode = mode;
  strncpy(args.pathname, pathname, PATH_MAX);

  return ioctl(fas_dev_fd, FAS_IOCTL_OPEN, &args);

}

