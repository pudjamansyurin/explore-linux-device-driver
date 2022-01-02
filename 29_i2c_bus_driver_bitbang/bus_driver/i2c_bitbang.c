#include <linux/delay.h>
#include "i2c_bitbang.h"

#define I2C_DELAY() usleep_range(5, 10)

/*
** i2c_bbang_start - send START condition (bit banging)
**      ______  
**            \
**  SDA        \_____
**              :
**              :
**           ___:____
**  SCL     /
**      ___/ 
*/
void i2c_bbang_start(struct i2c_gpio *p)
{
	i2c_gpio_write_sda(p, 1);
	i2c_gpio_write_scl(p, 1);
	I2C_DELAY();
	i2c_gpio_write_sda(p, 0);
	I2C_DELAY();
	i2c_gpio_write_scl(p, 0);
	I2C_DELAY();
}

/*
**i2c_bbang_stop - send STOP condition
**                _____
**               /
**  SDA  _______/
**              :
**              :
**            __:______
**  SCL      /
**       ___/ 
*/
void i2c_bbang_stop(struct i2c_gpio *p)
{
	i2c_gpio_write_sda(p, 0);
	I2C_DELAY();
	i2c_gpio_write_scl(p, 1);
	I2C_DELAY();
	i2c_gpio_write_sda(p, 1);
	I2C_DELAY();
	i2c_gpio_write_scl(p, 0);
}

/*
 * i2c_bbang_read_ack - read ACK/NACK status using SDA line
 *
 * Return: ACK=0; NACK=-1
 */
static int i2c_bbang_read_ack(struct i2c_gpio *p)
{
	int ack;

	I2C_DELAY();
	i2c_gpio_write_scl(p, 1);
	I2C_DELAY();
	ack = i2c_gpio_read_sda(p) == 0; /* check ack/nack */
	i2c_gpio_write_scl(p, 0);

	return ack ? 0 : -1;
}

/*
 * i2c_bbang_send_bit - send 1 bit to slave
 */
static void i2c_bbang_send_bit(struct i2c_gpio *p, bool bit)
{
	i2c_gpio_write_sda(p, bit);
	I2C_DELAY();
	i2c_gpio_write_scl(p, 1);
	I2C_DELAY();
	i2c_gpio_write_scl(p, 0);	
}

/*
 * i2c_bbang_send_addr - send 7-bit address to slave
 * @addr: address of the slave
 * @read: read or write operation
 *
 * Return: errno
 */
static int i2c_bbang_send_addr(struct i2c_gpio *p, u8 addr, bool is_read)
{
	int i;
	u8 bit;

	/* write the address 7bit */
	for (i=7; i>=0; --i) {
		bit = (addr >> i) & 0x01; /* send MSB first */
		i2c_bbang_send_bit(p, bit);	
	}

	/* write the r/w (8th bit) */
	i2c_bbang_send_bit(p, is_read);
	I2C_DELAY();

	/* read ack status */
	return i2c_bbang_read_ack(p);
}

/*
 * i2c_bbang_send_byte - send a byte to slave
 *
 * Return: errno
 */
static int i2c_bbang_send_byte(struct i2c_gpio *p, u8 payload)
{
	int i;
	u8 bit;

	/* send payload */
	for (i=8; i>=0; --i) {
		bit = (payload >> i) & 0x01;
		i2c_bbang_send_bit(p, bit);
	}
	
	return i2c_bbang_read_ack(p);
}

/*
 * i2c_bbang_send - send nr bytes to slave
 *
 * Return: errno
 */
int i2c_bbang_send(struct i2c_gpio *p, u8 addr, u8 *buf, u16 len)
{
	if (i2c_bbang_send_addr(p, addr, false) < 0) {
		pr_err("I2C send address failed\n");
		return -1;
	}

	while (len--) {
		if (i2c_bbang_send_byte(p, *buf++) < 0) {
			pr_err("I2C send byte failed\n");
			return -1;
		}
	}

	return 0;
}
