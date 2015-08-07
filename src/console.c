
/****************************************************************************/
/*            JMCCE  - console.c                                            */
/*                                                                          */
/*                   $Id: console.c,v 1.1.1.1 2002/05/03 04:01:07 kids Exp $    */
/****************************************************************************/

/*
 * The following code is derived from linux console.c
 * Copyright by Linus.
 * Under the GPL license
 */

#include <fcntl.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#include "hbffont.h"
#include "draw.h"
#include "hztty.h"
#include "jmcce.h"
#include "hzinput.h"

#include "wstring2utf8.h"
#include "ft2.h"

#include <string>


extern int gFont_bytes;

static unsigned char color_table[] = 
{ 
  BLACK, RED, GREEN, BROWN, BLUE, MAGENTA, CYAN, GRAY,
  LIGHTBLACK, LIGHTRED, LIGHTGREEN, LIGHTBROWN, LIGHTBLUE, LIGHTMAGENTA, LIGHTCYAN, LIGHTWHITE
};

static char *translations[] = {
/* 8-bit Latin-1 mapped to the PC character set: '\0' means non-printable */
  (char *)
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\0\376\0\0\0\0\0"
    " !\"#$%&'()*+,-./0123456789:;<=>?"
    "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_"
    "`abcdefghijklmnopqrstuvwxyz{|}~\0"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\377\255\233\234\376\235\174\025\376\376\246\256\252\055\376\376"
    "\370\361\375\376\376\346\024\371\376\376\247\257\254\253\376\250"
    "\376\376\376\376\216\217\222\200\376\220\376\376\376\376\376\376"
    "\376\245\376\376\376\376\231\376\350\376\376\376\232\376\376\341"
    "\205\240\203\376\204\206\221\207\212\202\210\211\215\241\214\213"
    "\376\244\225\242\223\376\224\366\355\227\243\226\201\376\376\230",

/* vt100 graphics */
  (char *)
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\0\0\0\0\0\0\0\0\0\0\376\0\0\0\0\0"
    " !\"#$%&'()*+,-./0123456789:;<=>?" "@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^ "
#ifdef VGA_FONT
    "\004\261\007\007\007\007\370\361\007\007\331\277\332\300\305\304"
    "\304\304\137\137\303\264\301\302\263\363\362\343\330\234\007\0"
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
#else
    "\001\002\003\004\005\006\007\010\011\012\013\014\015\016\017"
    "\020\021\022\023\024\025\026\027\030\031\032\033\034\035\036"
    "\037\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
#endif
    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
    "\377\255\233\234\376\235\174\025\376\376\246\256\252\055\376\376"
    "\370\361\375\376\376\346\024\371\376\376\247\257\254\253\376\250"
    "\376\376\376\376\216\217\222\200\376\220\376\376\376\376\376\376"
    "\376\245\376\376\376\376\231\376\376\376\376\376\232\376\376\341"
    "\205\240\203\376\204\206\221\207\212\202\210\211\215\241\214\213"
    "\376\244\225\242\223\376\224\366\376\227\243\226\201\376\376\230",

/* IBM graphics: minimal translations (BS, CR, LF, LL, SO, SI and ESC) */
  (char *)
    "\000\001\002\003\004\005\006\007\000\011\000\013\000\000\000\000"
    "\020\021\022\023\024\025\026\027\030\031\032\000\034\035\036\037"
    "\040\041\042\043\044\045\046\047\050\051\052\053\054\055\056\057"
    "\060\061\062\063\064\065\066\067\070\071\072\073\074\075\076\077"
    "\100\101\102\103\104\105\106\107\110\111\112\113\114\115\116\117"
    "\120\121\122\123\124\125\126\127\130\131\132\133\134\135\136\137"
    "\140\141\142\143\144\145\146\147\150\151\152\153\154\155\156\157"
    "\160\161\162\163\164\165\166\167\170\171\172\173\174\175\176\177"
    "\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217"
    "\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237"
    "\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257"
    "\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277"
    "\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317"
    "\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337"
    "\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357"
    "\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377",

  /* USER: customizable mappings, initialized as the previous one (IBM) */
  (char *)
  "\000\001\002\003\004\005\006\007\010\011\000\013\000\000\016\017"
    "\020\021\022\023\024\025\026\027\030\031\032\000\034\035\036\037"
    "\040\041\042\043\044\045\046\047\050\051\052\053\054\055\056\057"
    "\060\061\062\063\064\065\066\067\070\071\072\073\074\075\076\077"
    "\100\101\102\103\104\105\106\107\110\111\112\113\114\115\116\117"
    "\120\121\122\123\124\125\126\127\130\131\132\133\134\135\136\137"
    "\140\141\142\143\144\145\146\147\150\151\152\153\154\155\156\157"
    "\160\161\162\163\164\165\166\167\170\171\172\173\174\175\176\177"
    "\200\201\202\203\204\205\206\207\210\211\212\213\214\215\216\217"
    "\220\221\222\223\224\225\226\227\230\231\232\233\234\235\236\237"
    "\240\241\242\243\244\245\246\247\250\251\252\253\254\255\256\257"
    "\260\261\262\263\264\265\266\267\270\271\272\273\274\275\276\277"
    "\300\301\302\303\304\305\306\307\310\311\312\313\314\315\316\317"
    "\320\321\322\323\324\325\326\327\330\331\332\333\334\335\336\337"
    "\340\341\342\343\344\345\346\347\350\351\352\353\354\355\356\357"
    "\360\361\362\363\364\365\366\367\370\371\372\373\374\375\376\377"
};

#define NORM_TRANS (translations[0])
#define GRAF_TRANS (translations[1])
#define NULL_TRANS (translations[2])
#define USER_TRANS (translations[3])

#define clear_screen(tty) clear_block(tty, 0, 0, NUM_OF_COL, NUM_OF_ROW)
/* private function */


static void
invert_attrib (hz_tty * tty)
{
  int i, j, pos;
  for (i = 0, pos = tty->origin; i < NUM_OF_ROW; i++, pos %= tty->buf_size)
    for (j = 0; j < NUM_OF_COL; j++, pos++) {
/*     tty->attr_buf[pos] = (tty->attr_buf[pos] << 4) + 
                          (tty->attr_buf[pos] >> 4);  MMMMMM */
      tty->attr_buf[pos] = ((tty->attr_buf[pos] & 0x77) >> 4) +
	((tty->attr_buf[pos] & 0x77) << 4) + (tty->attr_buf[pos] & 0x08);
    }
}

