#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/wait.h>
#include <linux/kthread.h>

/* Private macros */
#define MOD_NAME "alman"
#define DEV_INFO KERN_INFO MOD_NAME ": "
#define USE_DYNAMIC 1

/* Device file: Function prototypes */
static int alm_open(struct inode *inode, struct file *filp);
static int alm_release(struct inode *inode, struct file *filp);
static ssize_t alm_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t alm_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);
/* Driver: Function prototypes */
static int __init alm_init(void);
static void __exit alm_exit(void);

/* Private variables */
static dev_t alm_devnum = 0;
static struct class *alm_class;
static struct cdev alm_cdev;

#if USE_DYNAMIC
static wait_queue_head_t alm_wq;
#else
static DECLARE_WAIT_QUEUE_HEAD(alm_wq);
#endif

static int wq_flag = 0;
static struct task_struct *wait_thread;

static struct file_operations fops = {
		.owner = THIS_MODULE,
		.read = alm_read,
		.write = alm_write,
		.open = alm_open,
		.release = alm_release,
};

/* Function implementations */
/* Thread: Function */
static int wait_func(void *unused)
{
	while (1)
	{
		pr_info(DEV_INFO "Waitting for event\n");
		wait_event_interruptible(alm_wq, wq_flag != 0);

		if (wq_flag == 2)
		{
			pr_info(DEV_INFO "Got event from exit()\n");
			return 0;
		}
		pr_info(DEV_INFO "Got event from read()\n");
		wq_flag = 0;
	}
	return 0;
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

static ssize_t alm_read(struct file *filp, char __user *buf, size_t len, loff_t *off)
{
	pr_info(DEV_INFO "Driver read() called\n");
	wq_flag = 1;
	wake_up_interruptible(&alm_wq);
	return 0;
}

static ssize_t alm_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	pr_info(DEV_INFO "Driver write() called\n");
	return len;
}

/* Driver: Function implementations */
static int __init alm_init(void)
{
	/* Chardev: Allocate major number */
	if (alloc_chrdev_region(&alm_devnum, 0, 1, MOD_NAME "_dev") < 0)
	{
		pr_err(DEV_INFO "Can't allocate major number for device\n");
		return -1;
	}
	printk(DEV_INFO "Major = %d, Minor = %d\n", MAJOR(alm_devnum), MINOR(alm_devnum));

	/* Chardev: Create struct chardev */
	cdev_init(&alm_cdev, &fops);

	/* Chardev: Add chardev to kernel */
	if (cdev_add(&alm_cdev, alm_devnum, 1) < 0)
	{
		pr_err(DEV_INFO "Can't add chardev to the system\n");
		goto r_cdev;
	}

	/* Device file: Create struct class */
	if ((alm_class = class_create(THIS_MODULE, MOD_NAME "_class")) == NULL)
	{
		pr_err(DEV_INFO "Can't create struct class for device\n");
		goto r_class;
	}

	/* Device file: Create the device */
	if (device_create(alm_class, NULL, alm_devnum, NULL, MOD_NAME "_device") == NULL)
	{
		pr_err(DEV_INFO "Can't create the device\n");
		goto r_device;
	}

	/* Wait queue: Initialization */
	init_waitqueue_head(&alm_wq);

	/* Kernel thread: Create */
	if ((wait_thread = kthread_create(wait_func, NULL, "wait_thread")) == NULL)
	{
		pr_info(DEV_INFO "Can't create thread\n");
		goto r_thread;
	}

	/* Kernel thread: Wakeup */
	pr_info(DEV_INFO "Thread created sucessfully\n");
	wake_up_process(wait_thread);

	printk(DEV_INFO "Driver inserted\n");
	return 0;

r_thread:
	device_destroy(alm_class, alm_devnum);
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
	wq_flag = 2;
	wake_up_interruptible(&alm_wq);

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
