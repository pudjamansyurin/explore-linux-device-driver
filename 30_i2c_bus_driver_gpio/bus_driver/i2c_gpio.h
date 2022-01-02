#ifndef ___I2C_GPIO_H__
#define ___I2C_GPIO_H__

#include <linux/module.h>

/* i2c_gpio structure */
struct i2c_gpio {
	unsigned int scl;
   	unsigned int sda;	
};

/* exported functions */
int i2c_gpio_init(struct i2c_gpio *p, int scl_pin, int sda_pin);
void i2c_gpio_deinit(struct i2c_gpio *p);
bool i2c_gpio_read_scl(struct i2c_gpio *p);
bool i2c_gpio_read_sda(struct i2c_gpio *p);
void i2c_gpio_write_scl(struct i2c_gpio *p, int state);
void i2c_gpio_write_sda(struct i2c_gpio *p, int state);

#endif /* __I2C_GPIO_H__ */