static void
set_autorepeat (hz_tty * tty, int on_off)
{
  if (on_off)
    write (console_fd, "\033[?8h", 5);
  else
    write (console_fd, "\033[?8l", 5);
  tty->autorepeat = on_off;
}

static void
set_dec_cursor_keys (hz_tty * tty, int on_off)
{
  if (on_off)
    write (console_fd, "\033[?1h", 5);
  else
    write (console_fd, "\033[?1l", 5);
  tty->dec_cursor_keys = on_off;
}

static void
set_lf_mode (hz_tty * tty, int on_off)
{
  if (on_off)
    write (console_fd, "\033[20h", 5);
  else
    write (console_fd, "\033[20l", 5);
  tty->lf_mode = on_off;
}

static void
set_applic (hz_tty * tty, int on_off)
{
  if (on_off)
    write (console_fd, "\033=", 2);
  else
    write (console_fd, "\033>", 2);
  tty->applic = on_off;
}

static void
clear_block (hz_tty * tty, int x, int y, int w, int h)
{
  int i, pos;
  for (pos = (tty->origin + y * NUM_OF_COL + x) % tty->buf_size, i = 0;
       i < h; pos = (pos + NUM_OF_COL) % tty->buf_size, i++) {
    memset (tty->text_buf + pos, 0x20, w);	//raner
    memset (tty->attr_buf + pos, COLOR_COMPOSE (tty->fg_color,
						tty->video_erase_color), w);
  }
  if (tty == hztty_list)
    screen_clear_block (x, y, w, h, tty->video_erase_color);
}

static void
insert_char (hz_tty * tty, int nr)
{
  int index = (tty->origin + tty->cur_y * NUM_OF_COL + tty->cur_x)
    % tty->buf_size;
  int count = NUM_OF_COL - tty->cur_x - nr;
  if (count < 0) {
    count = 0;
    nr = NUM_OF_COL - tty->cur_x;
  }
  memmove (tty->text_buf + index + nr, tty->text_buf + index, count);
  memmove (tty->attr_buf + index + nr, tty->attr_buf + index, count);
  memset (tty->text_buf + index, 0x20, nr);
  memset (tty->attr_buf + index, COLOR_COMPOSE (tty->fg_color,
						tty->video_erase_color), nr);
  if (tty == hztty_list)
    //screen_insert_char(tty->cur_x, tty->cur_y, nr, tty->bg_color);
    repaint_one_line (tty, tty->cur_y);
}

static void
insert_line (hz_tty * tty, int top_y, int nr)
{
  int top, bot, i;
  if (nr > tty->bottom - top_y)
    nr = tty->bottom - top_y;
  bot = (tty->origin + (tty->bottom - 1) * NUM_OF_COL) % tty->buf_size;
  top = bot - NUM_OF_COL * nr;
  if (top < 0)
    top += tty->buf_size;
  i = tty->bottom - nr - top_y;
  for (; i > 0; i--) {
    memcpy (tty->text_buf + bot, tty->text_buf + top, NUM_OF_COL);
    memcpy (tty->attr_buf + bot, tty->attr_buf + top, NUM_OF_COL);
    top -= NUM_OF_COL;
    if (top < 0)
      top += tty->buf_size;
    bot -= NUM_OF_COL;
    if (bot < 0)
      bot += tty->buf_size;
  }
  for (i = 0; i < nr; i++) {
    top = (top + NUM_OF_COL) % tty->buf_size;
    memset (tty->text_buf + top, 0x20, NUM_OF_COL);
    memset (tty->attr_buf + top, COLOR_COMPOSE (tty->fg_color,
						tty->video_erase_color),
	    NUM_OF_COL);
  }
  if (tty == hztty_list)
    screen_insert_line (top_y, tty->bottom, nr, tty->video_erase_color);
}

static void
delete_char (hz_tty * tty, int nr)
{
  int index = (tty->origin + tty->cur_y * NUM_OF_COL + tty->cur_x) %
    tty->buf_size;
  int count = NUM_OF_COL - tty->cur_x - nr;
  if (count < 0) {
    count = 0;
    nr = NUM_OF_COL - tty->cur_x;
  }
  memcpy (tty->text_buf + index, tty->text_buf + index + nr, count);
  memcpy (tty->attr_buf + index, tty->attr_buf + index + nr, count);
  memset (tty->text_buf + index + count, 0x20, nr);
  memset (tty->attr_buf + index + count, COLOR_COMPOSE (tty->fg_color,
							tty->
							video_erase_color),
	  nr);
  if (tty == hztty_list)
    //screen_delete_char(tty->cur_x, tty->cur_y, nr, tty->video_erase_color);
    repaint_one_line (tty, tty->cur_y);
}

static void
delete_line (hz_tty * tty, int top_y, int nr)
{
  int bot, i, top;

  if (nr > tty->bottom - top_y)
    nr = tty->bottom - top_y;
  top = (tty->origin + top_y * NUM_OF_COL) % tty->buf_size;
  bot = (top + NUM_OF_COL * nr) % tty->buf_size;
  i = tty->bottom - nr - top_y;
  for (; i > 0; i--) {
    memcpy (tty->text_buf + top, tty->text_buf + bot, NUM_OF_COL);
    memcpy (tty->attr_buf + top, tty->attr_buf + bot, NUM_OF_COL);
    top = (top + NUM_OF_COL) % tty->buf_size;
    bot = (bot + NUM_OF_COL) % tty->buf_size;
  }
  for (i = 0; i < nr; i++) {
    memset (tty->text_buf + top, 0x20, NUM_OF_COL);
    memset (tty->attr_buf + top, COLOR_COMPOSE (tty->fg_color,
						tty->video_erase_color),
	    NUM_OF_COL);
    top = (top + NUM_OF_COL) % tty->buf_size;
  }
  if (tty == hztty_list)
    screen_delete_line (top_y, tty->bottom, nr, tty->video_erase_color);
}

