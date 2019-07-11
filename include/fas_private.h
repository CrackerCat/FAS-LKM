#ifndef __FAS_PRIVATE_H__
#define __FAS_PRIVATE_H__

#include "fas.h"

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/fs.h> 
#include <linux/uaccess.h>
#include <linux/proc_fs.h>
#include <linux/fs_struct.h>
#include <linux/time.h>
#include <linux/path.h>
#include <linux/namei.h>
#include <linux/ioctl.h>
#include <linux/kern_levels.h>

/* Output macros */

#define FAS_FATAL(x...) \
  do { \
    pr_crit("[FAS] FATAL in %s(), %s:%u\n", __FUNCTION__, \
         __FILE__, __LINE__); \
    pr_crit("  Message: " x); \
    pr_crit("\n"); \
  } while(0)

#define FAS_WARN(x...) \
  pr_warning("[FAS] WARNING: " x)

#define FAS_SAY(x...) \
  pr_info("[FAS] SAY: " x)

/* Definitions */

#define DEVICE_NAME "fas"
#define CLASS_NAME "fas"

/* Cross object variables */

extern int fas_major_num; /* Dinamically allocated device number */
extern struct class *fas_class; /* Class struct for FAS */

long fas_dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg);

#endif
