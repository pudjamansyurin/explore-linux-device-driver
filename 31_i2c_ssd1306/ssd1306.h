#ifndef __SSD1306_H__
#define __SSD1306_H__

#include <linux/module.h>

#define SSD1306_FONT_SIZE (5)
#define SSD1306_MAX_SEG (128) // Maximum segment
#define SSD1306_MAX_LINE (7)  // Maximum line - started from 0
struct ssd1306
{
  struct i2c_client *client;
  u8 line_num;
  u8 cursor_pos;
  u8 font_size;
};

struct ssd1306 *ssd1306_new(struct i2c_client *client);
void ssd1306_del(struct ssd1306 *oled);

void ssd1306_init(struct ssd1306 *oled);
void ssd1306_fill(struct ssd1306 *oled, u8 payload);
void ssd1306_display_on(struct ssd1306 *oled, bool on);

void ssd1306_set_cursor(struct ssd1306 *oled, u8 line, u8 cursor);
void ssd1306_goto_newline(struct ssd1306 *oled);
void ssd1306_print_char(struct ssd1306 *oled, unsigned char c);
void ssd1306_print_str(struct ssd1306 *oled, unsigned char *str);
void ssd1306_invert(struct ssd1306 *oled, bool invert);
void ssd1306_set_brightness(struct ssd1306 *oled, u8 value);
void ssd1306_scroll_h(struct ssd1306 *oled,
                      bool is_left_scroll,
                      u8 start_line_no,
                      u8 end_line_no);
void ssd1306_scroll_vh(struct ssd1306 *oled,
                       bool is_vertical_left_scroll,
                       u8 start_line_no,
                       u8 end_line_no,
                       u8 vertical_area,
                       u8 rows);

#endif /* __SSD1306_H__ */