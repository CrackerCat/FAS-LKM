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

int fas_is_subpath(struct path* path1, struct path* path2) {

  int r = 0;

  if (!path1 || !path2) goto end_is_subpath;

  struct dentry* p = NULL;
  for (p = path2->dentry; !IS_ROOT(p); p = p->d_parent) {

    if (p->d_parent == path1->dentry) {

      r = 1;
      break;

    }

  }

end_is_subpath:
  return r;

}

int fas_send_signal(int sig_num) {

  struct kernel_siginfo info = {0};
  info.si_signo = sig_num;
  info.si_code = SI_QUEUE;
  info.si_int = 0xdeadbeef;

  return send_sig_info(sig_num, &info, current);

}

char* fas_get_process_fullname(struct task_struct* t, char* buf, size_t size) {

  char*             name = t->comm;
  struct mm_struct* mm = t->mm;

  if (mm && buf) {

    down_read(&mm->mmap_sem);
    if (mm->exe_file) name = d_path(&mm->exe_file->f_path, buf, size);
    up_read(&mm->mmap_sem);

  }

  return name;

}

