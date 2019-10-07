#include "fas_private.h"

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Andrea Fioraldi <andreafioraldi@gmail.com>");
MODULE_DESCRIPTION("FAS: A Linux subsistem for file access with sessions");

int fas_major_num;
struct class *fas_class; 

do_sys_open_t fas_do_sys_open;
EXPORT_SYMBOL(fas_do_sys_open);

static struct file_operations dev_fops =
{
    .owner = THIS_MODULE,
    .unlocked_ioctl = fas_dev_ioctl,
};

static int fas_lookup_needed_symbols(void) {

	struct kprobe kp = {0};
	
	kp.symbol_name = "do_sys_open";
	if (!register_kprobe(&kp)) {

		fas_do_sys_open = (unsigned long)kp.addr;
		unregister_kprobe(&kp);

	} else return -1;

  return 0;

}

static int __init fas_init(void) {

	int r;
	
	r = fas_lookup_needed_symbols();
	if (r < 0) {

    FAS_FATAL("Failed to lookup needed symbols");
    return r;
	}

  fas_major_num = register_chrdev(0, DEVICE_NAME, &dev_fops);
  if (fas_major_num < 0) {
  
      FAS_FATAL("Failed to register a major number");
      return fas_major_num;
  }

  /* TODO Code for the sysfs device
  
  fas_class = class_create(THIS_MODULE, CLASS_NAME);
  if (IS_ERR(fas_class)) {
  
      FAS_FATAL("Failed to register device class");
      
      unregister_chrdev(fas_major_num, DEVICE_NAME);
      return PTR_ERR(fas_class);
  }*/

  FAS_SAY("The major device number is %d", fas_major_num);

  return 0;
}

module_init(fas_init);

