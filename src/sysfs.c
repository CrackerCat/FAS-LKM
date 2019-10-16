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

  size_t r = snprintf(buf, PAGE_SIZE, "%ld", fas_opened_sessions_num);
  return r;

}
