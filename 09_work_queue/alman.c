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

/* Device file: Function prototypes */
static int	alman_open(struct inode *inode, struct file *file);
static int	alman_release(struct inode *inode, struct file *file);
static ssize_t	alman_read(struct file *file, char __user *buf, size_t len, loff_t *off);
static ssize_t	alman_write(struct file *file, const char *buf, size_t len, loff_t *off);
/* Driver: Function prototypes */
static int 	__init alman_init(void); 
static void 	__exit alman_exit(void);

/* Private variables */
static dev_t alman_dev = 0;
static struct class *alman_class;
static struct cdev alman_cdev;

static int wq_flag = 0;
static wait_queue_head_t alman_wq;
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
	while(1) 
	{
		pr_info(DEV_INFO "Waitting for event\n");
		wait_event_interruptible(alman_wq, wq_flag != 0);

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
	wq_flag = 1;
	wake_up_interruptible(&alman_wq);
	return 0;
}

static ssize_t alman_write(struct file *filep, const char *buf, size_t len, loff_t *off)
{
	pr_info(DEV_INFO "Driver write() called\n");
	return len;
}

/* Driver: Function implementations */
static int __init alman_init(void) 
{
	/* Chardev: Allocate major number */
	if (alloc_chrdev_region(&alman_dev, 0, 1, MOD_NAME "_dev") < 0) 
	{
		pr_err(DEV_INFO "Can't allocate major number for device\n");
		return -1;
	}
	printk(DEV_INFO "Major = %d, Minor = %d\n", MAJOR(alman_dev), MINOR(alman_dev));
	
	/* Chardev: Create struct chardev */
	cdev_init(&alman_cdev, &fops);

	/* Chardev: Add chardev to kernel */
	if (cdev_add(&alman_cdev, alman_dev, 1) < 0)
	{
		pr_err(DEV_INFO "Can't add chardev to the system\n");
		goto r_cdev;
	}

	/* Device file: Create struct class */
	alman_class = class_create(THIS_MODULE, MOD_NAME "_class");
	if (alman_class == NULL)
	{
		pr_err(DEV_INFO "Can't create struct class for device\n");
		goto r_class;
	}

	/* Device file: Create the device */
	if (device_create(alman_class, NULL, alman_dev, NULL, MOD_NAME "_device") == NULL)
	{
		pr_err(DEV_INFO "Can't create the device\n");
		goto r_device;
	}

	/* Wait queue: Initialization */
	init_waitqueue_head(&alman_wq);

	/* Kernel thread: Create */
	wait_thread = kthread_create(wait_func, NULL, "wait_thread");
	if (wait_thread == NULL) 
	{
		pr_info(DEV_INFO "Can't create thread\n");
		goto r_device;
	}
	
	/* Kernel thread: Wakeup */
	pr_info(DEV_INFO "Thread created sucessfully\n");
	wake_up_process(wait_thread);

	printk(DEV_INFO "Driver inserted\n");
	return 0;

r_device:
	class_destroy(alman_class);
r_class:
	cdev_del(&alman_cdev);
r_cdev:
	unregister_chrdev_region(alman_dev, 1);
	return -1;
}

static void __exit alman_exit(void)
{
	wq_flag = 2;
	wake_up_interruptible(&alman_wq);

	device_destroy(alman_class, alman_dev);
	class_destroy(alman_class);
	cdev_del(&alman_cdev);
	unregister_chrdev_region(alman_dev, 1);
	printk(DEV_INFO "Driver removed\n");
}

module_init(alman_init);
module_exit(alman_exit);

/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pudja Mansyurin");
MODULE_DESCRIPTION(MOD_NAME);
MODULE_VERSION("3:5.4");
