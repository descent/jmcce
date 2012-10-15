
/****************************************************************************/
/*            JMCCE  - c_routine.c                                          */
/*                                                                          */
/*                      $Id: c_routine.c,v 1.1.1.1 2002/05/03 04:01:07 kids Exp $                                              */
/****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include "asm_routine.h"	/* vga */
#include <string.h>		/* memmove */

#include "fb.h"

extern int use_fb;

unsigned char fgcolor = 15;
unsigned char bgcolor = 0;
int cursor_x0 = 0;		/* col */
int cursor_y0 = 16;		/* y */

void
clear_cursor ()
{
  int fgcolor0 = fgcolor;
  fgcolor = 0;
  fb_drawline (cursor_x0 * 8, cursor_y0 + 1, ((cursor_x0 + 1) * 8) - 1,
	       cursor_y0 + 1);
  fb_drawline (cursor_x0 * 8, cursor_y0, ((cursor_x0 + 1) * 8) - 1,
	       cursor_y0);
  fgcolor = fgcolor0;
}

/****************************************************************************
 *    void c_draw_ascii(int col,int y,unsigned char *bitmap,int color);     *
 *         col:      0-79     column value                                  * 
 *         y:        0-479    y coordinate                                  *
 *         bitmap:   18 bytes unsigned char buffer                          *
 *         color:    0-3 bit: ForeGround 8-11 bit: BackGround               *
 ****************************************************************************/

void vgalib_draw_ascii(int col, int y, unsigned char *bitmap, int color1)
{
  int i=0;
  int cx=0, cy=0;
  extern unsigned char ascii_font[256][18];
  unsigned char *ascii = ascii_font['A'];


  vga_setcolor(color1);
  for (i=0 ; i < 18 ; ++i)
  {
    char c = bitmap[i];
    int j=0;
    //printf("%x ", ascii[i]);

    for (j=7 ; j>=0 ; --j)
    {           
      if (((c >> j) & 0x1) == 1)
        vga_drawpixel(col+cx, y+cy);
        //printf("*");
      //else
        //printf("_");
      ++cx;
    }
    cx=0;
    ++cy;

  }
  //printf("\n");

}

void
c_draw_ascii (int col, int y, unsigned char *bitmap, int color1)
{
  BYTE temp0, temp1;
  int j = 0;
  int m, k, startx, starty, fgcolor0, fgcolor1, bgcolor1;
  int fontheight = 18;

  if (!use_fb) {
    asm_draw_ascii (col, y, bitmap, color1);
    return;
  }

  startx = col * 8;
  starty = y;

  fgcolor0 = fgcolor;
  fgcolor1 = (color1 & 0xf);
  bgcolor1 = color1 >> 8;

  for (m = 0; m < fontheight; m++) {
    temp0 = bitmap[j];
    for (k = 0; k < 8; k++) {
      temp1 = temp0 & 128;
      if (temp1 != 0) {
	fgcolor = fgcolor1;
	fb_drawpixel (k + startx, m + starty);
      } else {
	fgcolor = bgcolor1;
	fb_drawpixel (k + startx, m + starty);
      }

      temp0 <<= 1;
    }
    j++;
  }
  fgcolor = fgcolor0;
}

/****************************************************************************
 *    void c_draw_hanzi(int col,int y,unsigned char *bitmap,int color);     *
 *         col:      0-79     column value                                  *
 *         y:        0-479    y coordinate                                  *
 *         bitmap:   36 bytes unsigned char buffer                          *
 *         color:    0-3 bit: ForeGround 8-11 bit: BackGround               *
 ****************************************************************************/
void
c_draw_hanzi (int col, int y, unsigned char *bitmap, int color1)
{
  BYTE temp0, temp1;
  int j = 0;
  int l, m, k, startx, starty, fgcolor0, fgcolor1, bgcolor1;
  int fontheight = 18;

  if (!use_fb) {
    asm_draw_hanzi (col, y, bitmap, color1);
    return;
  }

  startx = col * 8;
  starty = y;

  fgcolor0 = fgcolor;
  fgcolor1 = (color1 & 0xf);
  bgcolor1 = color1 >> 8;

  for (m = 0; m < fontheight; m++) {
    for (l = 0; l < 2; l++) {
      temp0 = bitmap[j];
      for (k = 0; k < 8; k++) {
	temp1 = temp0 & 128;
	if (temp1 != 0) {
	  fgcolor = fgcolor1;
	  fb_drawpixel (k + l * 8 + startx, m + starty);
	} else {
	  fgcolor = bgcolor1;
	  fb_drawpixel (k + l * 8 + startx, m + starty);
	}
	temp0 <<= 1;
      }
      j++;
    }
  }
  fgcolor = fgcolor0;
}

/****************************************************************************
 *        void c_scroll_up(int sy,int ey,int line,int bgcolor);             *
 *             sy:       0-479    start y coordinate                        *
 *             ey:       0-479    end y coordinate                          *
 *                       (scroll area include sy & ey)                      *
 *             line:     scroll up lines                                    *
 *             bgcolor:  0-3 bit: BackGround Color                          *
 ****************************************************************************/