static void
scroll_up (hz_tty * tty)
{
  int index;

  if ((tty->top == 0) && (tty->bottom == NUM_OF_ROW)) {
    tty->origin = (tty->origin + NUM_OF_COL) % tty->buf_size;
    index = (tty->origin + (NUM_OF_ROW - 1) * NUM_OF_COL) % tty->buf_size;
    memset (tty->text_buf + index, 0x20, NUM_OF_COL);
    memset (tty->attr_buf + index, COLOR_COMPOSE (tty->fg_color,
						  tty->video_erase_color),
	    NUM_OF_COL);
    if (hztty_list == tty)
      screen_scroll_up (tty->video_erase_color);
  } else
    delete_line (tty, tty->top, 1);
}

static void
scroll_down (hz_tty * tty)
{
  if ((tty->top == 0) && (tty->bottom == NUM_OF_ROW)) {
    tty->origin -= NUM_OF_COL;
    if (tty->origin < 0)
      tty->origin += tty->buf_size;
    memset (tty->text_buf + tty->origin, 0x20, NUM_OF_COL);
    memset (tty->attr_buf + tty->origin, COLOR_COMPOSE (tty->fg_color,
							tty->
							video_erase_color),
	    NUM_OF_COL);
    if (hztty_list == tty)
      screen_scroll_down (tty->video_erase_color);
  } else
    insert_line (tty, tty->top, 1);

}

static void
default_attr (hz_tty * tty)
{
  tty->underline = 0;
  tty->blink = 0;
  tty->reverse = 0;
  tty->intensity = 1;
  tty->foreground_color = DEFAULT_FOREGROUND_COLOR;
  tty->background_color = DEFAULT_BACKGROUND_COLOR;
}

inline static void
cr (hz_tty * tty)
{
  tty->need_wrap = tty->cur_x = 0;
}

static void
lf (hz_tty * tty)
{
  if (tty->cur_y + 1 == tty->bottom)
    scroll_up (tty);
  else if (tty->cur_y < NUM_OF_ROW - 1)
    tty->cur_y++;
  tty->need_wrap = 0;
}

static void
ri (hz_tty * tty)
{
  if (tty->cur_y == tty->top)
    scroll_down (tty);
  else if (tty->cur_y > tty->top)
    tty->cur_y--;
  tty->need_wrap = 0;
}

static void
status_report (hz_tty * tty)
{
  static const char status[] = "\033[0n";
  write (tty->tty_fd, status, strlen (status));
}

static void
cursor_report (hz_tty * tty)
{
  char buf[40];
  sprintf (buf, "\033[%d;%dR", tty->cur_y + (tty->origin_mode ? tty->top + 1 :
					     1), tty->cur_x + 1);
  write (tty->tty_fd, buf, strlen (buf));
}

/*
 * gotoxy() must verify all boundaries, because the arguments
 * might also be negative. If the given position is out of
 * bounds, the cursor is placed at the nearest margin.
 */
static void
gotoxy (hz_tty * tty, int new_x, int new_y)
{
  int max_y;

  if (new_x < 0)
    tty->cur_x = 0;
  else if (new_x >= NUM_OF_COL)
    tty->cur_x = NUM_OF_COL - 1;
  else
    tty->cur_x = new_x;
  if (new_y < 0)
    new_y = 0;
  if (tty->origin_mode) {
    new_y += tty->top;
    max_y = tty->bottom;
  } else
    max_y = NUM_OF_ROW;
  if (new_y >= max_y)
    tty->cur_y = max_y - 1;
  else
    tty->cur_y = new_y;
  tty->need_wrap = 0;
}

static void
csi_J (hz_tty * tty, int vpar)
{
  switch (vpar) {
  case 0:			/* erase from cursor to end of display */
    clear_block (tty, tty->cur_x, tty->cur_y, NUM_OF_COL - tty->cur_x, 1);
    if (tty->cur_y + 1 < NUM_OF_ROW)
      clear_block (tty, 0, tty->cur_y + 1, NUM_OF_COL,
		   NUM_OF_ROW - (tty->cur_y + 1));
    break;
  case 1:			/* erase from start to cursor */
    if (tty->cur_y)
      clear_block (tty, 0, 0, NUM_OF_COL, tty->cur_y);
    clear_block (tty, 0, tty->cur_y, tty->cur_x + 1, 1);
    break;
  case 2:			/* erase whole display */
    clear_screen (tty);
    break;
  default:
    return;
  }
  tty->need_wrap = 0;
}

static void
csi_K (hz_tty * tty, int vpar)
{
  switch (vpar) {
  case 0:			/* erase from cursor to end of line */
    clear_block (tty, tty->cur_x, tty->cur_y, NUM_OF_COL - tty->cur_x, 1);
    break;
  case 1:			/* erase from start of line to cursor */
    clear_block (tty, 0, tty->cur_y, tty->cur_x + 1, 1);
    break;
  case 2:			/* erase whole line */
    clear_block (tty, 0, tty->cur_y, NUM_OF_COL, 1);
    break;
  default:
    return;
  }
  tty->need_wrap = 0;
}

static void
csi_L (hz_tty * tty, int nr)
{
  if (nr > NUM_OF_ROW)
    nr = NUM_OF_ROW;
  else if (!nr)
    nr = 1;
  insert_line (tty, tty->cur_y, nr);
}

static void
csi_at (hz_tty * tty, int nr)
{
  if (nr > NUM_OF_COL)
    nr = NUM_OF_COL;
  else if (!nr)
    nr = 1;
  insert_char (tty, nr);
}

static void
csi_M (hz_tty * tty, int nr)
{
  if (nr > NUM_OF_ROW)
    nr = NUM_OF_ROW;
  else if (!nr)
    nr = 1;
  delete_line (tty, tty->cur_y, nr);
}

static void
csi_P (hz_tty * tty, int nr)
{
  if (nr > NUM_OF_COL)
    nr = NUM_OF_COL;
  else if (!nr)
    nr = 1;
  delete_char (tty, nr);
}

static void
csi_X (hz_tty * tty, int vpar)
{
  int count;
  if (!vpar)
    vpar++;
#if 0				/* fixed */
  /* this bug is from yact */
  /* untest part */
  count = (vpar > NUM_OF_COL - tty->cur_x - 1) ?
    (NUM_OF_COL - tty->cur_x - 1) : vpar;
  clear_block (tty, tty->cur_x, tty->cur_y, tty->cur_x + vpar, tty->cur_y);
#else /* 0.16 */
  /* By cnoize */
  if (tty->cur_x + vpar >= NUM_OF_COL)
    clear_block (tty, tty->cur_x, tty->cur_y, NUM_OF_COL - tty->cur_x - 1, 1);
  else
    clear_block (tty, tty->cur_x, tty->cur_y, vpar, 1);
#endif
  tty->need_wrap = 0;
}


