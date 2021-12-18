#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/proc_fs.h>

/* Private macros */        
#define MOD_NAME "alman"
#define DEV_INFO KERN_INFO MOD_NAME ": "

#define BUF_SIZE 50

/* Function prototypes */
static int 	__init alman_init(void); 
static void 	__exit alman_exit(void);
/* Driver functions */
static int	alman_open(struct inode *inode, struct file *file);
static int	alman_release(struct inode *inode, struct file *file);
static ssize_t	alman_read(struct file *file, char __user *buf, size_t len, loff_t *off);
static ssize_t	alman_write(struct file *file, const char __user *buf, size_t len, loff_t *off);
/* Procfs functions */
static int	proc_open(struct inode *inode, struct file *file);
static int	proc_release(struct inode *inode, struct file *file);
static ssize_t	proc_read(struct file *file, char __user *buf, size_t len, loff_t *off);
static ssize_t	proc_write(struct file *file, const char __user *buf, size_t len, loff_t *off);

/* Private variables */
static dev_t alman_devnum = 0;
static struct class *alman_class;
static struct cdev alman_cdev;

static struct proc_dir_entry *parent;
static char alman_buf[BUF_SIZE] = "Al-Manshurin Informatika\n";

static struct file_operations fops = {
	.owner 		= THIS_MODULE,
	.read		= alman_read,
	.write 		= alman_write,
	.open 		= alman_open,
	.release	= alman_release,
};

static struct proc_ops pops = {
	.proc_open = proc_open,
	.proc_read = proc_read,
	.proc_write = proc_write,
	.proc_release = proc_release,
};

/* Function implementations */
/* Procfs functions */
static int proc_open(struct inode *inode, struct file *file) 
{
	pr_info(DEV_INFO "Procfs open() called\n");
	return 0;
}

static int proc_release(struct inode *inode, struct file *file)
{
	pr_info(DEV_INFO "Procfs release() called\n");
	return 0;
}

static ssize_t proc_read(struct file *filep, char __user *buf, size_t len, loff_t *off)
{
	pr_info(DEV_INFO "Procfs read() called\n");
	if (copy_to_user(buf, alman_buf, BUF_SIZE)) 
	{ 
		pr_err("Data read Error\n");
		return -ENOMEM;
	}

	return len;
}

static ssize_t proc_write(struct file *filep, const char __user *buf, size_t len, loff_t *off)
{
	pr_info(DEV_INFO "Procfs write() called\n");
	if (copy_from_user(alman_buf, buf, len))
	{
		pr_err(DEV_INFO "Data write Error\n");
		return -ENOMEM;
	}
	
	return len;
}

/* Driver functions */
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

	/* Create proc directory */
	parent = proc_mkdir(MOD_NAME, NULL);
	if (parent == NULL)
	{
		pr_info(DEV_INFO "Can't create procfs entry\n");
		goto r_device;
	}

	/* Create procfs file */
	proc_create(MOD_NAME "_proc", 0666, parent, &pops);

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
	remove_proc_entry(MOD_NAME "_proc", parent); 
	proc_remove(parent); 

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
