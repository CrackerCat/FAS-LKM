#include "fas_private.h"
#include <linux/fs.h>
#include <asm/segment.h>
#include <asm/uaccess.h>
#include <linux/buffer_head.h>

int fas_ioctl_open(char* filename, int flags, mode_t mode) {

  /* Session temporary files are not a thing. For O_PATH use regular open() */
  if (flags & (O_TMPFILE | O_PATH))
    return -EINVAL;
  
  struct file *filp = NULL;
  mm_segment_t oldfs;
  int err = 0;

  oldfs = get_fs();
  set_fs(get_ds());
  filp = filp_open(filename, flags, mode);
  set_fs(oldfs);
  
  if (IS_ERR(filp)) {
  
    err = PTR_ERR(filp);
    return -1;
  }
  return filp;

  
  
  return 0;
}
