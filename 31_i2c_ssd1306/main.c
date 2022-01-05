#include <linux/delay.h>
#include <linux/i2c.h>
#include <linux/module.h>
#include <linux/slab.h>
#include "ssd1306.h"

/* Private macros */
#define MOD_NAME "alman"
#define DEV_INFO KERN_INFO MOD_NAME ": "

#define I2C_BUS_NUMBER (1)
#define I2C_SLAVE_ADDR (0x3C)
#define I2C_SLAVE_NAME ("OLED_SSD1306")

/* Private types */
struct alm_dev
{
  struct i2c_adapter *adapter;
  struct i2c_client *client;
  struct ssd1306 *oled;
};

static struct alm_dev alm = {0};

/* alm_oled_probe - probe will be called when driver is loaded at first time
 *
 * Return: errno
 */
static int alm_oled_probe(struct i2c_client *client,
                          const struct i2c_device_id *id)
{
  struct ssd1306 *oled = alm.oled;

  ssd1306_init(oled);
  ssd1306_clear(oled);

  ssd1306_set_cursor(oled, 0, 0);
  ssd1306_scroll_h(oled, true, 0, 2);

  ssd1306_print_str(oled, "Hello World\n");
  ssd1306_set_cursor(oled, 7, 0);
  ssd1306_print_str(oled, "alman\n");

  pr_info(DEV_INFO "ssd1306 was probed\n");
  return 0;
}

/* alm_oled_remove - remove will be called when driver is unloaded at last time
 *
 * Return: errno
 */
static int alm_oled_remove(struct i2c_client *client)
{
  struct ssd1306 *oled = alm.oled;

  ssd1306_set_cursor(oled, 0, 0);
  ssd1306_print_str(oled, "Good bye!!!");
  msleep(1000);

  ssd1306_set_cursor(oled, 0, 0);
  ssd1306_clear(oled);

  ssd1306_display_on(oled, false);

  pr_info(DEV_INFO "ssd1306 was removed\n");
  return 0;
}

/* Slave device id structure */
static const struct i2c_device_id alm_oled_id[] = {
    {I2C_SLAVE_NAME, 0},
    {},
};
MODULE_DEVICE_TABLE(i2c, alm_oled_id);

/* I2C driver structure */
static struct i2c_driver alm_oled_driver = {
    .driver = {
        .owner = THIS_MODULE,
        .name = I2C_SLAVE_NAME,
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
  alm.adapter = i2c_get_adapter(I2C_BUS_NUMBER);
  if (alm.adapter == NULL)
  {
    pr_err(DEV_INFO "Can't get adapter bus %d\n", I2C_BUS_NUMBER);
    return -1;
  }

  alm.client = i2c_new_client_device(alm.adapter, &alm_oled_board);
  if (alm.client == NULL)
  {
    pr_err(DEV_INFO "Can't add board info\n");
    goto r_adapter;
  }

  alm.oled = ssd1306_new(alm.client);
  if (alm.oled == NULL)
  {
    pr_err(DEV_INFO "Can't allocate memory for ssd1306\n");
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
  ssd1306_del(alm.oled);
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
