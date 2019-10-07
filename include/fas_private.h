#ifndef __FAS_PRIVATE_H__
#define __FAS_PRIVATE_H__

#include <linux/module.h>
#include <linux/moduleparam.h>
#include <linux/init.h>
#include <linux/string.h>
#include <linux/device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <linux/proc_fs.h>
#include <linux/fs_struct.h>
#include <linux/time.h>
#include <linux/path.h>
#include <linux/namei.h>
#include <linux/kern_levels.h>
#include <linux/types.h>
#include <linux/ioctl.h>
#include <linux/file.h>
#include <asm/segment.h>
#include <linux/buffer_head.h>
#include <linux/kprobes.h>

#include "fas.h"

#define DEVICE_NAME "fas"
#define CLASS_NAME "fas"

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

struct fas_filp_info {

  struct file *filp;
  unsigned char is_w;
};

typedef long (*do_sys_open_t)(int, const char __user *, int, umode_t);

extern do_sys_open_t fas_do_sys_open;

/* Cross object variables */

extern int fas_major_num; /* Dinamically allocated device number */
extern struct class *fas_class; /* Class struct for FAS */

long fas_dev_ioctl(struct file *f, unsigned int cmd, unsigned long arg);

int fas_ioctl_open(char* filename, int flags, mode_t mode);

#endif
