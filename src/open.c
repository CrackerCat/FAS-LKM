#include "fas_private.h"

int fas_ioctl_open(char *filename, int flags, mode_t mode) {

  int r;

  FAS_DEBUG("fas_ioctl_open: (%p) %s, %x, %x", filename, filename, flags, mode);

  /* Session temporary files are not a thing. For O_PATH use regular open() */
  if (flags & (O_TMPFILE | O_PATH)) return -EINVAL;

  struct path i_path;
  if (kern_path(fas_initial_path, 0, &i_path)) {

    FAS_WARN(
        "fas_ioctl_open: the FAS initial path is not a valid path, please "
        "change it writing in /sys/kernel/fas/initial_path");
    return -EINVAL;

  }

  struct file *a_filp = NULL;
  mm_segment_t oldfs;

  int is_w = (flags & O_WRONLY) | (flags & O_RDWR);
  int a_flags = flags & ~(O_WRONLY | O_RDONLY);
  if (is_w)
    a_flags &= O_RDWR;
  else
    a_flags &= O_RDONLY;

  int fd = get_unused_fd_flags(O_TMPFILE | O_EXCL | O_RDWR);
  FAS_DEBUG("fas_ioctl_open: fd = %d", fd);
  if (fd < 0) goto exit_session_open;

  oldfs = get_fs();
  set_fs(KERNEL_DS);
  a_filp = filp_open(filename, a_flags, mode);
  set_fs(oldfs);

  FAS_DEBUG("fas_ioctl_open: a_filp = %p, %d, %d", a_filp, a_flags, mode);
  if (IS_ERR(a_filp)) {

    FAS_DEBUG("fas_ioctl_open: failed to open a_filp");
    r = PTR_ERR(a_filp);
    goto error1_session_open;

  }

  if (!fas_is_subpath(&i_path, &a_filp->f_path)) {

    FAS_DEBUG("fas_ioctl_open: not a subpath of initial_path!");
    r = -EINVAL;
    goto error2_session_open;

  }

  oldfs = get_fs();
  set_fs(KERNEL_DS);
  struct file *b_filp =
      filp_open(fas_initial_path, O_TMPFILE | O_EXCL | O_RDWR, 0644);
  set_fs(oldfs);

  FAS_DEBUG("fas_ioctl_open: b_filp = %p", b_filp);
  if (IS_ERR(b_filp)) {

    FAS_DEBUG("fas_ioctl_open: failed to open b_filp");
    r = PTR_ERR(b_filp);
    goto error2_session_open;

  }

  r = fas_filp_copy(a_filp, b_filp);
  if (r < 0) goto error3_session_open;

  struct fas_filp_info *finfo =
      kmalloc(sizeof(struct fas_filp_info), GFP_KERNEL);

  if (finfo == NULL) {

    r = -ENOMEM;
    goto error3_session_open;

  }

  char *out_pathname = d_path(&a_filp->f_path, finfo->pathname, PATH_MAX);
  memmove(finfo->pathname, out_pathname, strlen(out_pathname) + 1);

  finfo->orig_f_op = (struct file_operations *)b_filp->f_op;
  finfo->flags = a_flags & ~(O_CREAT | O_EXCL);
  finfo->is_w = (is_w != 0);

  FAS_DEBUG("fas_ioctl_open: generated finfo = %p", finfo);
  FAS_DEBUG("fas_ioctl_open:   finfo->orig_f_op = %p", finfo->orig_f_op);
  FAS_DEBUG("fas_ioctl_open:   finfo->pathname  = %s", finfo->pathname);
  FAS_DEBUG("fas_ioctl_open:   finfo->flags     = %d", finfo->flags);
  FAS_DEBUG("fas_ioctl_open:   finfo->is_w      = %d", finfo->is_w);

  struct file_operations *new_fops =
      kmalloc(sizeof(struct file_operations), GFP_KERNEL);

  if (new_fops == NULL) {

    r = -ENOMEM;
    goto error4_session_open;

  }

  FAS_DEBUG("fas_ioctl_open: new_fops = %p", new_fops);

  if (radix_tree_insert(&fas_files_tree, (unsigned long)b_filp, finfo) < 0) {

    kfree(new_fops);
    r = -ENOMEM;
    goto error4_session_open;

  }

  memcpy(new_fops, b_filp->f_op, sizeof(struct file_operations));

  new_fops->release = &fas_file_release;
  new_fops->flush = &fas_file_flush;

  b_filp->f_op = new_fops;

  filp_close(a_filp, NULL);

  atomic_long_add(1, &fas_opened_sessions_num);

  fsnotify_open(b_filp);
  fd_install(fd, b_filp);

exit_session_open:
  return fd;

error4_session_open:
  kfree(finfo);
error3_session_open:
  filp_close(b_filp, NULL);
error2_session_open:
  filp_close(a_filp, NULL);
error1_session_open:
  put_unused_fd(fd);
  path_put(&i_path);

  return r;

}

