#include "fas_private.h"

long fas_dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {

  switch (cmd) {
    case FAS_IOCTL_NOP:
    break;
  
    case FAS_IOCTL_OPEN: {
      struct fas_open_args open_args;
      if (copy_from_user(&open_args, (void*)arg, sizeof(struct fas_open_args)))
        return -EINVAL;
      
      return fas_ioctl_open(open_args.pathname, open_args.flags,
                            open_args.mode);
      break;
    }
  }
  
  return 0;
}
EXPORT_SYMBOL(fas_dev_ioctl);
