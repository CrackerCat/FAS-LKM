#ifndef __FAS_H__
#define __FAS_H__

#include <linux/ioctl.h>

#define FAS_FILE_NAME "/dev/fas"

#define FAS_IOCTL_MAGIC 52634

struct fas_open_args {

  char* pathname;
  int flags;
  mode_t mode;
};

#define FAS_IOCTL_NOP _IO(FAS_IOCTL_MAGIC, 1)
#define FAS_IOCTL_OPEN _IOR(FAS_IOCTL_MAGIC, 2, struct fas_open_args*)

#endif
