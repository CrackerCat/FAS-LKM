#include <linux/module.h>	
#include <linux/kernel.h>
#include <linux/init.h>

static int __init fas_init(void) {

	printk(KERN_INFO "Hello, world\n");
	return 0;

}

module_init(fas_init);

MODULE_LICENSE("GPL");

MODULE_AUTHOR("Andrea Fioraldi <andreafioraldi@gmail.com>");
MODULE_DESCRIPTION("FAS: A Linux subsistem for file access with sessions");
