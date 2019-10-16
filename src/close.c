#include "fas_private.h"

int fas_file_flush(struct file* filep, fl_owner_t id) {

  struct file* a_filp = NULL;
  mm_segment_t oldfs;

  FAS_DEBUG("fas_file_flush: %p", filep);

  struct fas_filp_info* finfo =
      radix_tree_lookup(&fas_files_tree, (unsigned long)filep);

  if (finfo == NULL) return -EINVAL;

  FAS_DEBUG("fas_file_flush: found finfo = %p", finfo);
  FAS_DEBUG("fas_file_flush:   finfo->orig_f_op = %p", finfo->orig_f_op);
  FAS_DEBUG("fas_file_flush:   finfo->pathname  = %s", finfo->pathname);
  FAS_DEBUG("fas_file_flush:   finfo->flags     = %d", finfo->flags);
  FAS_DEBUG("fas_file_flush:   finfo->is_w      = %d", finfo->is_w);

  if (finfo->is_w) {

    oldfs = get_fs();
    set_fs(KERNEL_DS);                    /* Set fs related to kernel space */
    a_filp = filp_open(finfo->pathname, finfo->flags, 0);
    set_fs(oldfs);

    if (IS_ERR(a_filp)) {

      fas_send_signal(SIGPIPE);
      return -EPIPE;

    }

    int r = fas_filp_copy(filep, a_filp);
    if (r < 0) {

      filp_close(a_filp, NULL);
      fas_send_signal(SIGPIPE);
      return -EPIPE;

    }

  }

  if (finfo->orig_f_op->flush) return finfo->orig_f_op->flush(filep, id);
  return 0;

}

EXPORT_SYMBOL(fas_file_flush);

int fas_file_release(struct inode *inodep, struct file *filep) {

  FAS_DEBUG("fas_file_release: %p", filep);

  --fas_opened_sessions_num;

  struct fas_filp_info *finfo =
      radix_tree_delete(&fas_files_tree, (unsigned long)filep);

  if (finfo == NULL) return -EINVAL;

  FAS_DEBUG("fas_file_release: found finfo = %p", finfo);
  FAS_DEBUG("fas_file_release:   finfo->orig_f_op = %p", finfo->orig_f_op);
  FAS_DEBUG("fas_file_release:   finfo->pathname  = %s", finfo->pathname);
  FAS_DEBUG("fas_file_release:   finfo->flags     = %d", finfo->flags);
  FAS_DEBUG("fas_file_release:   finfo->is_w      = %d", finfo->is_w);

  int r = 0;

  struct file_operations *new_fops = (struct file_operations *)filep->f_op;
  filep->f_op = finfo->orig_f_op;
  kfree(new_fops);

  if (finfo->orig_f_op->release) r = finfo->orig_f_op->release(inodep, filep);

  kfree(finfo->pathname);
  kfree(finfo);

  return r;

}

EXPORT_SYMBOL(fas_file_release);

