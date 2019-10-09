#include "fas_private.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrea Fioraldi <andreafioraldi@gmail.com>");
MODULE_DESCRIPTION("FAS: A Linux subsistem for file access with sessions");

int            fas_major_num;
struct class * fas_class;
struct device *fas_device;

struct radix_tree_root fas_files_tree;
EXPORT_SYMBOL(fas_files_tree);

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

static int __init fas_init(void) {

  FAS_SAY("Loaded FAS module (v" FAS_VERSION ")");

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

  device_destroy(fas_class, MKDEV(fas_major_num, 0));
  class_unregister(fas_class);
  class_destroy(fas_class);
  unregister_chrdev(fas_major_num, DEVICE_NAME);

  FAS_SAY("All done, exit");
  return;

}

module_init(fas_init);
module_exit(fas_exit);

