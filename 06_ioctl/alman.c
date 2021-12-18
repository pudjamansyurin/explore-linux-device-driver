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
static dev_t alman_devnum = 0;
static struct class *alman_class;
static struct cdev alman_cdev;

static int32_t alman_value;

/* Function prototypes */
static int 	__init alman_init(void); 
static void 	__exit alman_exit(void);
static int	alman_open(struct inode *inode, struct file *file);
static int	alman_release(struct inode *inode, struct file *file);
static ssize_t	alman_read(struct file *file, char __user *buf, size_t len, loff_t *off);
static ssize_t	alman_write(struct file *file, const char __user *buf, size_t len, loff_t *off);
static long	alman_ioctl(struct file *file, unsigned int cmd, unsigned long arg);

static struct file_operations fops = {
	.owner 		= THIS_MODULE,
	.read		= alman_read,
	.write 		= alman_write,
	.open 		= alman_open,
	.release	= alman_release,
	.unlocked_ioctl	= alman_ioctl,
};

/* Function implementations */
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

static long alman_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
	switch(cmd) 
	{
		case WR_VALUE:
			if (copy_from_user(&alman_value, (int32_t*) arg, sizeof(alman_value)))
				pr_err("Data write Error\n");
			pr_info("Value = %d\n", alman_value);
			break;

		case RD_VALUE:
			if (copy_to_user((int32_t*) arg, &alman_value, sizeof(alman_value))) 
				pr_err("Data read Error\n");
			break;

		default:
			break;
	}
	return 0;
}

static int __init alman_init(void) 
{
	/* Allocate major number */
	if (alloc_chrdev_region(&alman_devnum, 0, 1, MOD_NAME "_dev") < 0) 
	{
		pr_err(DEV_INFO "Can't allocate major number for device\n");
		return -1;
	}
	printk(DEV_INFO "Major = %d, Minor = %d\n", MAJOR(alman_devnum), MINOR(alman_devnum));
	
	/* Create struct chardev */
	cdev_init(&alman_cdev, &fops);

	/* Add chardev to kernel */
	if (cdev_add(&alman_cdev, alman_devnum, 1) < 0)
	{
		pr_err(DEV_INFO "Can't add chardev to the system\n");
		return -1;
	}

	/* Create struct class */
	if ((alman_class = class_create(THIS_MODULE, MOD_NAME "_class")) == NULL)
	{
		pr_err(DEV_INFO "Can't create struct class for device\n");
		goto r_class;
	}

	/* Create the device */
	if (device_create(alman_class, NULL, alman_devnum, NULL, MOD_NAME "_device") == NULL)
	{
		pr_err(DEV_INFO "Can't create the device\n");
		goto r_device;
	}

	printk(DEV_INFO "Driver inserted\n");
	return 0;

r_device:
	class_destroy(alman_class);
r_class:
	unregister_chrdev_region(alman_devnum, 1);

	return -1;

}

static void __exit alman_exit(void)
{
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
