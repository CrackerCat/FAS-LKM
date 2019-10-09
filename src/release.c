#include "fas_private.h"

int fas_file_release(struct inode *inodep, struct file *filep) {

  FAS_DEBUG("fas_file_release: %p", filep);

  struct fas_filp_info *finfo =
      radix_tree_delete(&fas_files_tree, (unsigned long)filep);

  if (finfo == NULL) return -EINVAL;

  FAS_DEBUG("fas_file_release: deleted finfo! %p", finfo);
  FAS_DEBUG("fas_file_release:   finfo->filp = %p", finfo->filp);
  FAS_DEBUG("fas_file_release:   finfo->orig_f_op = %p", finfo->orig_f_op);
  FAS_DEBUG("fas_file_release:   finfo->is_w = %d", finfo->is_w);

  filp_close(finfo->filp, NULL);

  int r = 0;

  struct file_operations *new_fops = (struct file_operations *)filep->f_op;
  filep->f_op = finfo->orig_f_op;
  kfree(new_fops);

  if (finfo->orig_f_op->release) r = finfo->orig_f_op->release(inodep, filep);

  kfree(finfo);

  return r;

}

EXPORT_SYMBOL(fas_file_release);

