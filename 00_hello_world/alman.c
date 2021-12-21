#include <linux/module.h>

static int __init alm_init(void) 
{
	printk(KERN_INFO "Hello World\n");
	return 0;
}

static void __exit alm_exit(void)
{
	printk(KERN_INFO "Goodbye World\n");
}

module_init(alm_init);
module_exit(alm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pudja Mansyurin");
MODULE_DESCRIPTION("Simple hello world driver");
MODULE_VERSION("3:5.4");
