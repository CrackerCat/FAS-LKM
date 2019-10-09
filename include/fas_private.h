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
#include <linux/slab.h>
#include <linux/file.h>
#include <asm/segment.h>
#include <linux/buffer_head.h>
#include <linux/kprobes.h>
#include <linux/radix-tree.h>

#include "fas.h"

#define DEVICE_NAME "fas"
#define CLASS_NAME "fas"

/* Output macros */

#define FAS_HEXDUMP(type, prefix, ptr, size)             \
  do {                                                   \
                                                         \
    int i;                                               \
    printk(type prefix "  [0] ");                        \
    for (i = 0; i < (size); ++i) {                       \
                                                         \
      printk(KERN_CONT "%02hhX ", ((char *)(ptr))[i]);   \
      if ((i + 1) % 16 == 0 && i < (size)-1)             \
        printk(KERN_CONT "\n" prefix "  [%4d] ", i + 1); \
                                                         \
    }                                                    \
    printk(KERN_CONT "\n");                              \
                                                         \
  } while (0)

#define FAS_FATAL(x...)                                                        \
  do {                                                                         \
                                                                               \
    pr_crit("[FAS] FATAL in %s(), %s:%u\n", __FUNCTION__, __FILE__, __LINE__); \
    pr_crit("  Message: " x);                                                  \
    pr_crit("\n");                                                             \
                                                                               \
  } while (0)

#define FAS_WARN(x...) pr_warning("[FAS] WARNING: " x)

#define FAS_SAY(x...) pr_info("[FAS] SAY: " x)

#ifdef DEBUG

#define FAS_DEBUG(x...) pr_alert("[FAS] DEBUG: " x)
#define FAS_DEBUG_HEXDUMP(ptr, size) \
  FAS_HEXDUMP(KERN_ALERT, "[FAS] DEBUG: ", ptr, size)

#else

#define FAS_DEBUG(x...) \
  do {                  \
                        \
  } while (0)
#define FAS_DEBUG_HEXDUMP(x...) \
  do {                          \
                                \
  } while (0)

#endif

/* Definitions */

struct fas_filp_info {

  struct file *           filp;
  struct file_operations *orig_f_op;
  unsigned char           is_w;

};

extern struct radix_tree_root fas_files_tree;

/* Cross object variables */

extern int           fas_major_num;  /* Dinamically allocated device number */
extern struct class *fas_class;                     /* Class struct for FAS */

int fas_filp_copy(struct file *src, struct file *dst);

long fas_dev_ioctl(struct file *filep, unsigned int cmd, unsigned long arg);

int fas_ioctl_open(char *filename, int flags, mode_t mode);
int fas_file_release(struct inode *inodep, struct file *filep);
int fas_file_flush(struct file *filep, fl_owner_t id);

#endif

