#include "fas_private.h"

int fas_file_release(struct inode *inodep, struct file *filep) {

  FAS_DEBUG("fas_file_release: %p", filep);
  
  return 0;

}

EXPORT_SYMBOL(fas_file_release);
