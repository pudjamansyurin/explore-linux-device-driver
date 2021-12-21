#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>
#include <linux/kthread.h>
#include <linux/delay.h>

/* Private macros */        
#define MOD_NAME "alman"
#define DEV_INFO KERN_INFO MOD_NAME ": "

#define SPIN_DYNAMIC 1

/* Threads: Function prototypes */
static int thread_fn1(void *pv);
static int thread_fn2(void *pv);
/* Device file: Function prototypes */
static int	alm_open(struct inode *inode, struct file *file);
static int	alm_release(struct inode *inode, struct file *file);
static ssize_t	alm_read(struct file *file, char __user *buf, size_t len, loff_t *off);
static ssize_t	alm_write(struct file *file, const char __user *buf, size_t len, loff_t *off);
/* Driver: Function prototypes */
static int 	__init alm_init(void); 
static void 	__exit alm_exit(void);

/* Private variables */
static dev_t alm_devnum = 0;
static struct class *alm_class;
static struct cdev alm_cdev;

static struct file_operations fops = {
	.owner 		= THIS_MODULE,
	.read		  = alm_read,
	.write 		= alm_write,
	.open 		= alm_open,
	.release	= alm_release,
};

#if SPIN_DYNAMIC
static rwlock_t alm_rwlock;
#else
static DEFINE_RWLOCK(alm_rwlock);
#endif
static unsigned long shared_var = 0;
static struct task_struct *alm_thread1;
static struct task_struct *alm_thread2; 

/* Function implementations */
/* Thread: Function */
int thread_fn1(void *pv)
{
    while(!kthread_should_stop()) {  
        write_lock(&alm_rwlock);
        shared_var++;
        pr_info("Producer: Write value %lu\n", shared_var);
        write_unlock(&alm_rwlock);
        msleep(1000);
    }
    return 0;
}

int thread_fn2(void *pv)
{
    while(!kthread_should_stop()) {
        read_lock(&alm_rwlock);
        pr_info("Consumer: Read value %lu\n", shared_var);
        read_unlock(&alm_rwlock);
        msleep(500);
    }
    return 0;
}

/* Device file: Function implementations */
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

/* Driver: Function implementations */
static int __init alm_init(void) 
{
	/* Device Number: Allocate major number */
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

  /* Spinlock */
#if SPIN_DYNAMIC
  rwlock_init(&alm_rwlock);
#endif

  /* Creating Thread 1 */
  alm_thread1 = kthread_run(thread_fn1, NULL, "Thread1");
  if(alm_thread1) {
      pr_err("Kthread1 Created Successfully...\n");
  } else {
      pr_err("Cannot create kthread1\n");
        goto r_thread;
  }

    /* Creating Thread 2 */
  alm_thread2 = kthread_run(thread_fn2, NULL, "Thread2");
  if(alm_thread2) {
      pr_err("Kthread2 Created Successfully...\n");
  } else {
      pr_err("Cannot create kthread2\n");
        goto r_thread;
  }
	
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
  kthread_stop(alm_thread1);
  kthread_stop(alm_thread2);

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