static void
respond_id (hz_tty * tty)
{
  static const char VT102ID[] = "\033[?6c";

  write (tty->tty_fd, VT102ID, strlen (VT102ID));
}

static void
update_attr (hz_tty * tty)
{
  if (tty->reverse ^ tty->invert_mode) {
    tty->bg_color = tty->foreground_color;
    tty->fg_color = tty->background_color;
  } else {
    tty->bg_color = tty->background_color;
    tty->fg_color = tty->foreground_color;
  }
  if (tty->intensity == 2)
    tty->fg_color ^= 0x08;
  if (tty->invert_mode)
    tty->video_erase_color = tty->foreground_color;
  else
    tty->video_erase_color = tty->background_color;
#if 0
  if (tty->bg_color==255)
    tty->bg_color = LIGHTWHITE;
  if (tty->fg_color==255)
    tty->fg_color = LIGHTWHITE;
#endif
}

#if 0
ref: http://www.termsys.demon.co.uk/vtansi.htm
ref: http://en.wikipedia.org/wiki/ANSI_escape_code#Colors

Set Display Attributes
0       Reset all attributes
1       Bright
2       Dim
4       Underscore      
5       Blink
7       Reverse
8       Hidden

        Foreground Colours
30      Black
31      Red
32      Green
33      Yellow
34      Blue
35      Magenta
36      Cyan
37      White

        Background Colours
40      Black
41      Red
42      Green
43      Yellow
44      Blue
45      Magenta
46      Cyan
47      White
#endif

static void
csi_m (hz_tty * tty)
{
  int i;

  for (i = 0; i <= tty->npar; i++)
    switch (tty->param.par[i]) {
    case 0:			/* all attributes off */
      default_attr (tty);
      break;
    case 1:
      tty->intensity = 2;
      break;
    case 2:
      tty->intensity = 0;
      break;
    case 4:
      tty->underline = 1;
      break;
    case 5:
      tty->blink = 1;
      break;
    case 7:
      tty->reverse = 1;
      break;
    case 21:
    case 22:
      tty->intensity = 1;
      break;
    case 24:
      tty->underline = 0;
      break;
    case 25:
      tty->blink = 0;
      break;
    case 27:
      tty->reverse = 0;
      break;
    case 38:			/* ANSI X3.64-1979 (SCO-ish?)
				 * Enables underscore, white foreground
				 * with white underscore (Linux - use
				 * default foreground).
				 */
      break;
    case 39:			/* ANSI X3.64-1979 (SCO-ish?)
				 * Disable underline option.
				 * Reset colour to default? It did this
				 * before...
				 */
      break;
    case 49:
      break;
    default:
      if (tty->param.par[i] >= 30 && tty->param.par[i] <= 37)
	tty->foreground_color = color_table[tty->param.par[i] - 30];
      else if (tty->param.par[i] >= 40 && tty->param.par[i] <= 47)
	tty->background_color = color_table[tty->param.par[i] - 40];
      break;
    }
  update_attr (tty);
}


static void
setterm_command (hz_tty * tty)
{
}

static void
save_cur (hz_tty * tty)
{
  tty->saved_cur_x = tty->cur_x;
  tty->saved_cur_y = tty->cur_y;
  tty->saved_fg_color = tty->foreground_color;
  tty->saved_bg_color = tty->background_color;
  tty->saved_G0 = tty->G0_charset;
  tty->saved_G1 = tty->G1_charset;
  tty->s_charset = tty->charset;
  tty->s_intensity = tty->intensity;
  tty->s_underline = tty->underline;
  tty->s_blink = tty->blink;
  tty->s_reverse = tty->reverse;
}

static void
restore_cur (hz_tty * tty)
{
  gotoxy (tty, tty->saved_cur_x, tty->saved_cur_y);
  tty->charset = tty->s_charset;
  tty->intensity = tty->s_intensity;
  tty->underline = tty->s_underline;
  tty->blink = tty->s_blink;
  tty->reverse = tty->s_reverse;
  tty->foreground_color = tty->saved_fg_color;
  tty->background_color = tty->saved_bg_color;
  tty->G0_charset = tty->saved_G0;
  tty->G1_charset = tty->saved_G1;
  tty->Translate = tty->charset ? tty->G1_charset : tty->G0_charset;
  tty->need_wrap = 0;
  update_attr (tty);
}

static void
set_mode (hz_tty * tty, int on_off)
{
  int i;

  for (i = 0; i <= tty->npar; i++)
    if (tty->ques)
      switch (tty->param.par[i]) {	/* DEC private modes set/reset */
      case 1:			/* Cursor keys send ^[Ox/^[[x */
	set_dec_cursor_keys (tty, on_off);
	break;
      case 3:			/* 80/132 mode switch unimplemented */
	clear_screen (tty);
	gotoxy (tty, 0, 0);
	break;
      case 4:
	/* soft scroll/Jump scroll toggle, not implement yet */
	break;
      case 5:			/* Inverted screen on/off */
	if (tty->invert_mode != on_off) {
	  tty->invert_mode = on_off;
	  invert_attrib (tty);
	  update_attr (tty);
	  if (tty == hztty_list)
	    repaint_hztty ();
	}
	break;
      case 6:			/* Origin relative/absolute */
	tty->origin_mode = on_off;
	gotoxy (tty, 0, 0);
	break;
      case 7:			/* Autowrap on/off */
	tty->autowrap_mode = on_off;
	break;
      case 8:			/* Autorepeat on/off */
	set_autorepeat (tty, on_off);
	break;
      case 25:			/* Cursor on/off */
	tty->cursor_visible = on_off;
/*        set_cursor(currcons);*/
	break;
    } else
      switch (tty->param.par[i]) {	/* ANSI modes set/reset */
      case 4:			/* Insert Mode on/off */
	tty->insert_mode = on_off;
	break;
      case 20:			/* Lf, Enter == CrLf/Lf */
	set_lf_mode (tty, on_off);
	break;
      }
}

