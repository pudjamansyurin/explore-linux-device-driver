#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/slab.h>

/* Private macros */
#define MOD_NAME "alman-client"
#define MOD_INFO MOD_NAME ": "

#define I2C_BUS_NUMBER (7) /* check in /sys/bus/i2c/ directory */
#define I2C_SLAVE_NAME ("OLED_SSD1306")
#define I2C_SLAVE_ADDR (0x3C)

#define OLED_PAGES (8)
#define OLED_SEGMENTS (128)

/* Private types */
enum control_type {
	CTL_CMD = 0x00,
	CTL_DATA = (0x01 << 6),
};

/* Private variables */
static struct i2c_adapter *alm_adapter;
static struct i2c_client *alm_client;

/**
 * i2c_write - low level i2c transmit
 *
 * Return: errno
 */
static int i2c_write(unsigned char *buf, unsigned int len)
{
	int ret = i2c_master_send(alm_client, buf, len);
	return ret;
}

/**
 * i2c_read - low level i2c receive
 *
 * Return: errno
 */
static int i2c_read(unsigned char *buf, unsigned int len)
{
	int ret = i2c_master_recv(alm_client, buf, len);
	return ret;
}

/**
 * ssd1306_write- write data/command to ssd1306
 *
 * Return: errno
 */
static int ssd1306_write(enum control_type type, unsigned char payload)
{
	unsigned char buf[2] = {
		type,
		payload,
	};

	return i2c_write(buf, 2);
}

/**
 * ssd13056_init - initialization sequence for ssd1306
 *
 * Return: none
 */
static void ssd1306_init(void)
{
	ssd1306_write(CTL_CMD, 0xAE); // Entire Display OFF

	ssd1306_write(CTL_CMD, 0xD5); // Set Clock Divide Ratio and Osc Freq
	ssd1306_write(CTL_CMD, 
		0x80); // Default Setting for Clock Divide Ratio and Osc Freq

	ssd1306_write(CTL_CMD, 0xA8); // Set Multiplex Ratio
	ssd1306_write(CTL_CMD, 0x3F); // 64 COM lines

	ssd1306_write(CTL_CMD, 0xD3); // Set display offset
	ssd1306_write(CTL_CMD, 0x00); // 0 offset

	ssd1306_write(CTL_CMD, 
		0x40); // Set first line as the start line of the display

	ssd1306_write(CTL_CMD, 0x8D); // Set Charge pump
	ssd1306_write(CTL_CMD, 0x14); // Enable charge dump during display on

	ssd1306_write(CTL_CMD, 0x20); // Set memory addressing mode
	ssd1306_write(CTL_CMD, 0x00); // Horizontal addressing mode

	ssd1306_write(CTL_CMD, 
		0xA1); // Set segment remap with column address 127 mapped to segment 0
	ssd1306_write(CTL_CMD, 
		0xC8); // Set com output scan direction, scan from com63 to com 0

	ssd1306_write(CTL_CMD, 0xDA); // Set com pins hardware configuration
	ssd1306_write(CTL_CMD, 
		0x12); // Alternative com pin config, disable com left/right remap

	ssd1306_write(CTL_CMD, 0x81); // Set contrast control
	ssd1306_write(CTL_CMD, 0xFF); // Set contrast value

	ssd1306_write(CTL_CMD, 0xD9); // Set pre-charge period
	ssd1306_write(CTL_CMD, 
		0xF1); // Phase 1 period of 15 DCLK, Phase 2 period of 1 DCLK

	ssd1306_write(CTL_CMD, 0xDB); // Set Vcomh deselect level
	ssd1306_write(CTL_CMD, 0x20); // Vcomh deselect level ~ 0.77 Vcc

	ssd1306_write(CTL_CMD, 
		0xA4); // Entire display ON, resume to RAM content display
	ssd1306_write(CTL_CMD, 0xA6); // Set Display in Normal Mode, 1 = ON, 0 = OFF
	ssd1306_write(CTL_CMD, 0x2E); // Deactivate scroll
	ssd1306_write(CTL_CMD, 0xAF); // Display ON in normal mode
}

/**
 * ssd1306_fill - fill display of ssd1306 with specific data
 * @payload: byte data to be written
 *
 * Return: none
 */
static void ssd1306_fill(unsigned char payload)
{
	unsigned int i;

	for (i = 0; i < (OLED_PAGES * OLED_SEGMENTS); i++)
		ssd1306_write(CTL_DATA, payload);
}

/**
 * alm_oled_probe - called when driver is loaded at first time
 */
static int alm_oled_probe(struct i2c_client *client,
			  const struct i2c_device_id *id)
{
	ssd1306_init();
	//ssd1306_fill(0xff);
	pr_info(MOD_INFO "ssd1306 was probed\n");
	return 0;
}

/**
 * alm_oled_remove - called when driver is unloaded at last time
 */
static int alm_oled_remove(struct i2c_client *client)
{
	//ssd1306_fill(0x00);
	pr_info(MOD_INFO "ssd1306 was removed\n");
	return 0;
}

/* I2C device id (slave) structure */
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

/**
 * alm_init - module init func
 */
static int __init alm_init(void)
{
	if ((alm_adapter = i2c_get_adapter(I2C_BUS_NUMBER)) == NULL) {
		pr_err(MOD_INFO "Can't get adapter bus %d\n", I2C_BUS_NUMBER);
		return -1;
	}

	if ((alm_client = i2c_new_client_device(alm_adapter,
						&alm_oled_board)) == NULL) {
		pr_err(MOD_INFO "Can't add board info\n");
		goto r_adapter;
	}

	i2c_add_driver(&alm_oled_driver);

	pr_info(MOD_INFO "Driver added\n");
	return 0;

r_adapter:
	i2c_put_adapter(alm_adapter);

	return -1;
}

/**
 * alm_exit - module exit func
 */
static void __exit alm_exit(void)
{
	i2c_del_driver(&alm_oled_driver);
	i2c_unregister_device(alm_client);
	pr_info(MOD_INFO "Driver removed\n");
}

module_init(alm_init);
module_exit(alm_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pudja Mansyurin");
MODULE_DESCRIPTION(MOD_NAME);
MODULE_VERSION("3:5.4");
