#include "fas_private.h"

int fas_filp_copy(struct file* src, struct file* dst) {

  long               r, n;
  char               buf[512];
  unsigned long long i = 0;
  unsigned long long j = 0;

  while (1) {

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

int fas_is_subpath(char* super_pathname, char* sub_pathname) {

  struct path path1, path2;
  int r = 0;
  
  int follow_param = 0;
  if (follow_links) follow_param = LOOKUP_FOLLOW;
  
  if (!super_pathname || kern_path(super_pathname, 0, &path1))
    goto end_is_subpath;
  
  if (!sub_pathname || kern_path(sub_pathname, follow_param, &path2))
    goto path1_cleanup;

  r = d_ancestor(path1->dentry, path2->dentry) != NULL;

  path_put(&path2);

path1_cleanup:
  path_put(&path1);

end_is_subpath:
  return r;

}
