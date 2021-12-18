#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/workqueue.h>
#include <linux/delay.h>
#include <linux/slab.h>

/* Private macros */        
#define MOD_NAME "alman"
#define DEV_INFO KERN_INFO MOD_NAME ": "

/* Type prototypes */
struct alman_list {
	struct list_head list;	
	int data;
};

/* Workqueue: Function prototypes */
static void 	workqueue_fn(struct work_struct *work);
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

static int alman_value;
static LIST_HEAD(alman_node);

static struct work_struct alman_work;
static struct workqueue_struct *alman_workqueue;

static struct file_operations fops = {
	.owner 		= THIS_MODULE,
	.read		= alman_read,
	.write 		= alman_write,
	.open 		= alman_open,
	.release	= alman_release,
};

/* Function implementations */
/* Workqueue: Function implementations */
static void workqueue_fn(struct work_struct *work)
{
	struct alman_list *tmp = NULL;

	pr_info(DEV_INFO "Workqueue is called\n");
	if ((tmp = kmalloc(sizeof(struct alman_list), GFP_KERNEL)) == NULL)
	{
		pr_err(DEV_INFO "Can't allocate memory\n");
		return;
	}

	tmp->data = alman_value;
	INIT_LIST_HEAD(&tmp->list);
	list_add_tail(&tmp->list, &alman_node);
	
	pr_info(DEV_INFO "Workqueue done\n");
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
	struct alman_list *tmp;
	int count = 0;

	pr_info(DEV_INFO "Driver read() called\n");
	list_for_each_entry(tmp, &alman_node, list) { 
		pr_info(DEV_INFO "Node %d, Data %d\n", count++, tmp->data);	
	}
	pr_info(DEV_INFO "Driver read() exit\n");
	return 0;
}

static ssize_t alman_write(struct file *filep, const char __user *buf, size_t len, loff_t *off)
{
  char *tmp_buf;

	pr_info(DEV_INFO "Driver write() called\n");

  if ((tmp_buf = kmalloc(len, GFP_KERNEL)) == NULL)
  {
    pr_err(DEV_INFO "Can't allocate memory\n");
    return -ENOMEM;
  }

	if (copy_from_user(tmp_buf, buf, len)) 
  {
		pr_err(DEV_INFO "Can't copy from user\n");
		return -ENOMEM;
	}	

	sscanf(tmp_buf, "%d", &alman_value);
	pr_info(DEV_INFO "Got value = %d\n", alman_value);
	queue_work(alman_workqueue, &alman_work);
	return len;
}

/* Driver: Function implementations */
static int __init alman_init(void) 
{
	/* Chardev: Allocate major number */
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

	/* Work queue: Initialization */
	INIT_WORK(&alman_work, workqueue_fn);
	alman_workqueue = create_workqueue(MOD_NAME "_workqueue");

	printk(DEV_INFO "Driver inserted\n");
	return 0;

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
	struct alman_list *cursor, *tmp;
	list_for_each_entry_safe(cursor, tmp, &alman_node, list)
	{
		list_del(&cursor->list);
		kfree(cursor);
	}
	destroy_workqueue(alman_workqueue);

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
