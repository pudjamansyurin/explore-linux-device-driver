#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
// #include <linux/slab.h>

/* Private macros */
#define MOD_NAME "alman1"
#define DEV_INFO KERN_INFO MOD_NAME ": "

/* Exported variables */
int alm_count = 0;

/* Private variables */
struct alm_dev
{
  dev_t devno;
  struct cdev *cdev;
  struct class *class;
};

static struct alm_dev alman = {0};

/* Module prototypes */
static int __init alm_init(void);
static void __exit alm_exit(void);
/* Cdev prototypes */
static int alm_open(struct inode *inode, struct file *filp);
static int alm_release(struct inode *inode, struct file *filp);
static ssize_t alm_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t alm_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);
/* Thread prototypes */
void alm_shared_fn(void);

static struct file_operations fops = {
    .owner = THIS_MODULE,
    .read = alm_read,
    .write = alm_write,
    .open = alm_open,
    .release = alm_release,
};

/* Exported symbols */
// EXPORT_SYMBOL_GPL(alm_shared_fn);
// EXPORT_SYMBOL(alm_count);
EXPORT_SYMBOL(alm_shared_fn);

/* Function implementations */
void alm_shared_fn(void)
{
  pr_info(DEV_INFO "Shared function called %d\n", alm_count++);
}

/*
** This function is called on device file open 
*/
static int alm_open(struct inode *inode, struct file *filp)
{
  struct alm_dev *dev;

  dev = container_of(&inode->i_cdev, struct alm_dev, cdev);
  filp->private_data = dev;

  pr_info(DEV_INFO "Driver open() called\n");
  return 0;
}

/*
** This function is called on device file close
*/
static int alm_release(struct inode *inode, struct file *filp)
{
  pr_info(DEV_INFO "Driver release() called\n");
  return 0;
}

/*
** This function is called on device file read 
*/
static ssize_t alm_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
  pr_info(DEV_INFO "Driver read() called\n");
  return 0;
}

/*
** This function is called on device file write 
*/
static ssize_t alm_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
  // struct alm_dev *dev;
  // uint8_t *buffer;

  // dev = (struct alm_dev *)filp->private_data;

  pr_info(DEV_INFO "Driver write() called\n");

  // if ((buffer = kmalloc(len, GFP_KERNEL)) == NULL)
  // 	return -ENOMEM;

  // if (copy_from_user(buffer, buf, len))
  // 	return -EFAULT;

  /* Do something with kernel buffer */

  return len;
}

/*
** This function is called at the first time module inserted
*/
static int __init alm_init(void)
{
  /* Allocate major number */
  if (alloc_chrdev_region(&alman.devno, 0, 1, MOD_NAME "_dev") < 0)
  {
    pr_err(DEV_INFO "Can't allocate major number for device\n");
    return -1;
  }
  printk(DEV_INFO "Major = %d, Minor = %d\n", MAJOR(alman.devno), MINOR(alman.devno));

  /* Create struct chardev */
  // cdev_init(&alman.cdev, &fops);
  if ((alman.cdev = cdev_alloc()) == NULL)
  {
    pr_err(DEV_INFO "Can't allocate cdev\n");
    goto r_major;
  }
  alman.cdev->owner = THIS_MODULE;
  alman.cdev->ops = &fops;

  /* Add chardev to kernel */
  if (cdev_add(alman.cdev, alman.devno, 1) < 0)
  {
    pr_err(DEV_INFO "Can't add chardev to the system\n");
    goto r_major;
  }

  /* Create struct class */
  if ((alman.class = class_create(THIS_MODULE, MOD_NAME "_class")) == NULL)
  {
    pr_err(DEV_INFO "Can't create struct class for device\n");
    goto r_cdev;
  }

  /* Create the device */
  if (device_create(alman.class, NULL, alman.devno, NULL, MOD_NAME "_device") == NULL)
  {
    pr_err(DEV_INFO "Can't create the device\n");
    goto r_class;
  }

  printk(DEV_INFO "Driver inserted\n");
  return 0;

// r_dev:
//   device_destroy(alman.class, alman.devno);
r_class:
  class_destroy(alman.class);
r_cdev:
  cdev_del(alman.cdev);
r_major:
  unregister_chrdev_region(alman.devno, 1);

  return -1;
}

/*
** This function is called at the last time module removed 
*/
static void __exit alm_exit(void)
{
  device_destroy(alman.class, alman.devno);
  class_destroy(alman.class);
  cdev_del(alman.cdev);
  unregister_chrdev_region(alman.devno, 1);

  printk(DEV_INFO "Driver removed\n");
}

module_init(alm_init);
module_exit(alm_exit);

/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pudja Mansyurin");
MODULE_DESCRIPTION(MOD_NAME);
MODULE_VERSION("3:5.4");
