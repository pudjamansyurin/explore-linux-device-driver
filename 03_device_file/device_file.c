//#include <linux/kernel.h>
//#include <linux/init.h>
#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>
#include <linux/device.h>

//URL: https://embetronicx.com/tutorials/linux/device-drivers/device-file-creation-for-character-drivers/

#define MOD_NAME "device_file"
#define DEV_INFO KERN_INFO MOD_NAME ": "

/* Private types */
struct name 
{
	const char *dev;
	const char *class;
	const char *device;
};

/* Private variables */
static dev_t dev = 0;
static struct class *dev_class;
static struct name alman = {
	.dev = "alman_dev",
	.class = "alman_class",
	.device = "alman_device",
};

/* Functions declaration */
static int __init mod_init(void); 
static void __exit mod_exit(void);

/* Functions implementation */
static int __init mod_init(void) 
{
	/* Allocate major number */
	if ((alloc_chrdev_region(&dev, 0, 1, alman.dev)) < 0) 
	{
		pr_err(DEV_INFO "Can't allocate major number for device\n");
		return -1;
	}
	printk(DEV_INFO "Major = %d, Minor = %d\n", MAJOR(dev), MINOR(dev));
	
	/* Create struct class */
	dev_class = class_create(THIS_MODULE, alman.class);
	if (dev_class == NULL)
	{
		pr_err(DEV_INFO "Can't create struct class for device\n");
		goto r_class;
	}

	/* Create the device */
	if (device_create(dev_class, NULL, dev, NULL, alman.device) == NULL)
	{
		pr_err(DEV_INFO "Can't create the device\n");
		goto r_device;
	}

	printk(DEV_INFO "Driver inserted\n");
	return 0;

r_device:
	class_destroy(dev_class);
r_class:
	unregister_chrdev_region(dev,1);
	return -1;

}

static void __exit mod_exit(void)
{
	device_destroy(dev_class, dev);
	class_destroy(dev_class);
	unregister_chrdev_region(dev, 1);
	printk(DEV_INFO "Driver removed\n");
}

module_init(mod_init);
module_exit(mod_exit);

/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pudja Mansyurin");
MODULE_DESCRIPTION(MOD_NAME);
MODULE_VERSION("3:5.4");
