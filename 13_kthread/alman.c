#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/delay.h>

/* Private macros */        
#define MOD_NAME "alman"
#define DEV_INFO KERN_INFO MOD_NAME ": "

/* Device file: Function prototypes */
static int	alman_open(struct inode *inode, struct file *file);
static int	alman_release(struct inode *inode, struct file *file);
static ssize_t	alman_read(struct file *file, char __user *buf, size_t len, loff_t *off);
static ssize_t	alman_write(struct file *file, const char __user *buf, size_t len, loff_t *off);
/* Driver: Function prototypes */
static int 	__init alman_init(void); 
static void 	__exit alman_exit(void);

/* Private variables */
static dev_t alman_devnum = 0;
static struct class *alman_class;
static struct cdev alman_cdev;

static struct task_struct *wait_thread;

static struct file_operations fops = {
	.owner 		= THIS_MODULE,
	.read		= alman_read,
	.write 		= alman_write,
	.open 		= alman_open,
	.release	= alman_release,
};

/* Function implementations */
/* Thread: Function */
static int wait_func(void *unused)
{
	do {
		pr_info(DEV_INFO "Thread is running\n");
		msleep(1000);
	} while(!kthread_should_stop()); 

	return 0;
}

/* Device file: Function implementations */
static int alman_open(struct inode *inode, struct file *file) 
{
	pr_info(DEV_INFO "Driver open() called\n");
	return 0;
}

static int alman_release(struct inode *inode, struct file *file)
{
	pr_info(DEV_INFO "Driver release() called\n");
	return 0;
}

static ssize_t alman_read(struct file *filep, char __user *buf, size_t len, loff_t *off)
{
	pr_info(DEV_INFO "Driver read() called\n");
	return 0;
}

static ssize_t alman_write(struct file *filep, const char __user *buf, size_t len, loff_t *off)
{
	pr_info(DEV_INFO "Driver write() called\n");
	return len;
}

/* Driver: Function implementations */
static int __init alman_init(void) 
{
	/* Device Number: Allocate major number */
	if (alloc_chrdev_region(&alman_devnum, 0, 1, MOD_NAME "_dev") < 0) 
	{
		pr_err(DEV_INFO "Can't allocate major number for device\n");
		return -1;
	}
	printk(DEV_INFO "Major = %d, Minor = %d\n", MAJOR(alman_devnum), MINOR(alman_devnum));
	
	/* Chardev: Create struct chardev */
	cdev_init(&alman_cdev, &fops);

	/* Chardev: Add chardev to kernel */
	if (cdev_add(&alman_cdev, alman_devnum, 1) < 0)
	{
		pr_err(DEV_INFO "Can't add chardev to the system\n");
		goto r_cdev;
	}

	/* Device file: Create struct class */
	if ((alman_class = class_create(THIS_MODULE, MOD_NAME "_class")) == NULL)
	{
		pr_err(DEV_INFO "Can't create struct class for device\n");
		goto r_class;
	}

	/* Device file: Create the device */
	if (device_create(alman_class, NULL, alman_devnum, NULL, MOD_NAME "_device") == NULL)
	{
		pr_err(DEV_INFO "Can't create the device\n");
		goto r_device;
	}

	/* Kernel thread: Create & Wakeup */
	if ((wait_thread = kthread_run(wait_func, NULL, "wait_thread")) == NULL)
	{
		pr_info(DEV_INFO "Can't create thread\n");
		goto r_thread;
	}
	
	printk(DEV_INFO "Driver inserted\n");
	return 0;

r_thread:
	device_destroy(alman_class, alman_devnum);
r_device:
	class_destroy(alman_class);
r_class:
	cdev_del(&alman_cdev);
r_cdev:
	unregister_chrdev_region(alman_devnum, 1);

	return -1;
}

static void __exit alman_exit(void)
{
	kthread_stop(wait_thread);

	device_destroy(alman_class, alman_devnum);
	class_destroy(alman_class);
	cdev_del(&alman_cdev);
	unregister_chrdev_region(alman_devnum, 1);
	printk(DEV_INFO "Driver removed\n");
}

module_init(alman_init);
module_exit(alman_exit);

/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pudja Mansyurin");
MODULE_DESCRIPTION(MOD_NAME);
MODULE_VERSION("3:5.4");
