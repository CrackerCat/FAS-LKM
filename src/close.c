#include "fas_private.h"

int fas_file_flush(struct file *filep, fl_owner_t id) {

  struct file *a_filp = NULL;
  mm_segment_t oldfs;

  FAS_DEBUG("fas_file_flush: %p", filep);

  /* https://lwn.net/Articles/175432/
     Flush is an atomic context for the resulting pointer (release is always
     after and not concurrent) and so RCU only is fine here. */
  // read_lock(&fas_files_tree_lock);
  rcu_read_lock();
  struct fas_filp_info *finfo =
      radix_tree_lookup(&fas_files_tree, (unsigned long)filep);
  rcu_read_unlock();
  // read_unlock(&fas_files_tree_lock);

  if (finfo == NULL) {                             /* Should *never* happen */
    FAS_FATAL(
        "radix_tree_lookup of fas_files_tree returned NULL. This is a critical "
        "state reached for an unknow reason, unload the module ASAP or, "
        "better, reboot");
    return -EINVAL;

  }

  FAS_DEBUG("fas_file_flush: found finfo = %p", finfo);
  FAS_DEBUG("fas_file_flush:   finfo->orig_f_op = %p", finfo->orig_f_op);
  FAS_DEBUG("fas_file_flush:   finfo->pathname  = %s", finfo->pathname);
  FAS_DEBUG("fas_file_flush:   finfo->flags     = %d", finfo->flags);
  FAS_DEBUG("fas_file_flush:   finfo->is_w      = %d", finfo->is_w);

  if (finfo->is_w) {

    oldfs = get_fs();
    set_fs(KERNEL_DS);
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

  int r = 0;

  if (finfo->orig_f_op->flush) r = finfo->orig_f_op->flush(filep, id);

  FAS_DEBUG("fas_file_flush: return %d", r);
  return r;

}

EXPORT_SYMBOL(fas_file_flush);

int fas_file_release(struct inode *inodep, struct file *filep) {

  FAS_DEBUG("fas_file_release: %p", filep);

  /* Should we use RCU here? Doc says nothing... */
  write_lock(&fas_files_tree_lock);
  struct fas_filp_info *finfo =
      radix_tree_delete(&fas_files_tree, (unsigned long)filep);
  write_unlock(&fas_files_tree_lock);

  atomic_long_sub(1, &fas_opened_sessions_num);

  if (finfo == NULL) {                             /* Should *never* happen */
    FAS_FATAL(
        "radix_tree_delete of fas_files_tree returned NULL. This is a critical "
        "state reached for an unknow reason, unload the module ASAP or, "
        "better, reboot");
    return -EINVAL;

  }

  FAS_DEBUG("fas_file_release: found finfo = %p", finfo);
  FAS_DEBUG("fas_file_release:   finfo->orig_f_op = %p", finfo->orig_f_op);
  FAS_DEBUG("fas_file_release:   finfo->pathname  = %s", finfo->pathname);
  FAS_DEBUG("fas_file_release:   finfo->flags     = %d", finfo->flags);
  FAS_DEBUG("fas_file_release:   finfo->is_w      = %d", finfo->is_w);

  int r = 0;

  struct file_operations *new_fops = (struct file_operations *)filep->f_op;
  filep->f_op = finfo->orig_f_op;

  synchronize_rcu();                                /* Wait all RCU readers */

  kfree(finfo);
  kfree(new_fops);
  FAS_DEBUG("fas_file_release: successfully released memory");

  if (filep->f_op && filep->f_op->release)
    r = filep->f_op->release(inodep, filep);

  FAS_DEBUG("fas_file_release: return %d", r);
  return r;

}

EXPORT_SYMBOL(fas_file_release);

