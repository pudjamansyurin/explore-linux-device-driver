#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
// #include <linux/slab.h>
#include <linux/kthread.h>
#include <linux/delay.h>

/* Private macros */
#define MOD_NAME "alman"
#define DEV_INFO KERN_INFO MOD_NAME ": "
#define MAX_THREAD (2)

/* Private types */
struct alm_dev
{
	atomic_t shared_var;
	unsigned int bit_var;

	dev_t devno;
	struct cdev *cdev;
	struct class *class;
};

struct alm_thread
{
	struct task_struct *pthread;
	char name[10];
};

/* Private variables */
static struct alm_dev alm = {0};
static struct alm_thread threads[MAX_THREAD] = {0};

/* Module prototypes */
static int __init alm_init(void);
static void __exit alm_exit(void);
/* Cdev prototypes */
static int alm_open(struct inode *inode, struct file *filp);
static int alm_release(struct inode *inode, struct file *filp);
static ssize_t alm_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t alm_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);
/* Thread prototypes */
static int thread_fn(void *pv);

static struct file_operations fops = {
		.owner = THIS_MODULE,
		.read = alm_read,
		.write = alm_write,
		.open = alm_open,
		.release = alm_release,
};

/* Function implementations */
static int thread_fn(void *pv)
{
	unsigned int bit_val;
	unsigned int shared_val;
	char *name = (char *)pv;

	while (!kthread_should_stop())
	{
		atomic_inc(&alm.shared_var);
		shared_val = atomic_read(&alm.shared_var);
		bit_val = test_and_change_bit(1, (void *)&alm.bit_var);
		pr_info("%s, value = %u, bit = %u\n", name, shared_val, bit_val);
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
	unsigned int thread_idx;
	struct alm_thread *thread;

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
	for (thread_idx = 0; thread_idx < MAX_THREAD; thread_idx++)
	{
		thread = &threads[thread_idx];

		sprintf(thread->name, "thread-%d", thread_idx + 1);
		if ((thread->pthread = kthread_run(thread_fn, thread->name, thread->name)) == NULL)
		{
			pr_info(DEV_INFO "Can't create %s\n", thread->name);
			goto r_dev;
		}
	}

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
	unsigned int thread_idx;

	for (thread_idx = 0; thread_idx < MAX_THREAD; thread_idx++)
	{
		kthread_stop(threads[thread_idx].pthread);
	}

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
