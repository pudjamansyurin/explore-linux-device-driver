#include <linux/module.h>

#define MOD_NAME "alm_param: "

/* Functions declaration */
static int __init alm_init(void);
static void __exit alm_exit(void);
static int notify_param(const char *value, const struct kernel_param *kp);

/* Private variables */
static int value_arg, array_arg[5];
static int cb_value_arg;
static char *name_arg;

static const struct kernel_param_ops my_param_ops = {
	.set = &notify_param, /* custom setter */
	.get = &param_get_int, /* standard getter */
};

/* Functions initialization*/
static int notify_param(const char *value, const struct kernel_param *kp)
{
	int res;

	res = param_set_int(value, kp); /* helper for write variable */
	if (res == 0) {
		printk(KERN_INFO MOD_NAME "Callback called \n");
		printk(KERN_INFO MOD_NAME "New value of value_arg = %d \n",
		       cb_value_arg);
		return 0;
	}
	return -1;
}

static int __init alm_init(void)
{
	int i;

	printk(KERN_INFO MOD_NAME "Driver inserted\n");
	printk(KERN_INFO MOD_NAME "value_arg = %d\n", value_arg);
	printk(KERN_INFO MOD_NAME "cb_value_arg = %d\n", cb_value_arg);
	printk(KERN_INFO MOD_NAME "name_arg = %s\n", name_arg);
	for (i = 0; i < (sizeof(array_arg) / sizeof(array_arg[0])); i++) {
		printk(KERN_INFO MOD_NAME "array_arg[%d] = %d\n", i,
		       array_arg[i]);
	}
	return 0;
}

static void __exit alm_exit(void)
{
	printk(KERN_INFO MOD_NAME "Driver removed\n");
}

/* Callbacks/Handlers */
module_param(value_arg, int, S_IRUSR | S_IWUSR);
module_param(name_arg, charp, S_IRUSR | S_IWUSR);
module_param_array(array_arg, int, NULL, S_IRUSR | S_IWUSR);
module_param_cb(cb_value_arg, &my_param_ops, &cb_value_arg, S_IRUGO | S_IWUSR);

module_init(alm_init);
module_exit(alm_exit);

/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pudja Mansyurin");
MODULE_DESCRIPTION(MOD_NAME);
MODULE_VERSION("3:5.4");
