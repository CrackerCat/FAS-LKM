#include "fas_private.h"

/* /sys/kernel/fas/initial_path */
char fas_initial_path[PATH_MAX] = {'/', 't', 'm', 'p', 0};
EXPORT_SYMBOL(fas_initial_path);

ssize_t fas_initial_path_show(struct kobject *kobj, struct kobj_attribute *attr,
                              char *buf) {

  size_t r = snprintf(buf, PAGE_SIZE, "%s\n", fas_initial_path);
  if (r > PAGE_SIZE) r = PAGE_SIZE;
  return r;

}

ssize_t fas_intial_path_store(struct kobject *kobj, struct kobj_attribute *attr,
                              const char *buf, size_t count) {

  size_t size = count;
  if (count > PATH_MAX) size = PATH_MAX;

  memset(fas_initial_path, 0, PATH_MAX);
  strncpy(fas_initial_path, buf, size);

  size = strlen(fas_initial_path);
  while (size && fas_initial_path[size - 1] == '\n')
    --size;

  fas_initial_path[size] = 0;

  struct path i_path;
  if (kern_path(fas_initial_path, 0, &i_path)) {

    FAS_WARN(
        "fas_intial_path_store: the inserted FAS initial path (%s) is not a "
        "valid "
        "path, please retry inserting a valid path",
        fas_initial_path);

  }

  return count;

}

/* /sys/kernel/fas/sessions_num */
atomic_long_t fas_opened_sessions_num;
EXPORT_SYMBOL(fas_opened_sessions_num);

ssize_t fas_sessions_num_show(struct kobject *kobj, struct kobj_attribute *attr,
                              char *buf) {

  return snprintf(buf, PAGE_SIZE, "%ld\n",
                  atomic_long_read(&fas_opened_sessions_num));

  /* // Count sessions also using with refcounts

  struct radix_tree_iter iter;
  void **slot;
  long counter = 0;

  radix_tree_for_each_slot(slot, &fas_files_tree, &iter,0) {

    rcu_read_lock();
    struct fas_filp_info *finfo = radix_tree_deref_slot(slot);
    if (finfo != NULL && finfo->filp)
      counter += atomic_long_read(&finfo->filp->f_count);
    rcu_read_unlock();

  }

  return snprintf(buf, PAGE_SIZE, "%ld", counter);*/

}

struct fas_files_h_struct {

  char              key[PATH_MAX];
  int               counter;
  struct hlist_node node;

};

struct fas_htable {

  DECLARE_HASHTABLE(ht, 16);

};

static int fas_key_hash(char *key) {

  size_t        keylen = strlen(key);
  unsigned long hashval = 0;
  int           i = 0;

  while (hashval < ULONG_MAX && i < keylen) {

    hashval = hashval << 8;
    hashval += key[i];
    i++;

  }

  return hashval % (1 << 16);

}

ssize_t fas_sessions_each_file_show(struct kobject *       kobj,
                                    struct kobj_attribute *attr, char *buf) {

  struct fas_htable *table = kzalloc(sizeof(struct fas_htable), GFP_KERNEL);
  if (table == NULL) return 0;                   /* Cannot return ENOMEM :( */

  hash_init(table->ht);

  struct radix_tree_iter     iter;
  struct fas_files_h_struct *entry;

  void **slot;
  char   path_buf[PATH_MAX];

  int i = 0;

  radix_tree_for_each_slot(slot, &fas_files_tree, &iter, 0) {

    read_lock(&fas_files_tree_lock);
    rcu_read_lock();

    struct fas_filp_info *finfo = radix_tree_deref_slot(slot);
    if (finfo != NULL)
      memcpy(path_buf, finfo->pathname, PATH_MAX);

    rcu_read_unlock();
    read_unlock(&fas_files_tree_lock);

    if (finfo == NULL) continue;

    int key = fas_key_hash(path_buf);
    int present = 0;

    hash_for_each_possible(table->ht, entry, node, key) {

      if (strcmp(entry->key, path_buf)) continue;

      present = 1;
      entry->counter++;
      break;

    }

    if (!present) {

      entry = kzalloc(sizeof(struct fas_files_h_struct), GFP_KERNEL);
      if (entry == NULL) goto exit_list_files;

      memcpy(entry->key, path_buf, PATH_MAX);
      entry->counter = 1;
      hash_add(table->ht, &entry->node, key);

    }

  }

  int bkt;
  hash_for_each(table->ht, bkt, entry, node) {

    if (entry->counter) {

      size_t size = PAGE_SIZE - i;
      size_t r = snprintf(buf + i, size, "%s %d\n", entry->key, entry->counter);

      if (r > size) {

        i = PAGE_SIZE;
        goto exit_list_files;

      }

      i += r;

    }

  }

exit_list_files:

  kfree(table);
  return i;

}

ssize_t fas_processes_show(struct kobject *kobj, struct kobj_attribute *attr,
                           char *buf) {

  struct task_struct *t;
  char                name_buf[PATH_MAX];

  int i = 0;
  
  struct files_struct * (*f_get_files_struct)(struct task_struct *) = (void*)kallsyms_lookup_name("get_files_struct");
  void (*f_put_files_struct)(struct files_struct *) = (void*)kallsyms_lookup_name("put_files_struct");

  if (f_get_files_struct == NULL || f_put_files_struct == NULL) {
    return snprintf(buf, PAGE_SIZE, " **** kallsyms not enabled on this kernel build ***\n");
  }

  rcu_read_lock();
  for_each_process(t) { /* Basically this is a list_entry_rcu */

    rcu_read_unlock();
    
    struct files_struct *files = f_get_files_struct(t);

    if (files == NULL) continue;

    int fd;
    int use_sessions = 0;

    struct fdtable *fdt = files_fdtable(files);
  
    spin_lock(&files->file_lock);
    for (fd = 0; fd < fdt->max_fds; ++fd) {

      if (fdt->fd[fd] == NULL) continue;

      rcu_read_lock();
      struct fas_filp_info *finfo =
          radix_tree_lookup(&fas_files_tree, (unsigned long)fdt->fd[fd]);
      rcu_read_unlock();

      /* finfo is only used here. We choosed to avoid locking in order to not
         decrease performance. In the unlikely case that finfo is freed
         between the radix_tree_lookup() call and this statement a simple
         pointer comparison will not affect the state of the kernel. */
      if (finfo == NULL) continue;

      use_sessions = 1;
      break;

    }
    spin_unlock(&files->file_lock);

    f_put_files_struct(files);

    if (use_sessions) {

      memset(name_buf, 0, PATH_MAX);

      size_t size = PAGE_SIZE - i;

      size_t r =
          snprintf(buf + i, size, "%s %d\n",
                   fas_get_process_fullname(t, name_buf, PATH_MAX), t->pid);

      if (r > size) {

        i = PAGE_SIZE;
        goto exit_list_procs;

      }

      i += r;

    }

    rcu_read_lock();

  }

  rcu_read_unlock();

exit_list_procs:

  return i;

}

