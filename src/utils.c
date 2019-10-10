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

