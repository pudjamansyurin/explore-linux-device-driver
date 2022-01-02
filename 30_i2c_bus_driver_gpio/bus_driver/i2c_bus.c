#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/i2c-algo-bit.h>
#include "i2c_gpio.h"

/* Private macros */
#define MOD_NAME "alman-bus"
#define MOD_INFO MOD_NAME ": "

#define ADAPTER_NAME "ALM_I2C_ADAPTER"
#define GPIO_SCL (4)
#define GPIO_SDA (17)

/* Private variables */
static struct i2c_gpio alm_i2c_gpio;

/* 
 * alm_i2c_get_scl - read scl pin state 
 */
static int alm_i2c_get_scl(void *data)
{
	return i2c_gpio_read_scl(&alm_i2c_gpio);
}

/*
 * alm_i2c_get_sda - read sda pin state
 */
static int alm_i2c_get_sda(void *data)
{
	return i2c_gpio_read_sda(&alm_i2c_gpio);
}

/*
 * alm_i2c_set_scl - write state to scl pin
 */
static void alm_i2c_set_scl(void *data, int state)
{
	i2c_gpio_write_scl(&alm_i2c_gpio, state);
}

/*
 * alm_i2c_set_sda - write state to sda pin
 */
static void alm_i2c_set_sda(void *data, int state)
{
	i2c_gpio_write_sda(&alm_i2c_gpio, state);
}

/* I2C algorithm structure */
static struct i2c_algo_bit_data alm_i2c_algorithm = {
	.setsda = alm_i2c_set_sda,
	.setscl = alm_i2c_set_scl,
	.getsda = alm_i2c_get_sda,
	.getscl = alm_i2c_get_scl,
	.udelay = 5,
	.timeout = 100, /* ms */
};

/* I2C adapter structure */
static struct i2c_adapter alm_i2c_adapter = {
	.owner = THIS_MODULE,
	.class = I2C_CLASS_HWMON | I2C_CLASS_SPD,
	.name = ADAPTER_NAME,
	.algo_data = &alm_i2c_algorithm,
	.nr = 7,
};

/* Module init callback */
static int __init alm_init(void)
{
	if (i2c_gpio_init(&alm_i2c_gpio, GPIO_SCL, GPIO_SDA) < 0) {
		pr_err(MOD_INFO "Can't initialize gpio\n");
		return -1;
	}

	if (i2c_bit_add_numbered_bus(&alm_i2c_adapter) < 0) {
		pr_err(MOD_INFO "Can't add I2C adapter\n");
		return -1;
	}

	pr_info(MOD_INFO "Driver added\n");
	return 0;
}

/* Module exit callback */
static void __exit alm_exit(void)
{
	i2c_gpio_deinit(&alm_i2c_gpio);
	i2c_del_adapter(&alm_i2c_adapter);
	pr_info(MOD_INFO "Driver removed\n");
}

module_init(alm_init);
module_exit(alm_exit);

/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pudja Mansyurin");
MODULE_DESCRIPTION(MOD_NAME);
MODULE_VERSION("3:5.4");
