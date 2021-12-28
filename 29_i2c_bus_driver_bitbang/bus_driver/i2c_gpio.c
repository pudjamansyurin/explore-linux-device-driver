#include <linux/module.h>
#include <linux/gpio.h>
#include <linux/slab.h>
#include "i2c_gpio.h"

/**
 * i2c_gpio_alloc - allocate gpio for i2c (bit banging)
 *
 * Return: pointer to i2c_gpio
 */
struct i2c_gpio *i2c_gpio_alloc(void)
{
	struct i2c_gpio *p = NULL;	
	
	if ((p = kmalloc(sizeof(*p), GFP_KERNEL)) == NULL) 
		pr_err("Can't allocate i2c_gpio\n");

	return p;
}

/**
 * i2c_pin_init - initialize i2c pin
 *
 * Return: errno
 */
static int i2c_pin_init(const char *name, int pin)
{
	char gpio_name[10];

	if (gpio_is_valid(pin) == false) {
		pr_err("%s GPIO %d is not valid\n", name, pin);
		return -1;
	}
	
	snprintf(gpio_name, sizeof(gpio_name), "%s_GPIO", name);
	if (gpio_request(pin, gpio_name) < 0) {
		pr_err("%s GPIO %d request failed\n", name, pin);
		return -1;
	}

	gpio_direction_output(pin, 1);

	return 0;
}

/**
 * i2c_gpio_init - initialize gpio for i2c
 *
 * Return: errno
 */
int i2c_gpio_init(struct i2c_gpio *p, int scl_pin, int sda_pin)
{
	int ret = 0;

	do {
		if ((ret = i2c_pin_init("SCL", scl_pin)) < 0)
			break;

		if ((ret = i2c_pin_init("SDA", sda_pin)) < 0) 
			break;
		
		p->scl = scl_pin;
		p->sda = sda_pin;

	} while(false);

	return ret;
}

/**
 * i2c_gpio_deinit - deinitilize gpio for i2c
 */
void i2c_gpio_deinit(struct i2c_gpio *p)
{
	gpio_free(p->scl);
	gpio_free(p->sda);
}

/**
 * i2c_gpio_read - read state of i2c pin
 *
 * Return: pin state
 */
static bool i2c_gpio_read(int pin)
{
	gpio_direction_input(pin);
	return gpio_get_value(pin);
}

/**
 * i2c_gpio_read_scl - read state of i2c scl pin
 *
 * Return: scl pin state
 */
bool i2c_gpio_read_scl(struct i2c_gpio *p)
{
	return i2c_gpio_read(p->scl);
}

/**
 * i2c_gpio_read_sda - read state of i2c sda pin
 *
 * Return: sda pin state
 */
bool i2c_gpio_read_sda(struct i2c_gpio *p)
{
	return i2c_gpio_read(p->sda);
}

/**
 * i2c_gpio_clear - clear state of i2c pin
 * @pin: gpio number
 * @state: value to be set
 */
static void i2c_gpio_set(int pin, int state)
{
	gpio_direction_output(pin, state);
	gpio_set_value(pin, state);
}

/**
 * i2c_gpio_set_scl - set state of i2c scl pin
 * @p: pointer to i2c_gpio
 * @state: value to be set
 */
void i2c_gpio_set_scl(struct i2c_gpio *p, int state)
{
	i2c_gpio_set(p->scl, state);
}

/**
 * i2c_gpio_set_sda - set state of i2c sda pin
 * @p: pointer to i2c_gpio
 * @state: value to be set
 */
void i2c_gpio_set_sda(struct i2c_gpio *p, int state)
{
	i2c_gpio_set(p->sda, state);
}


