#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/ioctl.h>

/* Private macros */        
#define MOD_NAME "alman"
#define DEV_INFO KERN_INFO MOD_NAME ": "

#define WR_VALUE _IOW('a', 'a', int32_t*)
#define RD_VALUE _IOR('a', 'b', int32_t*)

/* Private variables */
static dev_t alm_devnum = 0;
static struct class *alm_class;
static struct cdev alm_cdev;

static int32_t alm_value;

/* Function prototypes */
static int 	__init alm_init(void); 
static void 	__exit alm_exit(void);
static int	alm_open(struct inode *inode, struct file *file);
static int	alm_release(struct inode *inode, struct file *file);
static ssize_t	alm_read(struct file *file, char __user *buf, size_t len, loff_t *off);
static ssize_t	alm_write(struct file *file, const char __user *buf, size_t len, loff_t *off);
static long	alm_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations fops = {
	.owner 		= THIS_MODULE,
	.read		= alm_read,
	.write 		= alm_write,
	.open 		= alm_open,
	.release	= alm_release,
	.unlocked_ioctl	= alm_ioctl,
};

/* Function implementations */
static int alm_open(struct inode *inode, struct file *file) 
{
	pr_info(DEV_INFO "Driver open() called\n");
	return 0;
}

static int alm_release(struct inode *inode, struct file *file)
{
	pr_info(DEV_INFO "Driver release() called\n");
	return 0;
}

static ssize_t alm_read(struct file *filep, char __user *buf, size_t len, loff_t *off)
{
	pr_info(DEV_INFO "Driver read() called\n");
	return 0;
}

static ssize_t alm_write(struct file *filep, const char __user *buf, size_t len, loff_t *off)
{
	pr_info(DEV_INFO "Driver write() called\n");
	return len;
}

static long alm_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd) 
	{
		case WR_VALUE:
			if (copy_from_user(&alm_value, (int32_t*) arg, sizeof(alm_value)))
				pr_err("Data write Error\n");
			pr_info("Value = %d\n", alm_value);
			break;

		case RD_VALUE:
			if (copy_to_user((int32_t*) arg, &alm_value, sizeof(alm_value))) 
				pr_err("Data read Error\n");
			break;

		default:
			break;
	}
	return 0;
}

static int __init alm_init(void) 
{
	/* Allocate major number */
	if (alloc_chrdev_region(&alm_devnum, 0, 1, MOD_NAME "_dev") < 0) 
	{
		pr_err(DEV_INFO "Can't allocate major number for device\n");
		return -1;
	}
	printk(DEV_INFO "Major = %d, Minor = %d\n", MAJOR(alm_devnum), MINOR(alm_devnum));
	
	/* Create struct chardev */
	cdev_init(&alm_cdev, &fops);

	/* Add chardev to kernel */
	if (cdev_add(&alm_cdev, alm_devnum, 1) < 0)
	{
		pr_err(DEV_INFO "Can't add chardev to the system\n");
		return -1;
	}

	/* Create struct class */
	if ((alm_class = class_create(THIS_MODULE, MOD_NAME "_class")) == NULL)
	{
		pr_err(DEV_INFO "Can't create struct class for device\n");
		goto r_class;
	}

	/* Create the device */
	if (device_create(alm_class, NULL, alm_devnum, NULL, MOD_NAME "_device") == NULL)
	{
		pr_err(DEV_INFO "Can't create the device\n");
		goto r_device;
	}

	printk(DEV_INFO "Driver inserted\n");
	return 0;

r_device:
	class_destroy(alm_class);
r_class:
	unregister_chrdev_region(alm_devnum, 1);

	return -1;

}

static void __exit alm_exit(void)
{
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