void
c_scroll_up (int sy, int ey, int line, int bgcolor1)
{
  unsigned char *dest2, *dest, *src;
  int bsize;

#if 0				/* 0,431,18,0 */
  printf ("c_scroll_up(%d,%d,%d,%d)\n", sy, ey, line, bgcolor1);
#endif

  if (!use_fb) {
    asm_scroll_up (sy, ey, line, bgcolor1);
    return;
  }

  dest = addr2 + sy * (vinfo.xres * vinfo.bits_per_pixel / 8);
  src = addr2 + (line + sy) * (vinfo.xres * vinfo.bits_per_pixel / 8);
  bsize = (ey - sy - line) * (vinfo.xres * vinfo.bits_per_pixel / 8);
  dest2 = dest + bsize;

  clear_cursor ();
  memmove (dest, src, bsize);
  memset (dest2, bgcolor1, line * (vinfo.xres * vinfo.bits_per_pixel / 8));
}

/****************************************************************************
 *       void c_scroll_down(int sy,int ey,int line,int bgcolor);            *
 *             sy:       0-479    start y coordinate                        *
 *             ey:       0-479    end y coordinate                          *
 *                       (scroll area include sy & ey)                      *
 *             line:     scroll down lines                                  *
 *             bgcolor:  0-3 bit: BackGround Color                          *
 ****************************************************************************/
void
c_scroll_down (int sy, int ey, int line, int bgcolor1)
{
  unsigned char *dest2, *dest, *src;
  int bsize;

#if 0				/* 0,413,18,0 */
  printf ("c_scroll_down(%d,%d,%d,%d)\n", sy, ey, line, bgcolor1);
#endif

  if (!use_fb) {
    asm_scroll_down (sy, ey, line, bgcolor1);
    return;
  }

  src = addr2 + sy * (vinfo.xres * vinfo.bits_per_pixel / 8);
  dest = addr2 + (sy + line) * (vinfo.xres * vinfo.bits_per_pixel / 8);
  bsize = (ey - sy - line) * (vinfo.xres * vinfo.bits_per_pixel / 8);
  dest2 = src;

  clear_cursor ();
  memmove (dest, src, bsize);
  memset (dest2, bgcolor1, line * (vinfo.xres * vinfo.bits_per_pixel / 8));
}

/****************************************************************************
 *  void c_clear_block(int col,int y,int width,int height,int bgcolor);     *
 *         col:      0-79  left column value                                *
 *         y:        9-479 upper y coordinate                               *
 *         width:    1-80  clear block width(in columns)                    *
 *         height:   1-480 clear block height(in scanlines)                 *
 *         bgcolor:  0-3 bit: BackGround Color                              *
 ****************************************************************************/
void
c_clear_block (int col, int y, int width, int height, int bgcolor1)
{
  if (!use_fb) {
    asm_clear_block (col, y, width, height, bgcolor1);
    return;
  }

  bgcolor = bgcolor1;
  fb_clearblock (col * 8, y, col * 8 + width * 8, y + height);
}

/****************************************************************************
 *           void c_clear_lines(int sy,int height,int bgcolor);             *
 *              sy:       0-479 upper y coordinate                          *
 *              height:   1-480 clear block height(in scanlines)            *
 *              bgcolor:  0-3 bit: BackGround Color                         * 
 ****************************************************************************/
void
c_clear_lines (int sy, int height, int bgcolor1)
{
  int bgcolor0 = bgcolor;

  if (!use_fb) {
    asm_clear_lines (sy, height, bgcolor1);
    return;
  }

  bgcolor = bgcolor1;
  fb_clearblock (0, sy, 639, sy + height);
  bgcolor = bgcolor0;
}

/***************************************************************************
 *               void c_bold_hanzi(unsigned char *buffer);                 *
 *                  buffer:  32 bytes hanzi bitmap buffer                  *
 ***************************************************************************/
void
c_bold_hanzi (unsigned char *buffer)
{
  printf ("c_bold_hanzi()\n");
}

/***************************************************************************
 *               void c_bold_ascii(unsigned char *buffer);                 *
 *                  buffer:  16 bytes hanzi bitmap buffer                  *
 ***************************************************************************/
void
c_bold_ascii (unsigned char *buffer)
{
  printf ("c_bold_ascii()\n");
}

/***************************************************************************
 *               void c_toggle_cursor(int col,int y);                      *
 *                       col:      0-79  column value                      *
 *                         y:      0-479 y coordinate                      * 
 ***************************************************************************/
void
c_toggle_cursor (int col, int y)
{
  if (!use_fb) {
    asm_toggle_cursor (col, y);
    return;
  }

  clear_cursor ();
  fb_drawline (col * 8, y + 1, ((col + 1) * 8) - 1, y + 1);
  fb_drawline (col * 8, y, ((col + 1) * 8) - 1, y);
  cursor_x0 = col;
  cursor_y0 = y;
}
