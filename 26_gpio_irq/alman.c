#include <linux/module.h>
#include <linux/miscdevice.h>
#include <linux/fs.h>
#include <linux/slab.h>
#include <linux/gpio.h>
#include <linux/delay.h>
#include <linux/interrupt.h>

/* Private macros */
#define MOD_NAME "alman"
#define DEV_INFO KERN_INFO MOD_NAME ": "

#define USE_SW_DEBOUNCE (1)
#define DEBOUNCE_MS (250)
#define GPIO_LED (23)
#define GPIO_BTN (24)

#if (USE_SW_DEBOUNCE)
#include <linux/jiffies.h>
static unsigned long last_jiffies = 0;
#endif

/* Module prototypes */
static int __init alm_init(void);
static void __exit alm_exit(void);
/* Cdev prototypes */
static int alm_open(struct inode *inode, struct file *filp);
static int alm_release(struct inode *inode, struct file *filp);
static ssize_t alm_read(struct file *filp, char __user *buf, size_t len,
			loff_t *off);
static ssize_t alm_write(struct file *filp, const char __user *buf, size_t len,
			 loff_t *off);

static struct file_operations fops = {
	.owner = THIS_MODULE,
	.read = alm_read,
	.write = alm_write,
	.open = alm_open,
	.release = alm_release,
};

static struct miscdevice alm_miscdev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = MOD_NAME "_dev",
	.fops = &fops,
};

/* Function implementations */
/* 
 * GPIO interrupt handler 
 */
static irqreturn_t gpio_button_handler(int irq, void *dev_id)
{
	unsigned long flags;

#if USE_SW_DEBOUNCE
	if ((jiffies - last_jiffies) < msecs_to_jiffies(DEBOUNCE_MS)) {
		return IRQ_HANDLED;
	}
#endif

	local_irq_save(flags);
	gpio_set_value(GPIO_LED, gpio_get_value(GPIO_BTN));
	pr_info(DEV_INFO "Button interrupt handler called\n");
	local_irq_restore(flags);

	return IRQ_HANDLED;
}

/*
** This function is called on device file open 
*/
static int alm_open(struct inode *inode, struct file *filp)
{
	pr_info(DEV_INFO "Driver open() called\n");
	return 0;
}

/*
** This function is called on device file close
*/
static int alm_release(struct inode *inode, struct file *filp)
{
	pr_info(DEV_INFO "Driver release() called\n");
	return 0;
}

/*
** This function is called on device file read 
*/
static ssize_t alm_read(struct file *filp, char __user *buf, size_t len,
			loff_t *off)
{
	bool led_state;

	pr_info(DEV_INFO "Driver read() called\n");

	led_state = gpio_get_value(GPIO_LED);
	if (copy_to_user(buf, &led_state, 1)) {
		pr_err(DEV_INFO "Failed copy to user\n");
		return -EFAULT;
	}

	pr_info(DEV_INFO "Read GPIO_LED = %d\n", led_state);
	return 0;
}

/*
** This function is called on device file write 
*/
static ssize_t alm_write(struct file *filp, const char __user *buf, size_t len,
			 loff_t *off)
{
	int err = 0;
	uint8_t *buffer;
	bool led_state;

	pr_info(DEV_INFO "Driver write() called\n");

	if ((buffer = kmalloc(len, GFP_KERNEL)) == NULL) {
		return -ENOMEM;
	}

	if (copy_from_user(buffer, buf, len)) {
		err = -EFAULT;
		goto r_kmalloc;
	}

	/* Do something with kernel buffer */
	if (kstrtobool(buffer, &led_state) < 0) {
		pr_err(DEV_INFO "Invalid command state\n");
		err = -EFAULT;
		goto r_kmalloc;
	}

	gpio_set_value(GPIO_LED, led_state);

	kfree(buffer);
	return len;

r_kmalloc:
	kfree(buffer);
	return err;
}

/*
** This function is called at the first time module inserted
*/
static int __init alm_init(void)
{
	int err;

	if ((err = misc_register(&alm_miscdev))) {
		pr_err(DEV_INFO "Register failed: %d\n", err);
		return err;
	}

	/* GPIO: check is valid */
	if (gpio_is_valid(GPIO_LED) == false) {
		pr_err(DEV_INFO "GPIO %d is invalid\n", GPIO_LED);
		goto r_misc;
	}

	/* GPIO: request to kernel */
	if (gpio_request(GPIO_LED, "GPIO_LED") < 0) {
		pr_err(DEV_INFO "GPIO %d failed request\n", GPIO_LED);
		goto r_gpio_led;
	}

	if (gpio_request(GPIO_BTN, "GPIO_BTN") < 0) {
		pr_err(DEV_INFO "GPIO %d failed request\n", GPIO_BTN);
		goto r_gpio_btn;
	}

	/* GPIO: set as output/input */
	gpio_direction_output(GPIO_LED, 0);
	gpio_direction_input(GPIO_BTN);

	/* GPIO: export to sysfs */
	gpio_export(GPIO_LED, false);

	/* GPIO: set input debounce */
#if USE_SW_DEBOUNCE

#else
	if (gpio_set_debounce(GPIO_BTN, 200) < 0) {
		pr_err(DEV_INFO "GPIO %d debounce failed\n", GPIO_BTN);
		goto r_gpio_btn;
	}
#endif

	/* IRQ: setup for GPIO_BTN */
	if (request_irq(gpio_to_irq(GPIO_BTN), gpio_button_handler,
			IRQF_TRIGGER_RISING, MOD_NAME "_dev", NULL)) {
		pr_err(DEV_INFO "Can't regiest IRQ for gpio\n");
		goto r_gpio_btn;
	}

	printk(DEV_INFO "Driver inserted\n");
	return 0;

r_gpio_btn:
	gpio_free(GPIO_BTN);
r_gpio_led:
	gpio_free(GPIO_LED);
r_misc:
	misc_deregister(&alm_miscdev);
	return -1;
}

/*
** This function is called at the last time module removed 
*/
static void __exit alm_exit(void)
{
	free_irq(gpio_to_irq(GPIO_BTN), NULL);
	gpio_free(GPIO_BTN);
	gpio_unexport(GPIO_LED);
	gpio_free(GPIO_LED);

	misc_deregister(&alm_miscdev);
	printk(DEV_INFO "Driver removed\n");
}

module_init(alm_init);
module_exit(alm_exit);

/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pudja Mansyurin");
MODULE_DESCRIPTION(MOD_NAME);
MODULE_VERSION("3:5.4");
