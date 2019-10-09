#include "fas_private.h"

int fas_filp_copy(struct file *src, struct file* dst) {

  long r, n;
  char buf[512];
	unsigned long long i = 0;
	unsigned long long j = 0;
	
	while(1) {

	  n = kernel_read(src, buf, 512, &i);

    if (n <= 0) break;

    FAS_DEBUG("fas_filp_copy: readed %ld bytes:", n);
    FAS_DEBUG_HEXDUMP(buf, n);

	  long k = n;
	  while (k) {
	    r = kernel_write(dst, buf + j, k, &j);

	    if (r < 0) return r;
	    
	    k -= r;
	  }
	  
	}
	
	if (n < 0) return n;
	return 0;
}

int fas_ioctl_open(char* filename, int flags, mode_t mode) {

  int r;
  
  FAS_DEBUG("fas_ioctl_open: %s, %x, %x", filename, flags, mode);
  
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
  
  FAS_DEBUG("fas_ioctl_open: a_filp = %p", a_filp);
  
  if (IS_ERR(a_filp))
    return PTR_ERR(a_filp);

  int fd = get_unused_fd_flags(O_TMPFILE | O_EXCL | O_RDWR);
  if (fd < 0)
    return fd;
    
  FAS_DEBUG("fas_ioctl_open: fd = %d", fd);
  
  oldfs = get_fs();
  set_fs(KERNEL_DS); /* Set fs related to kernel space */
  struct file *b_filp = filp_open("/tmp", O_TMPFILE | O_EXCL | O_RDWR, 0644);
  set_fs(oldfs);

  FAS_DEBUG("fas_ioctl_open: b_filp = %p", b_filp);

  if (IS_ERR(b_filp)) {
    filp_close(a_filp, NULL);
    put_unused_fd(fd);
    return PTR_ERR(a_filp);
  }

  //fsnotify_open(b_filp);
  fd_install(fd, b_filp);
  
  r = fas_filp_copy(a_filp, b_filp);
  if (r < 0) return r;

  return fd;
  
}
