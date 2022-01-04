#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include "ssd1306.h"

/* i2c_write - i2c transmit wrapper 
 * 
 * Return: errno
 */
static int i2c_write(struct i2c_client *client, u8 *buf, unsigned int len)
{
  int ret;

  ret = i2c_master_send(client, buf, len);
  return ret;
}

// /* i2c_read - i2c receive wrapper
//  *
//  * Return: errno
//  */
// static int i2c_read(struct i2c_client *client, u8 *buf, unsigned int len)
// {
//   int ret;

//   ret = i2c_master_recv(client, buf, len);
//   return ret;
// }

/* write_data - ssd1306 write data
 *
 * Return: errno
 */
static int write_data(struct i2c_client *client, u8 payload)
{
  u8 buf[2] = {
      0x01 << 6,
      payload,
  };

  return i2c_write(client, buf, 2);
}

/* write_cmd - ssd1306 write command
 * 
 * Return: errno
 */
static int write_cmd(struct i2c_client *client, u8 payload)
{
  u8 buf[2] = {
      0x00,
      payload,
  };

  return i2c_write(client, buf, 2);
}

/* ssd1306_new - allocate and initiate ssd1306 
 */
struct ssd1306 *ssd1306_new(struct i2c_client *client, u8 npage, u8 nsegment)
{
  struct ssd1306 *oled;

  oled = kmalloc(sizeof(*oled), GFP_KERNEL);
  if (oled != NULL)
  {
    oled->client = client;
    oled->npage = npage;
    oled->nsegment = nsegment;
  }

  return oled;
}

/* ssd1306_del - delete allocated memory for ssd1306 
 */
void ssd1306_del(struct ssd1306 *oled)
{
  kfree(oled);
}

/* ssd1306_init - ssd1306 initialize power on sequence
 */
void ssd1306_init(struct ssd1306 *oled)
{
  struct i2c_client *client = oled->client;

  write_cmd(client, 0xAE); // Entire Display OFF

  write_cmd(client, 0xD5); // Set Display Clock Divide Ratio and Oscillator Frequency
  write_cmd(client, 0x80); // Default Setting for Display Clock Divide Ratio and Oscillator

  write_cmd(client, 0xA8); // Set Multiplex Ratio
  write_cmd(client, 0x3F); // 64 COM lines

  write_cmd(client, 0xD3); // Set display offset
  write_cmd(client, 0x00); // 0 offset

  write_cmd(client, 0x40); // Set first line as the start line of the display

  write_cmd(client, 0x8D); // Set Charge pump
  write_cmd(client, 0x14); // Enable charge dump during display on

  write_cmd(client, 0x20); // Set memory addressing mode
  write_cmd(client, 0x00); // Horizontal addressing mode

  write_cmd(client, 0xA1); // Set segment remap with column address 127 mapped to segment 0
  write_cmd(client, 0xC8); // Set com output scan direction, scan from com63 to com 0

  write_cmd(client, 0xDA); // Set com pins hardware configuration
  write_cmd(client, 0x12); // Alternative com pin config, disable com left/right remap

  write_cmd(client, 0x81); // Set contrast control
  write_cmd(client, 0xFF); // Set contrast value

  write_cmd(client, 0xD9); // Set pre-charge period
  write_cmd(client, 0xF1); // Phase 1 period of 15 DCLK, Phase 2 period of 1 DCLK

  write_cmd(client, 0xDB); // Set Vcomh deselect level
  write_cmd(client, 0x20); // Vcomh deselect level ~ 0.77 Vcc

  write_cmd(client, 0xA4); // Entire display ON, resume to RAM content display
  write_cmd(client, 0xA6); // Set Display in Normal Mode, 1 = ON, 0 = OFF
  write_cmd(client, 0x2E); // Deactivate scroll
  write_cmd(client, 0xAF); // Display ON in normal mode
}

/* ssd1306_fill - fill all display with data
 * @payload : byte data to be written
 */
void ssd1306_fill(struct ssd1306 *oled, u8 payload)
{
  u16 i;

  for (i = 0; i < (oled->npage * oled->nsegment); i++)
    write_data(oled->client, payload);
}