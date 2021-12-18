#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>

#define MOD_NAME "alman"
#define DEV_INFO KERN_INFO MOD_NAME ": "

/* Private variables */
static dev_t devnum = 0;
static struct class *dev_class;

/* Functions declaration */
static int 	__init mod_init(void); 
static void 	__exit mod_exit(void);

/* Functions implementation */
static int __init mod_init(void) 
{
	/* Allocate major number */
	if ((alloc_chrdev_region(&devnum, 0, 1, MOD_NAME)) < 0) 
	{
		pr_err(DEV_INFO "Can't allocate major number for device\n");
		return -1;
	}
	printk(DEV_INFO "Major = %d, Minor = %d\n", MAJOR(devnum), MINOR(devnum));
	
	/* Create struct class */
	if((dev_class = class_create(THIS_MODULE, MOD_NAME "_class")) == NULL)
	{
		pr_err(DEV_INFO "Can't create struct class for device\n");
		goto r_class;
	}

	/* Create the device */
	if (device_create(dev_class, NULL, devnum, NULL, MOD_NAME "_device") == NULL)
	{
		pr_err(DEV_INFO "Can't create the device\n");
		goto r_device;
	}

	printk(DEV_INFO "Driver inserted\n");
	return 0;

r_device:
	class_destroy(dev_class);
r_class:
	unregister_chrdev_region(devnum,1);
	return -1;

}

static void __exit mod_exit(void)
{
	device_destroy(dev_class, devnum);
	class_destroy(dev_class);
	unregister_chrdev_region(devnum, 1);
	printk(DEV_INFO "Driver removed\n");
}

module_init(mod_init);
module_exit(mod_exit);

/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pudja Mansyurin");
MODULE_DESCRIPTION(MOD_NAME);
MODULE_VERSION("3:5.4");
