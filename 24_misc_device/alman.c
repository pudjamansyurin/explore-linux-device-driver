#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>

/* Private macros */
#define MOD_NAME "alman"
#define DEV_INFO KERN_INFO MOD_NAME ": "

/* Module prototypes */
static int __init alm_init(void);
static void __exit alm_exit(void);
/* Cdev prototypes */
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
	.llseek = no_llseek,
};

static struct miscdevice alm_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = MOD_NAME "_misc",
	.fops = &fops,
};

/* Function implementations */
/*
** This function is called on device file open 
*/
static int alm_open(struct inode *inode, struct file *filp)
{
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
static ssize_t alm_read(struct file *filp, char __user *buf, size_t len,
			loff_t *off)
{
	pr_info(DEV_INFO "Driver read() called\n");
	return 0;
}

/*
** This function is called on device file write 
*/
static ssize_t alm_write(struct file *filp, const char __user *buf, size_t len,
			 loff_t *off)
{
	// uint8_t *buffer;

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
	int err;

	if ((err = misc_register(&alm_miscdev))) {
		pr_err(DEV_INFO "Register failed: %d\n", err);
		return err;
	}

	printk(DEV_INFO "Driver inserted\n");
	return 0;
}

/*
** This function is called at the last time module removed 
*/
static void __exit alm_exit(void)
{
	misc_deregister(&alm_miscdev);
	printk(DEV_INFO "Driver removed\n");
}

module_init(alm_init);
module_exit(alm_exit);

/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pudja Mansyurin");
MODULE_DESCRIPTION(MOD_NAME);
MODULE_VERSION("3:5.4");
