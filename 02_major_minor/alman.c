#include <linux/module.h>
#include <linux/kdev_t.h>
#include <linux/fs.h>

#define MOD_NAME "major_minor"
#define DEV_INFO KERN_INFO MOD_NAME ": "

/* Private variables */
static dev_t devnum = 0;

/* Functions declaration */
static int __init alm_init(void); 
static void __exit alm_exit(void);

/* Functions implementation */
static int __init alm_init(void) 
{
	if(alloc_chrdev_region(&devnum, 0, 1, "almanshurin_dev") < 0)
	{
		printk (DEV_INFO "Can't allocate major number for device 1\n");
		return -1;
	}
	printk(DEV_INFO "Major = %d, Minor = %d\n", MAJOR(devnum), MINOR(devnum));
	printk(DEV_INFO "Driver inserted\n");
	return 0;
}

static void __exit alm_exit(void)
{
	unregister_chrdev_region(devnum, 1);
	printk(DEV_INFO "Driver removed\n");
}

module_init(alm_init);
module_exit(alm_exit);

/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pudja Mansyurin");
MODULE_DESCRIPTION(MOD_NAME);
MODULE_VERSION("3:5.4");
