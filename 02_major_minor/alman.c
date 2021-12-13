#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>

//URL: https://embetronicx.com/tutorials/linux/device-drivers/character-device-driver-major-number-and-minor-number/

#define MOD_NAME "major_minor"
#define DEV_INFO KERN_INFO MOD_NAME ": "

/* Private variables */
static dev_t dev = 0;

/* Functions declaration */
static int __init mod_init(void); 
static void __exit mod_exit(void);

/* Functions implementation */
static int __init mod_init(void) 
{
	int res;

	res = alloc_chrdev_region(&dev, 0, 1, "almanshurin_dev");
	if (res < 0) 
	{
		printk (DEV_INFO "Can't allocate major number for device 1\n");
		return -1;
	}
	printk(DEV_INFO "Major = %d, Minor = %d\n", MAJOR(dev), MINOR(dev));
	printk(DEV_INFO "Driver inserted\n");
	return 0;
}

static void __exit mod_exit(void)
{
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