static void
print_ascii_char (hz_tty * tty, unsigned char c)
{
  if (tty->need_wrap) {
    cr (tty);
    lf (tty);
  }
  if (tty->insert_mode)
    insert_char (tty, 1);
  tty->text_buf[(tty->origin + tty->cur_x + tty->cur_y * NUM_OF_COL) % tty->buf_size] = c;	/* tty->Translate[c]; */
  tty->attr_buf[(tty->origin + tty->cur_x + tty->cur_y * NUM_OF_COL) %
		tty->buf_size] = COLOR_COMPOSE (tty->fg_color, tty->bg_color);
  if (tty == hztty_list)
    draw_ascii_char (tty->cur_x, tty->cur_y, c,	/*tty->Translate[c], */
		     tty->fg_color, tty->bg_color,
		     tty->underline /*STANDARD*/);
  if (tty->cur_x + 1 == NUM_OF_COL)
    tty->need_wrap = tty->autowrap_mode;
  else
    tty->cur_x++;
}

/* public function */

void
repaint_one_line (hz_tty * tty, int line)
{
  int i = (tty->origin + line * NUM_OF_COL) % tty->buf_size;
  unsigned char *bitmap, *text, *attr;

  text = &tty->text_buf[i];
  attr = &tty->attr_buf[i];

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

void
repaint_hztty (void)
{
  int i, j, index;
  unsigned char *bitmap;

  for (i = 0, index = hztty_list->origin; i < NUM_OF_ROW; i++) {
    for (j = 0; j < NUM_OF_COL; j++,
	 index = (index + 1) % hztty_list->buf_size) {
      if ((hztty_list->text_buf[index] > CHAR_DEL) && j < NUM_OF_COL - 1) {
	bitmap = hbfGetBitmap ((hztty_list->text_buf[index] << 8) +
			       hztty_list->text_buf[index + 1], STANDARD,
			       gFont_bytes);
	if (bitmap) {
	  draw_hanzi_char (j, i, bitmap,
			   FGCOLOR (hztty_list->attr_buf[index]),
			   BGCOLOR (hztty_list->attr_buf[index]));
	  j++;
	  index = (index + 1) % hztty_list->buf_size;
	  continue;
	}
      }
      draw_ascii_char (j, i, hztty_list->text_buf[index],
		       FGCOLOR (hztty_list->attr_buf[index]),
		       BGCOLOR (hztty_list->attr_buf[index]), STANDARD);
    }
  }
}

void
reset_terminal (hz_tty * tty, int do_clear)
{
  if (tty->terminate)
    return;
  tty->top = 0;
  tty->bottom = NUM_OF_ROW;
  tty->need_wrap = 0;
  tty->invert_mode = 0;
  tty->insert_mode = 0;
  tty->origin_mode = 0;
  tty->cursor_visible = 1;
  tty->autowrap_mode = 1;
  tty->ques = 0;
  tty->Translate = NORM_TRANS;
  tty->G0_charset = NORM_TRANS;
  tty->G1_charset = GRAF_TRANS;
  tty->charset = 0;
  default_attr (tty);
  tty->terminal_state = STATE_NORMAL;

  tty->tab_stop[0] = 0x01010100;
  tty->tab_stop[1] =
    tty->tab_stop[2] = tty->tab_stop[3] = tty->tab_stop[4] = 0x01010101;
  update_attr (tty);
  set_autorepeat (tty, 1);
  set_dec_cursor_keys (tty, 0);
  set_lf_mode (tty, 0);
  set_applic (tty, 0);
  if (do_clear) {
    gotoxy (tty, 0, 0);
    clear_screen (tty);
    save_cur (tty);
  }
}

void hztty_write(hz_tty * tty, unsigned char *buf, int num)
{
  static Ft2 ft2("../fonts/unifont.pcf.gz");

  std::string utf8_str((char *)buf, num);
  std::wstring utf32_str = utf8_to_wstring(utf8_str);
  //std::wstring utf32_str = utf8_to_wstring("a中文bc");

  if (tty == hztty_list)
    on_off_cursor (tty->cur_x, tty->cur_y);
  for (size_t i=0 ; i < utf32_str.size() ; ++i)
  {
   //printf("utf32_str[i]: %d\n", utf32_str[i]);

    if (isascii(utf32_str[i]) && (!isprint(utf32_str[i])) ) // controll char
    {
    }
    else  // printable ascii or ucs4 glyph
    {
      if (tty->terminal_state == STATE_NORMAL) 
      {
        FT_GlyphSlot slot;

        ft2.get_slot(slot, utf32_str[i]);
        my_draw_bitmap_mono(&slot->bitmap, tty->cur_x * 8, tty->cur_y * 18 , tty->fg_color, tty->bg_color);

        if (tty->need_wrap) 
        {
          cr (tty);
          lf (tty);
        }

	if (tty->insert_mode)
	  insert_char (tty, slot->bitmap.width/8);

        if ((tty->cur_x + (slot->bitmap.width/8)) == NUM_OF_COL)
          tty->need_wrap = tty->autowrap_mode;
        else
          tty->cur_x += (slot->bitmap.width/8);
       
        #if 0
        tty->cur_x += slot->bitmap.width;
        if (tty->cur_x > 640)
        {
          tty->cur_y += 18;
          tty->cur_x = 0;
        }
        #endif
      }
    }

    #if 1

    switch (utf32_str[i])
    {
    case CHAR_BEEP:
      beep1 ();
      continue;
    case CHAR_BS:
      if (tty->cur_x)
	tty->cur_x--;
      tty->need_wrap = 0;
      continue;
    case CHAR_VT:
      while (tty->cur_x < NUM_OF_COL - 1) {
	tty->cur_x++;
	if (tty->tab_stop[tty->cur_x >> 5] & (1 << (tty->cur_x & 31)))
	  break;
      }
      continue;
    case CHAR_CR:
      cr (tty);
      continue;
    case CHAR_LF:
      lf (tty);
      continue;
    case CHAR_S0:
      tty->charset = 1;
      tty->Translate = tty->G1_charset;
      continue;
    case CHAR_S1:
      tty->charset = 0;
      tty->Translate = tty->G0_charset;
      continue;
    case CHAR_ESC:
      tty->terminal_state = STATE_ESCAPE;
      continue;
    }
    switch (tty->terminal_state) {
    case STATE_ESCAPE:
      tty->terminal_state = STATE_NORMAL;
      switch (utf32_str[i]) {
      case '[':
	tty->terminal_state = STATE_SQUARE;
	continue;
      case 'E':
	cr (tty);
	lf (tty);
	continue;
      case 'M':
	ri (tty);
	continue;
      case 'D':
	lf (tty);
	continue;
      case 'H':
	tty->tab_stop[tty->cur_x >> 5] |= (1 << (tty->cur_x & 31));
	continue;
      case 'Z':
	respond_id (tty);
	continue;
      case '7':
	save_cur (tty);
	continue;
      case '8':
	restore_cur (tty);
	continue;
      case '(':
	tty->terminal_state = STATE_SETG0;
	continue;
      case ')':
	tty->terminal_state = STATE_SETG1;
	continue;
      case '#':
	tty->terminal_state = STATE_HASH;
	continue;
      case 'c':
	reset_terminal (tty, 1);
	continue;
      case '>':		/* numberic keyboard */
	set_applic (tty, 0);
	continue;
      case '=':		/* Appl. keyboard */
	set_applic (tty, 1);
	continue;
      }
    case STATE_FUNCKEY:
      tty->terminal_state = STATE_NORMAL;
      continue;
    case STATE_HASH:
      tty->terminal_state = STATE_NORMAL;
      if (utf32_str[i] == '8') {
	int i, j;
	if (hztty_list == tty)
	  for (i = 0; i < NUM_OF_ROW; i++)
	    for (j = 0; j < NUM_OF_COL; j++)
	      draw_ascii_char (j, i, 'E', tty->fg_color,
			       tty->bg_color, STANDARD);
      }
      continue;
    case STATE_SETG0:
      if (utf32_str[i] == '0')
	tty->G0_charset = GRAF_TRANS;
      else if (utf32_str[i] == 'B')
	tty->G0_charset = NORM_TRANS;
      else if (utf32_str[i] == 'U')
	tty->G0_charset = NULL_TRANS;
      else if (utf32_str[i] == 'K')
	tty->G0_charset = USER_TRANS;
      if (tty->charset == 0)
	tty->Translate = tty->G0_charset;
      tty->terminal_state = STATE_NORMAL;
      continue;
    case STATE_SETG1:
      if (utf32_str[i] == '0')
	tty->G1_charset = GRAF_TRANS;
      else if (utf32_str[i] == 'B')
	tty->G1_charset = NORM_TRANS;
      else if (utf32_str[i] == 'U')
	tty->G1_charset = NULL_TRANS;
      else if (utf32_str[i] == 'K')
	tty->G1_charset = USER_TRANS;
      if (tty->charset == 1)
	tty->Translate = tty->G1_charset;
      tty->terminal_state = STATE_NORMAL;
      continue;
    case STATE_SQUARE:
      for (tty->npar = 0; tty->npar < NPAR; tty->npar++)
	tty->param.par[tty->npar] = 0;
      tty->npar = 0;
      tty->terminal_state = STATE_GETPARS;
      if (utf32_str[i] == '[') {	/* Function key */
	tty->terminal_state = STATE_FUNCKEY;
	continue;
      }
      tty->ques = (utf32_str[i] == '?');
      if (tty->ques)
	continue;
    case STATE_GETPARS:
      if (utf32_str[i] == ';' && tty->npar < NPAR - 1) {
	tty->npar++;
	continue;
      } else if (utf32_str[i] >= '0' && utf32_str[i] <= '9') {
	tty->param.par[tty->npar] *= 10;
	tty->param.par[tty->npar] += utf32_str[i] - '0';
	continue;
      }
      tty->terminal_state = STATE_NORMAL;
      switch (utf32_str[i]) {
      case 'h':
	set_mode (tty, 1);
	continue;
      case 'l':
	set_mode (tty, 0);
	continue;
      case 'n':
	if (!tty->ques) {
	  if (tty->param.par[0] == 5)
	    status_report (tty);
	  else if (tty->param.par[0] == 6)
	    cursor_report (tty);
	}
	continue;
      }
      if (tty->ques) {
	tty->ques = 0;
	continue;
      }
      switch (utf32_str[i]) {
      case 'G':
      case '`':
	if (tty->param.par[0])
	  tty->param.par[0]--;
	gotoxy (tty, tty->param.par[0], tty->cur_y);
	continue;
      case 'A':
	if (!tty->param.par[0])
	  tty->param.par[0]++;
	gotoxy (tty, tty->cur_x, tty->cur_y - tty->param.par[0]);
	continue;
      case 'B':
      case 'e':
	if (!tty->param.par[0])
	  tty->param.par[0]++;
	gotoxy (tty, tty->cur_x, tty->cur_y + tty->param.par[0]);
	continue;
      case 'C':
      case 'a':
	if (!tty->param.par[0])
	  tty->param.par[0]++;
	gotoxy (tty, tty->cur_x + tty->param.par[0], tty->cur_y);
	continue;
      case 'D':
	if (!tty->param.par[0])
	  tty->param.par[0]++;
	gotoxy (tty, tty->cur_x - tty->param.par[0], tty->cur_y);
	continue;
      case 'E':
	if (!tty->param.par[0])
	  tty->param.par[0]++;
	gotoxy (tty, 0, tty->cur_y + tty->param.par[0]);
	continue;
      case 'F':
	if (!tty->param.par[0])
	  tty->param.par[0]++;
	gotoxy (tty, 0, tty->cur_y - tty->param.par[0]);
	continue;
      case 'd':
	if (tty->param.par[0])
	  tty->param.par[0]--;
	gotoxy (tty, tty->cur_x, tty->param.par[0]);
	continue;
      case 'H':
      case 'f':
	if (tty->param.par[0])
	  tty->param.par[0]--;
	if (tty->param.par[1])
	  tty->param.par[1]--;
	gotoxy (tty, tty->param.par[1], tty->param.par[0]);
	continue;
      case 'J':
	csi_J (tty, tty->param.par[0]);
	continue;
      case 'K':
	csi_K (tty, tty->param.par[0]);
	continue;
      case 'L':
	csi_L (tty, tty->param.par[0]);
	continue;
      case 'M':
	csi_M (tty, tty->param.par[0]);
	continue;
      case 'P':
	csi_P (tty, tty->param.par[0]);
	continue;
      case 'c':
	if (!tty->param.par[0])
	  respond_id (tty);
	continue;
      case 'g':
	if (!tty->param.par[0])
	  tty->tab_stop[tty->cur_x >> 5] &= ~(1 << (tty->cur_x & 31));
	else if (tty->param.par[0] == 3) {
	  tty->tab_stop[0] = tty->tab_stop[1] = tty->tab_stop[2] =
	    tty->tab_stop[3] = tty->tab_stop[4] = 0;
	}
	continue;
      case 'm':
	csi_m (tty);
	continue;
      case 'r':
	if (!tty->param.par[0])
	  tty->param.par[0]++;
	if (!tty->param.par[1])
	  tty->param.par[1] = NUM_OF_ROW;
	/* Minimum allowed region is 2 lines */
	if (tty->param.par[0] < tty->param.par[1] &&
	    tty->param.par[1] <= NUM_OF_ROW) {
	  tty->top = tty->param.par[0] - 1;
	  tty->bottom = tty->param.par[1];
	  gotoxy (tty, 0, 0);
	}
	continue;
      case 's':
	save_cur (tty);
	continue;
      case 'u':
	restore_cur (tty);
	continue;
      case 'X':
	csi_X (tty, tty->param.par[0]);
	continue;
      case '@':
	csi_at (tty, tty->param.par[0]);
	continue;
      case ']':		/* setterm functions */
	setterm_command (tty);
	continue;
      }
      continue;
    }
    #endif
  }
  if (tty == hztty_list) {
    on_off_cursor (tty->cur_x, tty->cur_y);
/* screen_flush();*/

  }
}

void org_hztty_write(hz_tty * tty, unsigned char *buf, int num)
{
  unsigned char *bitmap;

  if (tty == hztty_list)
    on_off_cursor (tty->cur_x, tty->cur_y);
  for (; num; num--, buf++) {
  begin:
    if ((*buf >= CHAR_SPC) && (tty->terminal_state == STATE_NORMAL)) {
      if (*buf <= CHAR_DEL)
	print_ascii_char (tty, *buf);
      else {
	tty->terminal_state = STATE_HZCODE;
	tty->param.par[0] = *buf;
      }
      continue;
    }
    if (tty->terminal_state == STATE_HZCODE) {
      tty->terminal_state = STATE_NORMAL;
      bitmap = hbfGetBitmap ((tty->param.par[0] << 8) + *buf,
			     tty->underline /*STANDARD*/, gFont_bytes);
      if (bitmap && (tty->cur_x != NUM_OF_COL - 1)) {
	int index;
	if (tty->need_wrap) {
	  cr (tty);
	  lf (tty);
	}
	if (tty->insert_mode)
	  insert_char (tty, 2);

	index = (tty->origin + tty->cur_x + tty->cur_y * NUM_OF_COL) %
	  tty->buf_size;
	tty->text_buf[index] = tty->param.par[0];
	tty->text_buf[index + 1] = *buf;
	tty->attr_buf[index] = tty->attr_buf[index + 1] =
	  COLOR_COMPOSE (tty->fg_color, tty->bg_color);
	if (hztty_list == tty)
	  draw_hanzi_char (tty->cur_x, tty->cur_y,
			   bitmap, tty->fg_color, tty->bg_color);
	if (tty->cur_x + 2 == NUM_OF_COL)
	  tty->need_wrap = tty->autowrap_mode;
	else
	  tty->cur_x += 2;
	continue;
      } else {
	print_ascii_char (tty, tty->param.par[0]);
	goto begin;
      }
    }
    switch (*buf) {
    case CHAR_BEEP:
      beep1 ();
      continue;
    case CHAR_BS:
      if (tty->cur_x)
	tty->cur_x--;
      tty->need_wrap = 0;
      continue;
    case CHAR_VT:
      while (tty->cur_x < NUM_OF_COL - 1) {
	tty->cur_x++;
	if (tty->tab_stop[tty->cur_x >> 5] & (1 << (tty->cur_x & 31)))
	  break;
      }
      continue;
    case CHAR_CR:
      cr (tty);
      continue;
    case CHAR_LF:
      lf (tty);
      continue;
    case CHAR_S0:
      tty->charset = 1;
      tty->Translate = tty->G1_charset;
      continue;
    case CHAR_S1:
      tty->charset = 0;
      tty->Translate = tty->G0_charset;
      continue;
    case CHAR_ESC:
      tty->terminal_state = STATE_ESCAPE;
      continue;
    }
    switch (tty->terminal_state) {
    case STATE_ESCAPE:
      tty->terminal_state = STATE_NORMAL;
      switch (*buf) {
      case '[':
	tty->terminal_state = STATE_SQUARE;
	continue;
      case 'E':
	cr (tty);
	lf (tty);
	continue;
      case 'M':
	ri (tty);
	continue;
      case 'D':
	lf (tty);
	continue;
      case 'H':
	tty->tab_stop[tty->cur_x >> 5] |= (1 << (tty->cur_x & 31));
	continue;
      case 'Z':
	respond_id (tty);
	continue;
      case '7':
	save_cur (tty);
	continue;
      case '8':
	restore_cur (tty);
	continue;
      case '(':
	tty->terminal_state = STATE_SETG0;
	continue;
      case ')':
	tty->terminal_state = STATE_SETG1;
	continue;
      case '#':
	tty->terminal_state = STATE_HASH;
	continue;
      case 'c':
	reset_terminal (tty, 1);
	continue;
      case '>':		/* numberic keyboard */
	set_applic (tty, 0);
	continue;
      case '=':		/* Appl. keyboard */
	set_applic (tty, 1);
	continue;
      }
    case STATE_FUNCKEY:
      tty->terminal_state = STATE_NORMAL;
      continue;
    case STATE_HASH:
      tty->terminal_state = STATE_NORMAL;
      if (*buf == '8') {
	int i, j;
	if (hztty_list == tty)
	  for (i = 0; i < NUM_OF_ROW; i++)
	    for (j = 0; j < NUM_OF_COL; j++)
	      draw_ascii_char (j, i, 'E', tty->fg_color,
			       tty->bg_color, STANDARD);
      }
      continue;
    case STATE_SETG0:
      if (*buf == '0')
	tty->G0_charset = GRAF_TRANS;
      else if (*buf == 'B')
	tty->G0_charset = NORM_TRANS;
      else if (*buf == 'U')
	tty->G0_charset = NULL_TRANS;
      else if (*buf == 'K')
	tty->G0_charset = USER_TRANS;
      if (tty->charset == 0)
	tty->Translate = tty->G0_charset;
      tty->terminal_state = STATE_NORMAL;
      continue;
    case STATE_SETG1:
      if (*buf == '0')
	tty->G1_charset = GRAF_TRANS;
      else if (*buf == 'B')
	tty->G1_charset = NORM_TRANS;
      else if (*buf == 'U')
	tty->G1_charset = NULL_TRANS;
      else if (*buf == 'K')
	tty->G1_charset = USER_TRANS;
      if (tty->charset == 1)
	tty->Translate = tty->G1_charset;
      tty->terminal_state = STATE_NORMAL;
      continue;
    case STATE_SQUARE:
      for (tty->npar = 0; tty->npar < NPAR; tty->npar++)
	tty->param.par[tty->npar] = 0;
      tty->npar = 0;
      tty->terminal_state = STATE_GETPARS;
      if (*buf == '[') {	/* Function key */
	tty->terminal_state = STATE_FUNCKEY;
	continue;
      }
      tty->ques = (*buf == '?');
      if (tty->ques)
	continue;
    case STATE_GETPARS:
      if (*buf == ';' && tty->npar < NPAR - 1) {
	tty->npar++;
	continue;
      } else if (*buf >= '0' && *buf <= '9') {
	tty->param.par[tty->npar] *= 10;
	tty->param.par[tty->npar] += *buf - '0';
	continue;
      }
      tty->terminal_state = STATE_NORMAL;
      switch (*buf) {
      case 'h':
	set_mode (tty, 1);
	continue;
      case 'l':
	set_mode (tty, 0);
	continue;
      case 'n':
	if (!tty->ques) {
	  if (tty->param.par[0] == 5)
	    status_report (tty);
	  else if (tty->param.par[0] == 6)
	    cursor_report (tty);
	}
	continue;
      }
      if (tty->ques) {
	tty->ques = 0;
	continue;
      }
      switch (*buf) {
      case 'G':
      case '`':
	if (tty->param.par[0])
	  tty->param.par[0]--;
	gotoxy (tty, tty->param.par[0], tty->cur_y);
	continue;
      case 'A':
	if (!tty->param.par[0])
	  tty->param.par[0]++;
	gotoxy (tty, tty->cur_x, tty->cur_y - tty->param.par[0]);
	continue;
      case 'B':
      case 'e':
	if (!tty->param.par[0])
	  tty->param.par[0]++;
	gotoxy (tty, tty->cur_x, tty->cur_y + tty->param.par[0]);
	continue;
      case 'C':
      case 'a':
	if (!tty->param.par[0])
	  tty->param.par[0]++;
	gotoxy (tty, tty->cur_x + tty->param.par[0], tty->cur_y);
	continue;
      case 'D':
	if (!tty->param.par[0])
	  tty->param.par[0]++;
	gotoxy (tty, tty->cur_x - tty->param.par[0], tty->cur_y);
	continue;
      case 'E':
	if (!tty->param.par[0])
	  tty->param.par[0]++;
	gotoxy (tty, 0, tty->cur_y + tty->param.par[0]);
	continue;
      case 'F':
	if (!tty->param.par[0])
	  tty->param.par[0]++;
	gotoxy (tty, 0, tty->cur_y - tty->param.par[0]);
	continue;
      case 'd':
	if (tty->param.par[0])
	  tty->param.par[0]--;
	gotoxy (tty, tty->cur_x, tty->param.par[0]);
	continue;
      case 'H':
      case 'f':
	if (tty->param.par[0])
	  tty->param.par[0]--;
	if (tty->param.par[1])
	  tty->param.par[1]--;
	gotoxy (tty, tty->param.par[1], tty->param.par[0]);
	continue;
      case 'J':
	csi_J (tty, tty->param.par[0]);
	continue;
      case 'K':
	csi_K (tty, tty->param.par[0]);
	continue;
      case 'L':
	csi_L (tty, tty->param.par[0]);
	continue;
      case 'M':
	csi_M (tty, tty->param.par[0]);
	continue;
      case 'P':
	csi_P (tty, tty->param.par[0]);
	continue;
      case 'c':
	if (!tty->param.par[0])
	  respond_id (tty);
	continue;
      case 'g':
	if (!tty->param.par[0])
	  tty->tab_stop[tty->cur_x >> 5] &= ~(1 << (tty->cur_x & 31));
	else if (tty->param.par[0] == 3) {
	  tty->tab_stop[0] = tty->tab_stop[1] = tty->tab_stop[2] =
	    tty->tab_stop[3] = tty->tab_stop[4] = 0;
	}
	continue;
      case 'm':
	csi_m (tty);
	continue;
      case 'r':
	if (!tty->param.par[0])
	  tty->param.par[0]++;
	if (!tty->param.par[1])
	  tty->param.par[1] = NUM_OF_ROW;
	/* Minimum allowed region is 2 lines */
	if (tty->param.par[0] < tty->param.par[1] &&
	    tty->param.par[1] <= NUM_OF_ROW) {
	  tty->top = tty->param.par[0] - 1;
	  tty->bottom = tty->param.par[1];
	  gotoxy (tty, 0, 0);
	}
	continue;
      case 's':
	save_cur (tty);
	continue;
      case 'u':
	restore_cur (tty);
	continue;
      case 'X':
	csi_X (tty, tty->param.par[0]);
	continue;
      case '@':
	csi_at (tty, tty->param.par[0]);
	continue;
      case ']':		/* setterm functions */
	setterm_command (tty);
	continue;
      }
      continue;
    }
  }
  if (tty == hztty_list) {
    on_off_cursor (tty->cur_x, tty->cur_y);
/* screen_flush();*/

  }
}

void
hztty_redraw (void)
{
  if (hztty_list) {
    repaint_hztty ();
    on_off_cursor (hztty_list->cur_x, hztty_list->cur_y);
  }
}
