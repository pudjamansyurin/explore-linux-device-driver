#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/cdev.h>

/* Private macros */
#define MOD_NAME "alman"
#define DEV_INFO KERN_INFO MOD_NAME ": "
#define USE_DYNAMIC 1

/* Private variables */
static dev_t devnum = 0;
static struct class *alm_class;
#if USE_DYNAMIC
static struct cdev *dev_cdev = NULL;
#else
static struct cdev alm_cdev;
static struct cdev *dev_cdev = &alm_cdev;
#endif

/* Function prototypes */
static int __init alm_init(void);
static void __exit alm_exit(void);
static int alm_open(struct inode *inode, struct file *filp);
static int alm_release(struct inode *inode, struct file *filp);
static ssize_t alm_read(struct file *filp, char __user *buf, size_t len,
			loff_t *off);
static ssize_t alm_write(struct file *filp, const char __user *buf, size_t len,
			 loff_t *off);

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = alm_read,
	.write = alm_write,
	.open = alm_open,
	.release = alm_release,
};

/* Function implementations */
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
	return 0;
}

static ssize_t alm_write(struct file *filp, const char __user *buf, size_t len,
			 loff_t *off)
{
	pr_info(DEV_INFO "Driver write() called\n");
	return len;
}

static int __init alm_init(void)
{
	/* Allocate major number */
	if (alloc_chrdev_region(&devnum, 0, 1, MOD_NAME "_dev") < 0) {
		pr_err(DEV_INFO "Can't allocate major number for device\n");
		return -1;
	}
	printk(DEV_INFO "Major = %d, Minor = %d\n", MAJOR(devnum),
	       MINOR(devnum));

	/* Create struct chardev */
#if USE_DYNAMIC
	if ((dev_cdev = cdev_alloc()) == NULL) {
		pr_err(DEV_INFO "Can't allocate cdev\n");
		return -ENOMEM;
	}
	dev_cdev->ops = &fops;
#else
	cdev_init(dev_cdev, &fops);
#endif

	/* Add chardev to kernel */
	if (cdev_add(dev_cdev, devnum, 1) < 0) {
		pr_err(DEV_INFO "Can't add chardev to the system\n");
		return -1;
	}

	/* Create struct class */
	if ((alm_class = class_create(THIS_MODULE, MOD_NAME "_class")) ==
	    NULL) {
		pr_err(DEV_INFO "Can't create struct class for device\n");
		goto r_class;
	}

	/* Create the device */
	if (device_create(alm_class, NULL, devnum, NULL, MOD_NAME "_device") ==
	    NULL) {
		pr_err(DEV_INFO "Can't create the device\n");
		goto r_device;
	}

	printk(DEV_INFO "Driver inserted\n");
	return 0;

r_device:
	class_destroy(alm_class);
r_class:
	unregister_chrdev_region(devnum, 1);

	return -1;
}

static void __exit alm_exit(void)
{
	device_destroy(alm_class, devnum);
	class_destroy(alm_class);
	cdev_del(dev_cdev);
	unregister_chrdev_region(devnum, 1);
	printk(DEV_INFO "Driver removed\n");
}

module_init(alm_init);
module_exit(alm_exit);

/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pudja Mansyurin");
MODULE_DESCRIPTION(MOD_NAME);
MODULE_VERSION("3:5.4");
