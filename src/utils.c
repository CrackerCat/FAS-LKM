#include "fas_private.h"

int fas_filp_copy(struct file* src, struct file* dst) {

  long               r;
  char               buf[512];
  unsigned long long i = 0;
  unsigned long long j = 0;
  unsigned long long k = 0;
  unsigned long long n;

  while (1) {
    
    unsigned long long o_i = i;

    r = kernel_read(src, buf, 512, &i);

    if (r <= 0) break;
    
    n = i - o_i;

    FAS_DEBUG("fas_filp_copy: readed %llu bytes:", n);
    FAS_DEBUG_HEXDUMP(buf, n);

    for (k = 0; k < n; ) {
    
      unsigned long long o_j = j;

      r = kernel_write(dst, buf + k, n - k, &j);

      if (r < 0) return r;

      k += (j - o_j);

    }

  }

  if (r < 0) return r;
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

