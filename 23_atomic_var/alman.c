#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
// #include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/delay.h>
#include <linux/seqlock.h>

/* Private macros */
#define MOD_NAME "alman"
#define DEV_INFO KERN_INFO MOD_NAME ": "
#define MAX_THREAD (2)

/* Private types */
struct alm_dev
{
	unsigned long shared_var;
	seqlock_t seqlock;

	dev_t devno;
	struct cdev *cdev;
	struct class *class;
};

/* Private variables */
static struct alm_dev alm = {0};
static struct task_struct *alm_thread1;
static struct task_struct *alm_thread2;

/* Module prototypes */
static int __init alm_init(void);
static void __exit alm_exit(void);
/* Cdev prototypes */
static int alm_open(struct inode *inode, struct file *filp);
static int alm_release(struct inode *inode, struct file *filp);
static ssize_t alm_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t alm_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);
/* Thread prototypes */
static int thread1_fn(void *pv);
static int thread2_fn(void *pv);

static struct file_operations fops = {
		.owner = THIS_MODULE,
		.read = alm_read,
		.write = alm_write,
		.open = alm_open,
		.release = alm_release,
};

/* Function implementations */
static int thread1_fn(void *pv)
{
	while (!kthread_should_stop())
	{
		write_seqlock(&alm.seqlock);
		alm.shared_var++;
		pr_info(DEV_INFO "Thread1: write value = %lu\n", alm.shared_var);
		write_sequnlock(&alm.seqlock);
		msleep(1000);
	}
	return 0;
}

static int thread2_fn(void *pv)
{
	unsigned int seqno;
	unsigned long value;

	while (!kthread_should_stop())
	{
		do
		{
			seqno = read_seqbegin(&alm.seqlock);
			value = alm.shared_var;
		} while (read_seqretry(&alm.seqlock, seqno));
		pr_info(DEV_INFO "Thread2: read value = %lu\n", value);
		msleep(1000);
	}
	return 0;
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
	if (alloc_chrdev_region(&alm.devno, 0, 1, MOD_NAME "_dev") < 0)
	{
		pr_err(DEV_INFO "Can't allocate major number for device\n");
		return -1;
	}
	printk(DEV_INFO "Major = %d, Minor = %d\n", MAJOR(alm.devno), MINOR(alm.devno));

	/* Create struct chardev */
	// cdev_init(alm.cdev, &fops);
	if ((alm.cdev = cdev_alloc()) == NULL)
	{
		pr_err(DEV_INFO "Can't allocate cdev\n");
		goto r_major;
	}
	alm.cdev->owner = THIS_MODULE;
	alm.cdev->ops = &fops;

	/* Add chardev to kernel */
	if (cdev_add(alm.cdev, alm.devno, 1) < 0)
	{
		pr_err(DEV_INFO "Can't add chardev to the system\n");
		goto r_major;
	}

	/* Create struct class */
	if ((alm.class = class_create(THIS_MODULE, MOD_NAME "_class")) == NULL)
	{
		pr_err(DEV_INFO "Can't create struct class for device\n");
		goto r_cdev;
	}

	/* Create the device */
	if (device_create(alm.class, NULL, alm.devno, NULL, MOD_NAME "_device") == NULL)
	{
		pr_err(DEV_INFO "Can't create the device\n");
		goto r_class;
	}

	/* Kernel thread: Create & Wakeup */
	if ((alm_thread1 = kthread_run(thread1_fn, NULL, "thread1")) == NULL)
	{
		pr_info(DEV_INFO "Can't create thread1\n");
		goto r_dev;
	}

	if ((alm_thread2 = kthread_run(thread2_fn, NULL, "thread2")) == NULL)
	{
		pr_info(DEV_INFO "Can't create thread2\n");
		goto r_dev;
	}

	/* Seqlock: Initiate */
	seqlock_init(&alm.seqlock);

	printk(DEV_INFO "Driver inserted\n");
	return 0;

r_dev:
	device_destroy(alm.class, alm.devno);
r_class:
	class_destroy(alm.class);
r_cdev:
	cdev_del(alm.cdev);
r_major:
	unregister_chrdev_region(alm.devno, 1);

	return -1;
}

/*
** This function is called at the last time module removed 
*/
static void __exit alm_exit(void)
{
	kthread_stop(alm_thread1);
	kthread_stop(alm_thread2);

	device_destroy(alm.class, alm.devno);
	class_destroy(alm.class);
	cdev_del(alm.cdev);
	unregister_chrdev_region(alm.devno, 1);

	printk(DEV_INFO "Driver removed\n");
}

module_init(alm_init);
module_exit(alm_exit);

/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pudja Mansyurin");
MODULE_DESCRIPTION(MOD_NAME);
MODULE_VERSION("3:5.4");
