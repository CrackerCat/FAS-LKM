#include "fas_private.h"

long fas_dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg) {

  switch (cmd) {
    case FAS_IOCTL_NOP:
    break;
  
    case FAS_IOCTL_OPEN: {
      struct fas_open_args* open_args = (struct fas_open_args*)arg;
      return fas_ioctl_open(open_args->pathname, open_args->flags,
                            open_args->mode);
      break;
    }
  }
  
  return 0;
}
EXPORT_SYMBOL(fas_dev_ioctl);
