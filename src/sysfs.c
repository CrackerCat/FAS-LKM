#include "fas_private.h"

/* /sys/kernel/fas/initial_path */
char fas_initial_path[PATH_MAX + 1] = {'/', 't', 'm', 'p', 0};
EXPORT_SYMBOL(fas_initial_path);

ssize_t fas_initial_path_show(struct kobject *kobj, struct kobj_attribute *attr,
                              char *buf) {

  size_t size = strlen(fas_initial_path);
  strncpy(buf, fas_initial_path, size);
  return size;

}

ssize_t fas_intial_path_store(struct kobject *kobj, struct kobj_attribute *attr,
                              const char *buf, size_t count) {

  size_t size = count;
  if (count > PATH_MAX) size = PATH_MAX;

  strncpy(fas_initial_path, buf, size);

  size = strlen(fas_initial_path);
  while (size && fas_initial_path[size - 1] == '\n')
    --size;

  fas_initial_path[size] = 0;

  return count;

}

/* /sys/kernel/fas/sessions_num */
long fas_opened_sessions_num;
EXPORT_SYMBOL(fas_opened_sessions_num);

ssize_t fas_sessions_num_show(struct kobject *kobj, struct kobj_attribute *attr,
                              char *buf) {

  return snprintf(buf, PAGE_SIZE, "%ld", fas_opened_sessions_num);
  
  /* // Count sessions also using with refcounts
  
  struct radix_tree_iter iter;            
  void **slot;
  long counter = 0;

  radix_tree_for_each_slot(slot, &fas_files_tree, &iter,0) {

    struct fas_filp_info *finfo = radix_tree_deref_slot(slot);
    if (finfo == NULL) continue;
    
    if (finfo->filp) counter += atomic_long_read(&finfo->filp->f_count);

  }

  return snprintf(buf, PAGE_SIZE, "%ld", counter);*/

}


struct fas_files_h_struct {

    char* key;
    int counter;
    struct hlist_node node;

};

struct fas_htable {

  DECLARE_HASHTABLE(ht, 16);

};

static int fas_key_hash(char *key) {

  size_t keylen = strlen(key);
	unsigned long hashval = 0;
	int i = 0;

	while (hashval < ULONG_MAX && i < keylen) {

		hashval = hashval << 8;
		hashval += key[i];
		i++;

	}

	return hashval % (1 << 16);

}

ssize_t fas_sessions_each_file_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {

  struct fas_htable *table = kzalloc(sizeof(struct fas_htable), GFP_KERNEL);

  hash_init(table->ht);

  struct radix_tree_iter iter;            
  void **slot;

  radix_tree_for_each_slot(slot, &fas_files_tree, &iter, 0) {

    struct fas_filp_info *finfo = radix_tree_deref_slot(slot);
    if (finfo == NULL) continue;
    
    int key = fas_key_hash(finfo->pathname);
    struct fas_files_h_struct* entry;
    int present = 0;
    
    hash_for_each_possible(table->ht, entry, node, key) {

      if (strcmp(entry->key, finfo->pathname)) continue;

      present = 1;
      entry->counter++;
      break;

    }
    
    if (!present) {
    
      entry = kzalloc(sizeof(struct fas_files_h_struct), GFP_KERNEL);
      entry->key = finfo->pathname; //TODO copy for race
      entry->counter = 1;
      hash_add(table->ht, &entry->node, key);
    
    }

  }

  int bkt, i = 0;
  struct fas_files_h_struct* entry;
  
  hash_for_each (table->ht, bkt, entry, node) {
  
    if (entry->key) {
    
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

/*
static int current_init(void)
{ 
  char pathname[PATH_MAX];

  struct task_struct *t = current;
  struct fdtable *fdt = files_fdtable(t->files);

  int i;
  for(i = 0; i < fdt->max_fds; ++i) {

    if (fdt->fd[i] == NULL) continue;

    struct fas_filp_info* finfo =
      radix_tree_lookup(&fas_files_tree, (unsigned long)fdt->fd[i]);

    if (finfo == NULL) continue;

    struct path* files_path = files_table->fd[i]->f_path;
    char* cwd = d_path(&files_path, pathname, PATH_MAX);

  }

  return 0;

} 
*/



