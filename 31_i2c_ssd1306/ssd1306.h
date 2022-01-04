#ifndef __SSD1306_H__
#define __SSD1306_H__

#include <linux/module.h>

struct ssd1306
{
  struct i2c_client *client;
  u8 npage;
  u8 nsegment;
  u8 line_num;
  u8 cursor_pos;
  u8 font_size;
};

struct ssd1306 *ssd1306_new(struct i2c_client *client, u8 npage, u8 nsegment);
void ssd1306_del(struct ssd1306 *oled);
void ssd1306_init(struct ssd1306 *oled);
void ssd1306_fill(struct ssd1306 *oled, u8 payload);

#endif /* __SSD1306_H__ */