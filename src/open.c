#include "fas_private.h"

static int filp_copy(struct file *src, struct file* dst) {

  long r, n;
  char buf[512];
	unsigned long long i = 0;
	unsigned long long j = 0;
	
	do {
	  n = vfs_read(src, buf, 512, &i);
	  
	  while (n) {
	    r = vfs_write(dst, buf + j, n, &j);
	    if (r < 0) return r;
	    
	    n -= r;
	  }
	  
	} while (n > 0);
	
	if (n < 0) return n;
	return 0;
}

int fas_ioctl_open(char* filename, int flags, mode_t mode) {

  int r;
  
  FAS_WARN("fas_ioctl_open: %s, %x, %x", filename, flags, mode);
  
  /* Session temporary files are not a thing. For O_PATH use regular open() */
  if (flags & (O_TMPFILE | O_PATH))
    return -EINVAL;
  
  struct file *a_filp = NULL;
  mm_segment_t oldfs;
  
  int is_w = (flags & O_WRONLY) | (flags & O_RDWR);
  int a_flags = flags & ~(O_WRONLY | O_RDONLY);
  if (is_w) a_flags &= O_RDWR;
  else a_flags &= O_RDONLY;

  oldfs = get_fs();
  set_fs(KERNEL_DS); /* Set fs related to kernel space */
  a_filp = filp_open(filename, a_flags, mode);
  set_fs(oldfs);
  
  if (IS_ERR(a_filp))
    return PTR_ERR(a_filp);

  int fd = fas_do_sys_open(0, "/tmp", O_TMPFILE | O_EXCL | O_RDWR, 0);
  struct file *b_filp = fget(fd);
  if (b_filp == NULL) // Should never happen
    return -1;

  r = filp_copy(a_filp, b_filp);
  if (r < 0) return r;

  return fd;
  
}
