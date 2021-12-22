#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/ioctl.h>
#include <linux/slab.h>

/* Private macros */
#define MOD_NAME "alman"
#define DEV_INFO KERN_INFO MOD_NAME ": "

#define REG_CURRENT_TASK _IOW('a', 'a', int32_t *)
#define SIGETX 44

/* Private variables */
struct alm_dev
{
	struct task_struct *task;
	int signum;

	dev_t dev_num;
	struct cdev dev_cdev;
	struct class *dev_class;
};

static struct alm_dev alman = {
		.task = NULL,
		.signum = 0,
		.dev_num = 0,
};

/* Function prototypes */
static int __init alm_init(void);
static void __exit alm_exit(void);
/* Fops prototypes */
static int alm_open(struct inode *inode, struct file *filp);
static int alm_release(struct inode *inode, struct file *filp);
static ssize_t alm_read(struct file *filp, char __user *buf, size_t len, loff_t *off);
static ssize_t alm_write(struct file *filp, const char __user *buf, size_t len, loff_t *off);
static long alm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg);

static struct file_operations fops = {
		.owner = THIS_MODULE,
		.read = alm_read,
		.write = alm_write,
		.open = alm_open,
		.release = alm_release,
		.unlocked_ioctl = alm_ioctl,
};

/* Function implementations */
static long alm_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
	struct alm_dev *dev;

	dev = (struct alm_dev *)filp->private_data;
	switch (cmd)
	{
	case REG_CURRENT_TASK:
		pr_info("IOCTL: REG_CURRENT_TASK\n");
		dev->task = get_current();
		dev->signum = SIGETX;
		break;

	default:
		break;
	}
	return 0;
}

static int alm_open(struct inode *inode, struct file *filp)
{
	struct alm_dev *dev;

	dev = container_of(inode->i_cdev, struct alm_dev, dev_cdev);
	filp->private_data = dev;

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
	return 0;
}

static ssize_t alm_write(struct file *filp, const char __user *buf, size_t len, loff_t *off)
{
	struct alm_dev *dev;
	struct kernel_siginfo info;
	uint8_t *buffer;

	dev = (struct alm_dev *)filp->private_data;

	pr_info(DEV_INFO "Driver write() called\n");

	if ((buffer = kmalloc(len, GFP_KERNEL)) == NULL)
		return -ENOMEM;

	if (copy_from_user(buffer, buf, len))
		return -ENOMEM;

	sscanf(buffer, "%d", &info.si_int);
	info.si_signo = dev->signum;
	info.si_code = SI_QUEUE;

	if (dev->task != NULL)
	{
		pr_info(DEV_INFO "Send signal to app\n");
		if (send_sig_info(dev->signum, &info, dev->task) < 0)
			pr_err(DEV_INFO "Send signal failed\n");
	}

	return len;
}

static int __init alm_init(void)
{
	/* Allocate major number */
	if (alloc_chrdev_region(&alman.dev_num, 0, 1, MOD_NAME "_dev") < 0)
	{
		pr_err(DEV_INFO "Can't allocate major number for device\n");
		return -1;
	}
	printk(DEV_INFO "Major = %d, Minor = %d\n", MAJOR(alman.dev_num), MINOR(alman.dev_num));

	/* Create struct chardev */
	cdev_init(&alman.dev_cdev, &fops);

	/* Add chardev to kernel */
	if (cdev_add(&alman.dev_cdev, alman.dev_num, 1) < 0)
	{
		pr_err(DEV_INFO "Can't add chardev to the system\n");
		return -1;
	}

	/* Create struct class */
	if ((alman.dev_class = class_create(THIS_MODULE, MOD_NAME "_class")) == NULL)
	{
		pr_err(DEV_INFO "Can't create struct class for device\n");
		goto r_class;
	}

	/* Create the device */
	if (device_create(alman.dev_class, NULL, alman.dev_num, NULL, MOD_NAME "_device") == NULL)
	{
		pr_err(DEV_INFO "Can't create the device\n");
		goto r_device;
	}

	printk(DEV_INFO "Driver inserted\n");
	return 0;

r_device:
	class_destroy(alman.dev_class);
r_class:
	unregister_chrdev_region(alman.dev_num, 1);

	return -1;
}

static void __exit alm_exit(void)
{
	device_destroy(alman.dev_class, alman.dev_num);
	class_destroy(alman.dev_class);
	cdev_del(&alman.dev_cdev);
	unregister_chrdev_region(alman.dev_num, 1);
	printk(DEV_INFO "Driver removed\n");
}

module_init(alm_init);
module_exit(alm_exit);

/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pudja Mansyurin");
MODULE_DESCRIPTION(MOD_NAME);
MODULE_VERSION("3:5.4");
