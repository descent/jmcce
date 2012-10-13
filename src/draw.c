
/****************************************************************************/
/*            JMCCE  - draw.c                                               */
/*                                                                          */
/*                      $Id: draw.c,v 1.1.1.1 2002/05/03 04:01:07 kids Exp $    */
/****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <vga.h>
#include <sys/types.h>
#include <string.h>
//#include <asm/bitops.h>
#include <signal.h>

#include "jmcce.h"
#include "error.h"
#include "hbffont.h"
#include "draw.h"
#include "hztty.h"
#include "c_routine.h"
#include "fb.h"

extern int gFont_bytes;

int use_fb = 1;

void
screen_delete_line (int top, int bottom, int n, int bg_color)
{

  if (!active_console)
    return;
  if (top >= bottom)
    return;
  if (n > (bottom - top))
    n = bottom - top;

  c_scroll_up (top * LINE_HEIGHT, bottom * LINE_HEIGHT - 1,
	       n * LINE_HEIGHT, bg_color);
}

void
screen_insert_line (int top, int bottom, int n, int bg_color)
{

  if (!active_console)
    return;
  if (top >= bottom)
    return;
  if (n > (bottom - top))
    n = bottom - top;
  c_scroll_down (top * LINE_HEIGHT, bottom * LINE_HEIGHT - 1,
		 n * LINE_HEIGHT, bg_color);
}

inline void
screen_scroll_up (int bg_color)
{

  if (!active_console)
    return;
  c_scroll_up (0, LINE_HEIGHT * NUM_OF_ROW - 1, LINE_HEIGHT, bg_color);
}

inline void
screen_scroll_down (int bg_color)
{

  if (!active_console)
    return;
  c_scroll_down (0, LINE_HEIGHT * NUM_OF_ROW - 1, LINE_HEIGHT, bg_color);
}

inline void
screen_clear_block (int x, int y, int w, int h, int color)
{

  if (!active_console)
    return;
  c_clear_block (x, y * LINE_HEIGHT, w, h * LINE_HEIGHT, color);
}

inline void
clear_line (int line, int color)
{

  if (!active_console)
    return;
  c_clear_lines (line * LINE_HEIGHT, LINE_HEIGHT, color);
}

/* use by signal usr1 (SIGREL) handler -- relese terminal */
void
screen_flipaway (void)
{
  if (!use_fb)
    vga_setmode (TEXT);

  active_console = 0;
}

/* use by signal usr2 (SIGACQ) handler -- acquire terminal */
void
screen_return (void)
{
  if (!use_fb)
    vga_setmode (G640x480x16);

  active_console = 1;
}

void
screen_init (void)
{
  use_fb = 1;
  if (fb_init ()) {
    use_fb = 0;
    vga_init ();
    vga_setmode (G640x480x16);
  }

  active_console = 1;
}

void
screen_done (void)
{
  if (!use_fb)
    vga_setmode (TEXT);
}

inline void
on_off_cursor (int x, int y)
{

  if (!active_console)
    return;
  c_toggle_cursor (x, y * LINE_HEIGHT + FONT_HEIGHT);
}

inline void
draw_ascii_char (int x, int y, int c,
		 unsigned char fg_color, unsigned char bg_color,
		 unsigned char mode)
{
  unsigned char *buffer;


  if (!active_console)
    return;
  if (mode == STANDARD)
    buffer = &ascii_font[c][0];
  else
    buffer = ascGetBitmap (c, mode);
  c_draw_ascii (x, y * LINE_HEIGHT, buffer, fg_color | (bg_color << 8));
}

inline void
draw_hanzi_char (int x, int y, unsigned char *bitmap,
		 unsigned char fg_color, unsigned char bg_color)
{

  if (!active_console)
    return;
  c_draw_hanzi (x, y * LINE_HEIGHT, bitmap, fg_color | (bg_color << 8));
}

void
print_string (int x, int y, unsigned char *string, int fg_color, int bg_color)
{
  unsigned char *bitmap;


  while (*string) {
    bitmap =
      hbfGetBitmap ((*string << 8) + *(string + 1), STANDARD, gFont_bytes);
    if (bitmap)			// a valid HBF chinese char
    {
      draw_hanzi_char (x, y, bitmap, fg_color, bg_color);
      x += 2;
      string += 2;
    } else
      draw_ascii_char (x++, y, *string++, fg_color, bg_color, STANDARD);
  }
}
