#include "fas_private.h"

struct dentry *fas_tmpfile(struct dentry *dentry, umode_t mode, int open_flag) {

  struct dentry *child = NULL;
  struct inode *dir = dentry->d_inode;
  struct inode *inode;
  int error;

  /* BE AWARE This routine does not perform checks on permissions */

  error = -EOPNOTSUPP;
  if (!dir->i_op->tmpfile)
    goto out_err;

  error = -ENOMEM;
  child = d_alloc(dentry, &slash_name);
  if (unlikely(!child))
    goto out_err;

  error = dir->i_op->tmpfile(dir, child, mode);
  if (error)
    goto out_err;

  error = -ENOENT;
  inode = child->d_inode;
  if (unlikely(!inode))
    goto out_err;
  if (!(open_flag & O_EXCL)) {
  
    spin_lock(&inode->i_lock);
    inode->i_state |= I_LINKABLE;
    spin_unlock(&inode->i_lock);
  }
  
  ima_post_create_tmpfile(inode);
  return child;

out_err:
  dput(child);
  return ERR_PTR(error);
}

int do_fas_tmpfile(struct nameidata *nd, unsigned flags,
                   const struct open_flags *op, struct file *file) {

  struct dentry *child;
  struct dentry *parent;
  struct path path;
  
  int error = path_lookupat(nd, flags | LOOKUP_DIRECTORY, &path);
  if (unlikely(error))
    return error;
  
  error = mnt_want_write(path.mnt);
  if (unlikely(error))
    goto out;
  
  parent = path.dentry->parent;
  
  if (unlikely(!parent)) {
    error = -ENOENT;
    goto out2;
  }
  
  child = fas_tmpfile(parent, op->mode, op->open_flag);
  error = PTR_ERR(child);
  if (IS_ERR(child))
    goto out2;
  
  dput(parent);
  path.dentry = child;
  audit_inode(nd->name, child, 0);
  
  file->f_path.mnt = path.mnt;
  error = finish_open(file, child, NULL);

out2:
  mnt_drop_write(path.mnt);
out:
  path_put(&path);
  return error;
}


struct file* do_fas_tmp_filp_open(struct filename* pathname,
                                  const struct open_flags *op) {

  struct nameidata nd;
  int flags = op->lookup_flags;
  struct file *filp;
  int error;

  set_nameidata(&nd, dfd, pathname);
  
  filp = alloc_empty_file(op->open_flag, current_cred());
  if (!IS_ERR(filp)) {
    
    error = do_fas_tmpfile(&nd, flags, op);
    if (error)
      filp = ERR_PTR(error);
  }
  
  // TODO handle EOPENSTALE
  restore_nameidata();
  return filp;
}


int fas_ioctl_open(char* filename, int flags, mode_t mode) {

  /* Session temporary files are not a thing. For O_PATH use regular open() */
  if (flags & (O_TMPFILE | O_PATH))
    return -EINVAL;
  
  struct open_flags op;
  struct filename *tmp;
  
  int fd = build_open_flags(flags, mode, &op);
  if (fd)
    return fd;
  
  tmp = getname(filename);
  if (IS_ERR(tmp))
    return PTR_ERR(tmp);
  
  struct file *f = do_fas_tmp_filp_open(tmp, &op);
  if (IS_ERR(f))
    return PTR_ERR(f);

  
  
  return 0;
}
