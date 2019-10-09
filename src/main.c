#include "fas_private.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrea Fioraldi <andreafioraldi@gmail.com>");
MODULE_DESCRIPTION("FAS: A Linux subsistem for file access with sessions");

int fas_major_num;
struct class *fas_class; 
struct device *fas_device;

do_sys_open_t fas_do_sys_open;
EXPORT_SYMBOL(fas_do_sys_open);

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

static int fas_lookup_needed_symbols(void) {

	struct kprobe kp = {0};
	
	kp.symbol_name = "do_sys_open";
	if (register_kprobe(&kp) < 0) {

		fas_do_sys_open = (void*)kp.addr;
		unregister_kprobe(&kp);

	} else return -1;

  return 0;

}

static int __init fas_init(void) {

	int r;
	
	FAS_SAY("Loaded FAS module (v" FAS_VERSION ")");
	
	INIT_RADIX_TREE(&fas_files_tree, GFP_KERNEL);
	
	r = fas_lookup_needed_symbols();
	
	FAS_SAY("Symbols lookup completed");
	
	if (r < 0) {

    FAS_FATAL("Failed to lookup needed symbols");
    return r;
	}

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
  
  fas_device = device_create(fas_class, NULL, MKDEV(fas_major_num, 0), NULL, DEVICE_NAME);
  if (IS_ERR(fas_device)) {

    FAS_FATAL("Failed to create the device\n");
    
    class_destroy(fas_class);
    unregister_chrdev(fas_major_num, DEVICE_NAME);
    return PTR_ERR(fas_device);
  }

  FAS_SAY("The major device number is %d", fas_major_num);

  return 0;
}


static void __exit fas_exit(void)
{
    device_destroy(fas_class, MKDEV(fas_major_num, 0));
    class_unregister(fas_class);
    class_destroy(fas_class);
    unregister_chrdev(fas_major_num, DEVICE_NAME);

    FAS_SAY("All done, exit");
    return;
}

module_init(fas_init);
module_exit(fas_exit);

