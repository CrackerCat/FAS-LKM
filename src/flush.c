#include "fas_private.h"

int fas_file_flush(struct file * filep, fl_owner_t id) {

  FAS_DEBUG("fas_file_flush: %p", filep);
  
  struct fas_filp_info* finfo = radix_tree_lookup(&fas_files_tree, (unsigned long)filep);
  
  if (finfo == NULL) return -EINVAL;
  
  FAS_DEBUG("fas_file_flush: found finfo! %p", finfo);
  FAS_DEBUG("fas_file_flush:   finfo->filp = %p", finfo->filp);
  FAS_DEBUG("fas_file_flush:   finfo->orig_f_op = %p", finfo->orig_f_op);
  FAS_DEBUG("fas_file_flush:   finfo->is_w = %d", finfo->is_w);
  
  if (finfo->is_w) {
    int r = fas_filp_copy(filep, finfo->filp);
    if (r < 0) return r;
  }
  
  if (finfo->orig_f_op->flush)
    return finfo->orig_f_op->flush(filep, id);
  return 0;

}
EXPORT_SYMBOL(fas_file_flush);
