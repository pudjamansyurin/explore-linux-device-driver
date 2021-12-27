#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/slab.h>

/* Private macros */
#define MOD_NAME "alman"
#define DEV_INFO KERN_INFO MOD_NAME ": "

#define I2C_BUS_NUMBER (1)
#define I2C_SLAVE_NAME ("OLED_SSD1306")
#define I2C_SLAVE_ADDR (0x3C)

#define OLED_PAGES (8)
#define OLED_SEGMENTS (128)

/* Private types */
struct alm_dev {
	struct i2c_adapter *adapter;
	struct i2c_client *client;
};

static struct alm_dev alm = { 0 };

/* @brief: i2c transmit wrapper
 * @return: error code
 */
static int i2c_write(unsigned char *buf, unsigned int len)
{
	int ret = i2c_master_send(alm.client, buf, len);
	return ret;
}

/* @brief: i2c receive wrapper
 * @return: error code
 */
static int i2c_read(unsigned char *buf, unsigned int len)
{
	int ret = i2c_master_recv(alm.client, buf, len);
	return ret;
}

/* @brief: ssd1306 write data
 * @return: error code
 */
static int ssd1306_write_data(unsigned char payload)
{
	unsigned char buf[2] = {
		0x01 << 6,
		payload,
	};

	return i2c_write(buf, 2);
}

/* @brief: ssd1306 write command
 * @return: error code
 */
static int ssd1306_write_cmd(unsigned char payload)
{
	unsigned char buf[2] = {
		0x00,
		payload,
	};

	return i2c_write(buf, 2);
}

/* @brief: ssd1306 initialization sequence
 * @return: none
 */
static void ssd1306_init(void)
{
	ssd1306_write_cmd(0xAE); // Entire Display OFF

	ssd1306_write_cmd(
		0xD5); // Set Display Clock Divide Ratio and Oscillator Frequency
	ssd1306_write_cmd(
		0x80); // Default Setting for Display Clock Divide Ratio and Oscillator

	ssd1306_write_cmd(0xA8); // Set Multiplex Ratio
	ssd1306_write_cmd(0x3F); // 64 COM lines

	ssd1306_write_cmd(0xD3); // Set display offset
	ssd1306_write_cmd(0x00); // 0 offset

	ssd1306_write_cmd(
		0x40); // Set first line as the start line of the display

	ssd1306_write_cmd(0x8D); // Set Charge pump
	ssd1306_write_cmd(0x14); // Enable charge dump during display on

	ssd1306_write_cmd(0x20); // Set memory addressing mode
	ssd1306_write_cmd(0x00); // Horizontal addressing mode

	ssd1306_write_cmd(
		0xA1); // Set segment remap with column address 127 mapped to segment 0
	ssd1306_write_cmd(
		0xC8); // Set com output scan direction, scan from com63 to com 0

	ssd1306_write_cmd(0xDA); // Set com pins hardware configuration
	ssd1306_write_cmd(
		0x12); // Alternative com pin config, disable com left/right remap

	ssd1306_write_cmd(0x81); // Set contrast control
	ssd1306_write_cmd(0xFF); // Set contrast value

	ssd1306_write_cmd(0xD9); // Set pre-charge period
	ssd1306_write_cmd(
		0xF1); // Phase 1 period of 15 DCLK, Phase 2 period of 1 DCLK

	ssd1306_write_cmd(0xDB); // Set Vcomh deselect level
	ssd1306_write_cmd(0x20); // Vcomh deselect level ~ 0.77 Vcc

	ssd1306_write_cmd(
		0xA4); // Entire display ON, resume to RAM content display
	ssd1306_write_cmd(0xA6); // Set Display in Normal Mode, 1 = ON, 0 = OFF
	ssd1306_write_cmd(0x2E); // Deactivate scroll
	ssd1306_write_cmd(0xAF); // Display ON in normal mode
}

/* @brief: fill all display with data
 * @args : byte data to be written
 * @return: none
 */
static void ssd1306_fill(unsigned char payload)
{
	unsigned int i;

	for (i = 0; i < (OLED_PAGES * OLED_SEGMENTS); i++) {
		ssd1306_write_data(payload);
	}
}

/* @brief: probe will be called when driver is loaded at first time
 */
static int alm_oled_probe(struct i2c_client *client,
			  const struct i2c_device_id *id)
{
	ssd1306_init();
	ssd1306_fill(0xff);
	pr_info(DEV_INFO "ssd1306 was probed\n");
	return 0;
}

/* @brief: remove will be called when driver is unloaded at last time
 */
static int alm_oled_remove(struct i2c_client *client)
{
	ssd1306_fill(0x00);
	pr_info(DEV_INFO "ssd1306 was removed\n");
	return 0;
}

/* Slave device id structure */
static const struct i2c_device_id alm_oled_id[] = {
	{ I2C_SLAVE_NAME, 0 },
	{},
};
MODULE_DEVICE_TABLE(i2c, alm_oled_id);

/* I2C driver structure */
static struct i2c_driver alm_oled_driver = {
    .driver =
        {
            .name = I2C_SLAVE_NAME,
            .owner = THIS_MODULE,
        },
    .probe = alm_oled_probe,
    .remove = alm_oled_remove,
    .id_table = alm_oled_id,
};

/* I2C board info structure */
static struct i2c_board_info alm_oled_board = {
	I2C_BOARD_INFO(I2C_SLAVE_NAME, I2C_SLAVE_ADDR),
};

/* Module init callback */
static int __init alm_init(void)
{
	if ((alm.adapter = i2c_get_adapter(I2C_BUS_NUMBER)) == NULL) {
		pr_err(DEV_INFO "Can't get adapter bus %d\n", I2C_BUS_NUMBER);
		return -1;
	}

	if ((alm.client = i2c_new_client_device(alm.adapter,
						&alm_oled_board)) == NULL) {
		pr_err(DEV_INFO "Can't add board info\n");
		goto r_adapter;
	}

	i2c_add_driver(&alm_oled_driver);

	pr_info(DEV_INFO "Driver added\n");
	return 0;

r_adapter:
	i2c_put_adapter(alm.adapter);

	return -1;
}

/* Module exit callback */
static void __exit alm_exit(void)
{
	i2c_del_driver(&alm_oled_driver);
	i2c_unregister_device(alm.client);
	pr_info(DEV_INFO "Driver removed\n");
}

module_init(alm_init);
module_exit(alm_exit);

/* Module description */
MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pudja Mansyurin");
MODULE_DESCRIPTION(MOD_NAME);
MODULE_VERSION("3:5.4");
