#ifndef __I2C_BITBANG_H__
#define __I2C_BITBANG_H__

#include <linux/module.h>
#include "i2c_gpio.h"

/* exported functions */
void i2c_bbang_start(struct i2c_gpio *p);
void i2c_bbang_stop(struct i2c_gpio *p);
int i2c_bbang_send(struct i2c_gpio *p, u8 addr, u8 *buf, u16 len);

#endif /* __I2C_BITBANG_H__ */

