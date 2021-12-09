//#include <linux/kernel.h>
//#include <linux/init.h>
#include <linux/module.h>

static int __init hello_world_init(void) 
{
	printk(KERN_INFO "Hello World\n");
	return 0;
}

static void __exit hello_world_exit(void)
{
	printk(KERN_INFO "Goodbye World\n");
}

module_init(hello_world_init);
module_exit(hello_world_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pudja Mansyurin");
MODULE_DESCRIPTION("Simple hello world driver");
MODULE_VERSION("3:5.4");
