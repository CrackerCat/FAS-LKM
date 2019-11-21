#include "fas_private.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrea Fioraldi <andreafioraldi@gmail.com>");
MODULE_DESCRIPTION("FAS: A Linux subsistem for file access with sessions");

int            fas_major_num;
struct class * fas_class;
struct device *fas_device;

struct radix_tree_root fas_files_tree;
EXPORT_SYMBOL(fas_files_tree);

DEFINE_RWLOCK(fas_files_tree_lock);
EXPORT_SYMBOL(fas_files_tree_lock);

struct kobject *fas_kobj;

static struct kobj_attribute fas_initial_path_attribute =
    __ATTR(initial_path, S_IRUGO | S_IWUSR, fas_initial_path_show,
           fas_intial_path_store);

static struct kobj_attribute fas_sessions_num_attribute =
    __ATTR(sessions_num, S_IRUGO, fas_sessions_num_show, 0);

static struct kobj_attribute fas_sessions_each_file_attribute =
    __ATTR(sessions_each_file, S_IRUGO, fas_sessions_each_file_show, 0);

static struct kobj_attribute fas_processes_attribute =
    __ATTR(processes, S_IRUGO, fas_processes_show, 0);

static struct attribute *fas_attrs[] = {

    &fas_initial_path_attribute.attr,
    &fas_sessions_num_attribute.attr,
    &fas_sessions_each_file_attribute.attr,
    &fas_processes_attribute.attr,
    NULL,

};

static struct attribute_group fas_attr_group = {

    .attrs = fas_attrs,

};

static int fas_dev_open(struct inode *inodep, struct file *filep) {

  FAS_SAY("Just opened!");
  return 0;

}

static char *fas_devnode(struct device *dev, umode_t *mode) {

  if (mode) *mode = 0644;
  return NULL;

}

static struct file_operations dev_fops = {

    .owner = THIS_MODULE,
    .open = fas_dev_open,
    .unlocked_ioctl = fas_dev_ioctl,

};

long fas_dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg) {

  switch (cmd) {

    case FAS_IOCTL_NOP: break;

    case FAS_IOCTL_OPEN: {

      struct fas_open_args open_args;
      if (copy_from_user(&open_args, (void *)arg, sizeof(struct fas_open_args)))
        return -EINVAL;

      /* Ensure NUL termination */
      open_args.pathname[PATH_MAX - 1] = 0;

      return fas_ioctl_open(open_args.pathname, open_args.flags,
                            open_args.mode);
      break;

    }

  }

  return 0;

}

EXPORT_SYMBOL(fas_dev_ioctl);

static int __init fas_init(void) {

  FAS_SAY("Loaded FAS module (v" FAS_VERSION ")");

  fas_kobj = kobject_create_and_add("fas", kernel_kobj);
  if (!fas_kobj) return -ENOMEM;

  int r = sysfs_create_group(fas_kobj, &fas_attr_group);
  if (r) {

    FAS_FATAL("Failed to install in sysfs");
    kobject_put(fas_kobj);
    return r;

  }

  FAS_SAY("Installed FAS in sysfs");

  INIT_RADIX_TREE(&fas_files_tree, GFP_KERNEL);

  fas_major_num = register_chrdev(0, DEVICE_NAME, &dev_fops);
  if (fas_major_num < 0) {

    FAS_FATAL("Failed to register a major number");
    return fas_major_num;

  }

  fas_class = class_create(THIS_MODULE, CLASS_NAME);
  if (IS_ERR(fas_class)) {

    FAS_FATAL("Failed to register device class");

    unregister_chrdev(fas_major_num, DEVICE_NAME);
    return PTR_ERR(fas_class);

  }

  fas_class->devnode = fas_devnode;

  fas_device = device_create(fas_class, NULL, MKDEV(fas_major_num, 0), NULL,
                             DEVICE_NAME);
  if (IS_ERR(fas_device)) {

    FAS_FATAL("Failed to create the device\n");

    class_destroy(fas_class);
    unregister_chrdev(fas_major_num, DEVICE_NAME);
    return PTR_ERR(fas_device);

  }

  FAS_SAY("The major device number is %d", fas_major_num);

  return 0;

}

static void __exit fas_exit(void) {

  kobject_put(fas_kobj);

  device_destroy(fas_class, MKDEV(fas_major_num, 0));
  class_unregister(fas_class);
  class_destroy(fas_class);
  unregister_chrdev(fas_major_num, DEVICE_NAME);

  FAS_SAY("All done, exit");
  return;

}

module_init(fas_init);
module_exit(fas_exit);

