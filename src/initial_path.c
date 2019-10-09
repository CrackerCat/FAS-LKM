#include "fas_private.h"

char fas_initial_path[PATH_MAX];

ssize_t fas_initial_path_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf) {
  
  size_t size = strlen(fas_initial_path);
	strncpy(buf, fas_initial_path, size);
	return size;

}

ssize_t fas_intial_path_store(struct  kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count) {

	size_t size = count;
	if (count > PATH_MAX)
	  size = PATH_MAX;

	strncpy(fas_initial_path, buf, size);
	return size;

}
