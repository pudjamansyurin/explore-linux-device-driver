#include <linux/module.h>
#include <linux/slab.h>
#include <linux/i2c.h>
#include "ssd1306.h"
#include "ssd1306_font.c"

/* write - ssd1306 write i2c frame
 *
 * Return: errno
 */
static int write(struct i2c_client *client, bool is_cmd, u8 payload)
{
  u8 buf[2] = {
      is_cmd ? 0x00 : 0x01 << 6,
      payload,
  };

  return i2c_master_send(client, buf, 2);
}

/* write_data - ssd1306 write data
 *
 * Return: errno
 */
static int write_data(struct ssd1306 *oled, u8 payload)
{
  return write(oled->client, false, payload);
}

/* write_cmd - ssd1306 write command
 * 
 * Return: errno
 */
static int write_cmd(struct ssd1306 *oled, u8 payload)
{
  return write(oled->client, true, payload);
}

/* ssd1306_new - allocate and initiate ssd1306 
 */
struct ssd1306 *ssd1306_new(struct i2c_client *client)
{
  struct ssd1306 *oled;

  oled = kmalloc(sizeof(*oled), GFP_KERNEL);
  if (oled != NULL)
  {
    oled->client = client;
    oled->line_num = 0;
    oled->cursor_pos = 0;
    oled->font_size = SSD1306_FONT_SIZE;
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
  ssd1306_display_on(oled, false);

  write_cmd(oled, 0xD5); // Set Display Clock Divide Ratio and Oscillator Frequency
  write_cmd(oled, 0x80); // Default Setting for Display Clock Divide Ratio and Oscillator

  write_cmd(oled, 0xA8); // Set Multiplex Ratio
  write_cmd(oled, 0x3F); // 64 COM lines

  write_cmd(oled, 0xD3); // Set display offset
  write_cmd(oled, 0x00); // 0 offset

  write_cmd(oled, 0x40); // Set first line as the start line of the display

  write_cmd(oled, 0x8D); // Set Charge pump
  write_cmd(oled, 0x14); // Enable charge dump during display on

  write_cmd(oled, 0x20); // Set memory addressing mode
  write_cmd(oled, 0x00); // Horizontal addressing mode

  write_cmd(oled, 0xA1); // Set segment remap with column address 127 mapped to segment 0
  write_cmd(oled, 0xC8); // Set com output scan direction, scan from com63 to com 0

  write_cmd(oled, 0xDA); // Set com pins hardware configuration
  write_cmd(oled, 0x12); // Alternative com pin config, disable com left/right remap

  ssd1306_set_brightness(oled, 0xFF);

  write_cmd(oled, 0xD9); // Set pre-charge period
  write_cmd(oled, 0xF1); // Phase 1 period of 15 DCLK, Phase 2 period of 1 DCLK

  write_cmd(oled, 0xDB); // Set Vcomh deselect level
  write_cmd(oled, 0x20); // Vcomh deselect level ~ 0.77 Vcc

  write_cmd(oled, 0xA4); // Entire display ON, resume to RAM content display
  write_cmd(oled, 0xA6); // Set Display in Normal Mode, 1 = ON, 0 = OFF
  write_cmd(oled, 0x2E); // Deactivate scroll

  ssd1306_display_on(oled, true);
}

/*
 * ssd1306_display_on - set display on/off
 * @on: on/off state
 */
void ssd1306_display_on(struct ssd1306 *oled, bool on)
{
  write_cmd(oled, on ? 0xAF : 0xAE);
}

/*
 * ssd1306_invert - inverts the display
 * @invert: do inversion
 */
void ssd1306_invert(struct ssd1306 *oled, bool invert)
{
  write_cmd(oled, invert ? 0xA7 : 0xA8);
}

/*
 * ssd1306_set_brightness - sets the brightness of  the display.
 * @value: the intensity
 */
void ssd1306_set_brightness(struct ssd1306 *oled, u8 value)
{
  write_cmd(oled, 0x81);  // Contrast command
  write_cmd(oled, value); // Contrast value (default value = 0x7F)
}



/*
 * ssd1306_set_cursor - set cursor position
 * @line: line number
 * @cursor: cursor position
 */
void ssd1306_set_cursor(struct ssd1306 *oled, u8 line, u8 cursor)
{
  /* Move the Cursor to specified position only if it is in range */
  if ((line > SSD1306_MAX_LINE) || (cursor >= SSD1306_MAX_SEG))
    return;

  oled->line_num = line;     // Save the specified line number
  oled->cursor_pos = cursor; // Save the specified cursor position

  write_cmd(oled, 0x21);                // cmd for the column start and end address
  write_cmd(oled, cursor);              // column start addr
  write_cmd(oled, SSD1306_MAX_SEG - 1); // column end addr

  write_cmd(oled, 0x22);             // cmd for the page start and end address
  write_cmd(oled, line);             // page start addr
  write_cmd(oled, SSD1306_MAX_LINE); // page end addr
}

/*
 * ssd1306_goto_newline - goto next line 
 */
void ssd1306_goto_newline(struct ssd1306 *oled)
{
  u8 line;

  /*
  ** Increment the current line number.
  ** roll it back to first line, if it exceeds the limit. 
  */
  line = oled->line_num + 1;
  if (line > SSD1306_MAX_LINE)
    line = 0;

  ssd1306_set_cursor(oled, line, 0); /* Finally move it to next line */
}

/*
 * ssd1306_scroll_h - Scrolls the data right/left in horizontally.
 * @is_left_scroll: left horizontal scroll
 * @start_line_no: Start address of the line to scroll 
 * @end_line_no: End address of the line to scroll    
 */
void ssd1306_scroll_h(struct ssd1306 *oled,
                      bool is_left_scroll,
                      u8 start_line_no,
                      u8 end_line_no)
{
  write_cmd(oled, is_left_scroll ? 0x27 : 0x26);

  write_cmd(oled, 0x00);          // Dummy byte (dont change)
  write_cmd(oled, start_line_no); // Start page address
  write_cmd(oled, 0x00);          // 5 frames interval
  write_cmd(oled, end_line_no);   // End page address
  write_cmd(oled, 0x00);          // Dummy byte (dont change)
  write_cmd(oled, 0xFF);          // Dummy byte (dont change)
  write_cmd(oled, 0x2F);          // activate scroll
}

/*
 * ssd1306_scroll_vh - Scrolls the data in vertically and right/left horizontally(Diagonally).
 * @is_vertical_left_scroll: vertical and left horizontal scroll
 * @start_line_no: Start address of the line to scroll 
 * @end_line_no: End address of the line to scroll 
 * @vertical_area: Area for vertical scroll (0-63)
 * @rows: Number of rows to scroll vertically             
 */
void ssd1306_scroll_vh(struct ssd1306 *oled,
                       bool is_vertical_left_scroll,
                       u8 start_line_no,
                       u8 end_line_no,
                       u8 vertical_area,
                       u8 rows)
{
  write_cmd(oled, 0xA3);          // Set Vertical Scroll Area
  write_cmd(oled, 0x00);          // Check datasheet
  write_cmd(oled, vertical_area); // area for vertical scroll

  write_cmd(oled, is_vertical_left_scroll ? 0x2A : 0x29);

  write_cmd(oled, 0x00);          // Dummy byte (dont change)
  write_cmd(oled, start_line_no); // Start page address
  write_cmd(oled, 0x00);          // 5 frames interval
  write_cmd(oled, end_line_no);   // End page address
  write_cmd(oled, rows);          // Vertical scrolling offset
  write_cmd(oled, 0x2F);          // activate scroll
}
/* 
 * ssd1306_fill - fill all display with data
 * @payload : byte data to be written
 */
void ssd1306_fill(struct ssd1306 *oled, u8 payload)
{
  u16 i, bits;

  bits = (SSD1306_MAX_LINE + 1) * SSD1306_MAX_SEG;

  for (i = 0; i <= bits; i++)
    write_data(oled, payload);
}

/*
 * ssd1306_clear - clear all display
 */
void ssd1306_clear(struct ssd1306 *oled)
{
	ssd1306_fill(oled, 0x00);
}

/*
 * ssd1306_print_char - sends the single char to the OLED.
 * @c: character to be written
 */
void ssd1306_print_char(struct ssd1306 *oled, unsigned char c)
{
  u8 byte;
  u8 tmp = 0;

  /*
  ** If we character is greater than segment len or we got new line charcter
  ** then move the cursor to the new line
  */
  if (((oled->cursor_pos + oled->font_size) >= SSD1306_MAX_SEG) ||
      (c == '\n'))
  {
    ssd1306_goto_newline(oled);
  }

  // print characters other than new line
  if (c != '\n')
  {

    /*
    ** In our font array (SSD1306_Font), space starts in 0th index.
    ** But in ASCII table, Space starts from 32 (0x20).
    ** So we need to match the ASCII table with our font table.
    ** We can subtract 32 (0x20) in order to match with our font table.
    */
    c -= 0x20; //or c -= ' ';

    do
    {
      byte = SSD1306_Font[c][tmp]; // Get the data to be displayed from LookUptable

      write_data(oled, byte); // write data to the OLED
      oled->cursor_pos++;

      tmp++;

    } while (tmp < oled->font_size);

    write_data(oled, 0x00); //Display the data
    oled->cursor_pos++;
  }
}

/*
 * ssd1306_print_str - sends the string to the OLED.
 * @str: string to be written
 */
void ssd1306_print_str(struct ssd1306 *oled, unsigned char *str)
{
  while (*str)
    ssd1306_print_char(oled, *str++);
}

