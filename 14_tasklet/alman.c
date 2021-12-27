#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/slab.h>

/* Private macros */
#define MOD_NAME "alman"
#define DEV_INFO KERN_INFO MOD_NAME ": "

#define USE_DYNAMIC 0

/* Tasklet: Function prototypes */
static void tasklet_fn(unsigned long arg);
/* Device file: Function prototypes */
static int alm_open(struct inode *inode, struct file *filp);
static int alm_release(struct inode *inode, struct file *filp);
static ssize_t alm_read(struct file *filp, char __user *buf, size_t len,
			loff_t *off);
static ssize_t alm_write(struct file *filp, const char __user *buf, size_t len,
			 loff_t *off);
/* Driver: Function prototypes */
static int __init alm_init(void);
static void __exit alm_exit(void);

/* Private variables */
static dev_t alm_devnum = 0;
static struct class *alm_class;
static struct cdev alm_cdev;

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = alm_read,
	.write = alm_write,
	.open = alm_open,
	.release = alm_release,
};

#if USE_DYNAMIC
static struct tasklet_struct alm_tasklet;
#else
static DECLARE_TASKLET_OLD(alm_tasklet, tasklet_fn);
#endif

/* Function implementations */
/* Tasklet: Function */
static void tasklet_fn(unsigned long arg)
{
	pr_info(DEV_INFO "Tasklet is executing in atomic context\n");
	pr_info(DEV_INFO "Tasklet arg = %ld\n", arg);
}

/* Device file: Function implementations */
static int alm_open(struct inode *inode, struct file *filp)
{
	pr_info(DEV_INFO "Driver open() called\n");
	return 0;
}

static int alm_release(struct inode *inode, struct file *filp)
{
	pr_info(DEV_INFO "Driver release() called\n");
	return 0;
}

static ssize_t alm_read(struct file *filp, char __user *buf, size_t len,
			loff_t *off)
{
	pr_info(DEV_INFO "Driver read() called\n");
	tasklet_schedule(&alm_tasklet);
	pr_info(DEV_INFO "Driver read() exited\n");
	return 0;
}

static ssize_t alm_write(struct file *filp, const char __user *buf, size_t len,
			 loff_t *off)
{
	pr_info(DEV_INFO "Driver write() called\n");
	return len;
}

/* Driver: Function implementations */
static int __init alm_init(void)
{
	/* Chardev: Allocate major number */
	if (alloc_chrdev_region(&alm_devnum, 0, 1, MOD_NAME "_dev") < 0) {
		pr_err(DEV_INFO "Can't allocate major number for device\n");
		return -1;
	}
	printk(DEV_INFO "Major = %d, Minor = %d\n", MAJOR(alm_devnum),
	       MINOR(alm_devnum));

	/* Chardev: Create struct chardev */
	cdev_init(&alm_cdev, &fops);

	/* Chardev: Add chardev to kernel */
	if (cdev_add(&alm_cdev, alm_devnum, 1) < 0) {
		pr_err(DEV_INFO "Can't add chardev to the system\n");
		goto r_cdev;
	}

	/* Device file: Create struct class */
	if ((alm_class = class_create(THIS_MODULE, MOD_NAME "_class")) ==
	    NULL) {
		pr_err(DEV_INFO "Can't create struct class for device\n");
		goto r_class;
	}

	/* Device file: Create the device */
	if (device_create(alm_class, NULL, alm_devnum, NULL,
			  MOD_NAME "_device") == NULL) {
		pr_err(DEV_INFO "Can't create the device\n");
		goto r_device;
	}

	/* Tasklet (atomic context) */
#if USE_DYNAMIC
	tasklet_init(&alm_tasklet, tasklet_fn, 313);
#endif

	printk(DEV_INFO "Driver inserted\n");
	return 0;

r_device:
	class_destroy(alm_class);
r_class:
	cdev_del(&alm_cdev);
r_cdev:
	unregister_chrdev_region(alm_devnum, 1);

	return -1;
}

static void __exit alm_exit(void)
{
#if USE_DYNAMIC
	tasklet_kill(&alm_tasklet);
#endif

	device_destroy(alm_class, alm_devnum);
	class_destroy(alm_class);
	cdev_del(&alm_cdev);
	unregister_chrdev_region(alm_devnum, 1);
	printk(DEV_INFO "Driver removed\n");
}

module_init(alm_init);
module_exit(alm_exit);

/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pudja Mansyurin");
MODULE_DESCRIPTION(MOD_NAME);
MODULE_VERSION("3:5.4");
