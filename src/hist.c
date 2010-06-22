
/****************************************************************************/
/*            JMCCE  - hist.c                                               */
/*                                                                          */
/*                      $Id: hist.c,v 1.1.1.1 2002/05/03 04:01:07 kids Exp $    */
/****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>

#include "jmcce.h"
#include "hbffont.h"
#include "draw.h"
#include "hztty.h"

extern int gFont_bytes;

static int _origin, _x, _y;

/* PROBLEM: one hanzi have two different color? */

/* private function */
static void
write_one_line (int line)
{
  int i = (hztty_list->origin + line * NUM_OF_COL) % hztty_list->buf_size;
  unsigned char *bitmap, *text, *attr;

  text = &hztty_list->text_buf[i];
  attr = &hztty_list->attr_buf[i];

  for (i = 0; i < NUM_OF_COL; i++, text++, attr++) {
    if ((*text > CHAR_DEL) && i < NUM_OF_COL - 1) {
      bitmap = hbfGetBitmap ((*text << 8) + text[1], STANDARD, gFont_bytes);
      if (bitmap)		// a valid HBF Chinese char
      {
	draw_hanzi_char (i, line, bitmap, FGCOLOR (*attr), BGCOLOR (*attr));
	i++;
	text++;
	attr++;
	continue;
      }
    }
    draw_ascii_char (i, line, *text, FGCOLOR (*attr), BGCOLOR (*attr),
		     STANDARD);
  }
}


/* public function */

void
enter_history_mode (void)
{
  _origin = hztty_list->origin;
  _x = hztty_list->cur_x;
  _y = hztty_list->cur_y;
}

void
leave_history_mode (void)
{
  hztty_list->origin = _origin;
  hztty_list->cur_x = _x;
  hztty_list->cur_y = _y;
  hztty_redraw ();
}

void
history_up (void)
{
  int tmp;

  on_off_cursor (hztty_list->cur_x, hztty_list->cur_y);
  --hztty_list->cur_y;
  if (hztty_list->cur_y < 0) {
    tmp = hztty_list->origin - NUM_OF_COL;
    if (tmp < 0)
      tmp += hztty_list->buf_size;
    if (tmp != ((_origin + (NUM_OF_ROW - 1) * NUM_OF_COL) %
		hztty_list->buf_size)) {
      hztty_list->origin = tmp;
      screen_scroll_down (BLACK);
      write_one_line (0);
    }
    hztty_list->cur_y = 0;
  }
  on_off_cursor (hztty_list->cur_x, hztty_list->cur_y);
}

void
history_down (void)
{
  int tmp;

  on_off_cursor (hztty_list->cur_x, hztty_list->cur_y);
  ++hztty_list->cur_y;
  if (hztty_list->cur_y > (NUM_OF_ROW - 1)) {
    tmp = (hztty_list->origin + NUM_OF_COL) % hztty_list->buf_size;
    if ((tmp <= _origin) || (tmp > (_origin + (NUM_OF_ROW - 1) * NUM_OF_COL))) {
      hztty_list->origin = tmp;
      screen_scroll_up (BLACK);
      write_one_line (NUM_OF_ROW - 1);
    }
    hztty_list->cur_y = NUM_OF_ROW - 1;
  }
  on_off_cursor (hztty_list->cur_x, hztty_list->cur_y);
}

void
history_right (void)
{
  on_off_cursor (hztty_list->cur_x, hztty_list->cur_y);
  ++hztty_list->cur_x;
  if (hztty_list->cur_x > NUM_OF_COL - 1)
    hztty_list->cur_x = NUM_OF_COL - 1;
  on_off_cursor (hztty_list->cur_x, hztty_list->cur_y);
}

void
history_left (void)
{
  if (hztty_list->cur_x) {
    on_off_cursor (hztty_list->cur_x, hztty_list->cur_y);
    --hztty_list->cur_x;
    on_off_cursor (hztty_list->cur_x, hztty_list->cur_y);
  }
}

void
history_home (void)
{
  if (hztty_list->cur_x) {
    on_off_cursor (hztty_list->cur_x, hztty_list->cur_y);
    hztty_list->cur_x = 0;
    on_off_cursor (hztty_list->cur_x, hztty_list->cur_y);
  }
}

void
history_end (void)
{
  on_off_cursor (hztty_list->cur_x, hztty_list->cur_y);
  hztty_list->cur_x = NUM_OF_COL - 1;
  on_off_cursor (hztty_list->cur_x, hztty_list->cur_y);
}

void
history_page_down (void)
{
  int i;

  for (i = 0; (i < NUM_OF_ROW) && (hztty_list->origin != _origin); i++)
    hztty_list->origin = (hztty_list->origin + NUM_OF_COL) %
      hztty_list->buf_size;
  if (i)
    hztty_redraw ();
}

void
history_page_up (void)
{
  int i;
  for (i = 0; (i < NUM_OF_ROW) && (hztty_list->origin !=
				   ((_origin +
				     NUM_OF_ROW * NUM_OF_COL) %
				    hztty_list->buf_size)); i++) {
    hztty_list->origin -= NUM_OF_COL;
    if (hztty_list->origin < 0)
      hztty_list->origin += hztty_list->buf_size;
  }
  if (i)
    hztty_redraw ();
}
