
/****************************************************************************/
/*            JMCCE  - draw.c                                               */
/*                                                                          */
/*                      $Id: draw.c,v 1.1.1.1 2002/05/03 04:01:07 kids Exp $    */
/****************************************************************************/

#include <stdio.h>
#include <sys/types.h>
#include <string.h>
//#include <asm/bitops.h>
#include <signal.h>
#include <iostream>

using namespace std;

#include "jmcce.h"
#include "error.h"
#include "hbffont.h"
#include "draw.h"
#include "hztty.h"
#include "fb.h"
#include "c_routine.h"
#include "draw.h"

int graphic_mode = 1;
const char *c_fg = "*";
const char *c_bg = "|";

extern int gFont_bytes;
extern FILE *fs;

#ifdef VGALIB
GraphicsContext *physical_screen;
GraphicsContext *virtual_screen;
#endif

#ifdef LINUXFB
int use_fb = 1;
Fb fb;
#else
int use_fb = 0;
#endif

void
screen_delete_line (int top, int bottom, int n, int bg_color)
{

  if (!active_console)
    return;
  if (top >= bottom)
    return;
  if (n > (bottom - top))
    n = bottom - top;

#ifdef DEBUG_
  fprintf(fs, "top: %d\n", top);
  fprintf(fs, "bottom: %d\n", bottom);
  fprintf(fs, "n: %d\n", n);
#endif

#if 1
  if (!use_fb) 
  {
    void vgalib_scroll_up(int sy,int ey,int line,int bgcolor);

    vgalib_scroll_up (top, bottom-1, n, bg_color);
  }
  else
#endif
    c_scroll_up (top * LINE_HEIGHT, bottom * LINE_HEIGHT - 1, n * LINE_HEIGHT, bg_color);
}

void screen_insert_line (int top, int bottom, int n, int bg_color)
{
  fprintf(fs, "xx top: %d\n", top);
  fprintf(fs, "xx bottom: %d\n", bottom);
  fprintf(fs, "xx n: %d\n", n);

  if (!active_console)
    return;
  if (top >= bottom)
    return;
  if (n > (bottom - top))
    n = bottom - top;
  void vgalib_scroll_down(int sy,int ey,int line,int bgcolor);

  if (!use_fb) 
    vgalib_scroll_down(top, bottom - 1, n, bg_color);
  else
    c_scroll_down (top * LINE_HEIGHT, bottom * LINE_HEIGHT - 1, n * LINE_HEIGHT, bg_color); 
}

void screen_scroll_up (int bg_color)
{

  if (!active_console)
    return;
  if (!use_fb) 
  {
    void vgalib_scroll_up(int sy,int ey,int line,int bgcolor);

    vgalib_scroll_up (0, NUM_OF_ROW - 1, 1, bg_color);
  }
  else
    c_scroll_up (0, LINE_HEIGHT * NUM_OF_ROW - 1, LINE_HEIGHT, bg_color);
}

void
screen_scroll_down (int bg_color)
{
  void vgalib_scroll_down(int sy,int ey,int line,int bgcolor);

  if (!active_console)
    return;
  fprintf(fs, "yy top: %d\n", 0);
  fprintf(fs, "yy bottom: %d\n", NUM_OF_ROW);
  fprintf(fs, "yy n: %d\n", LINE_HEIGHT);
  if (!use_fb) 
    vgalib_scroll_down(0, NUM_OF_ROW - 1, 1, bg_color);
  else
    c_scroll_down (0, LINE_HEIGHT * NUM_OF_ROW - 1, LINE_HEIGHT, bg_color);
}

void screen_clear_block (int x, int y, int w, int h, int color)
{

  if (!active_console)
    return;
  c_clear_block (x, y * LINE_HEIGHT, w, h * LINE_HEIGHT, color);
}

void
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
#if 0
  if (!use_fb)
    vga_setmode (TEXT);
#endif
  screen_done ();
  active_console = 0;
}

//#define DEFAULT_VGA_MODE G640x480x16M
#define DEFAULT_VGA_MODE G640x480x256 // in my fujitsu lifebook ati card, it works

/* use by signal usr2 (SIGACQ) handler -- acquire terminal */
void
screen_return (void)
{
#ifdef VGALIB
  if (!use_fb)
    vga_setmode (DEFAULT_VGA_MODE);
#endif
  active_console = 1;
}

bool screen_init (void)
{
#ifdef LINUXFB
  use_fb = 1;
  if (fb_init ()) 
  {
    use_fb = 0;
    return false;
  }
#endif

#ifdef VGALIB
  use_fb = 0;
  vga_init ();
  vga_setmode(DEFAULT_VGA_MODE);
  gl_setcontextvga(DEFAULT_VGA_MODE);
  physical_screen = gl_allocatecontext();
  gl_getcontext(physical_screen);

  gl_setcontextvgavirtual(DEFAULT_VGA_MODE);
  virtual_screen = gl_allocatecontext();
  gl_getcontext(virtual_screen);

  gl_setcontext(virtual_screen);
  //vga_ext_set(VGA_EXT_SET,  VGA_CLUT8);

// color table:
// http://en.wikipedia.org/wiki/ANSI_escape_code#Colors 

  gl_setpalettecolor(BLUE, 0, 0, 63); // blue
  gl_setpalettecolor(BLACK, 0, 0, 0); // black
  gl_setpalettecolor(GREEN, 0, 63, 0); 
  gl_setpalettecolor(RED, 63, 0, 0); 
  gl_setpalettecolor(BROWN, 170/4, 85/4, 0);
  gl_setpalettecolor(MAGENTA, 170/4, 0, 170/4);
  gl_setpalettecolor(CYAN, 0, 170/4, 170/4); 
  gl_setpalettecolor(GRAY, 48, 48, 48);

  gl_setpalettecolor(LIGHTBLACK, 85/4, 85/4, 85/4); 
  gl_setpalettecolor(LIGHTBLUE, 85/4, 85/4, 255/4); 
  gl_setpalettecolor(LIGHTGREEN, 85/4, 255/4, 85/4); 
  gl_setpalettecolor(LIGHTCYAN, 85/4, 255/4, 255/4); 
  gl_setpalettecolor(LIGHTRED, 25/45/4, 85/4, 85/4); 
  gl_setpalettecolor(LIGHTMAGENTA, 255/4, 85/4, 255/4); 
  gl_setpalettecolor(LIGHTBROWN, 255/4, 255/4, 85/4); 

#endif

  active_console = 1;
  return true;
}

void
screen_done (void)
{
#ifdef VGALIB
  if (!use_fb)
    vga_setmode (TEXT);
#endif
}


void
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

  if (!use_fb)
    vgalib_draw_ascii (x, y * LINE_HEIGHT, buffer, fg_color, (bg_color));
  else
    c_draw_ascii (x, y * LINE_HEIGHT, buffer, fg_color | (bg_color << 8));
}

void
draw_hanzi_char (int x, int y, unsigned char *bitmap,
		 unsigned char fg_color, unsigned char bg_color)
{

  if (!active_console)
    return;
  if (!use_fb)
    vgalib_draw_hanzi (x, y * LINE_HEIGHT, bitmap, fg_color, bg_color);
  else
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

void on_off_cursor (int x, int y)
{

  if (!active_console)
    return;
  c_toggle_cursor (x, y * LINE_HEIGHT + FONT_HEIGHT);
}


void my_draw_bitmap_mono(FT_Bitmap *bitmap,int pen_x,int pen_y, int fg_c, int bg_c)
{
  uint8_t r=0, g=170, b=0;

  cout << "bitmap rows : " << bitmap->rows << endl;
  cout << "bitmap width : " << bitmap->width << endl;
  cout << "bitmap pitch : " << bitmap->pitch << endl;
  if (bitmap->pixel_mode==ft_pixel_mode_mono)
    cout << "bitmap pixel mode : mono" << endl;
   if (bitmap->pixel_mode==ft_pixel_mode_grays)
   {
    cout << "bitmap pixel mode : grays" << endl;
    cout << "bitmap grays level : " << bitmap->num_grays << endl;
   }

  int startx = pen_x;
  int starty = pen_y;
  int cx=0, cy=0;

  unsigned char *tmp = bitmap->buffer;
  for (int i=0 ; i < bitmap->rows ; i++)
  {
    unsigned char c = *tmp;
    
    int font_w = 0;
    for (int j=0 ; j < bitmap->pitch ; j++)
    {
      #if 0
      printf("%x, ", ch); 
      ++tmp;
      ch = *tmp;
      #endif



      for (int i=7 ; i>=0 ; --i)
      {
        if (font_w < bitmap->width)
        {

        if (((c >> i) & 0x1) == 1)
        {
          if (graphic_mode == '0')
            printf(c_fg);
#ifdef SVGALIB
          else
            gl_setpixel(startx+cx, starty+cy, vga_white());
#endif
#ifdef LINUXFB
          else
          {
            color2rgb(fg_c, r, g, b);
            // color2rgb(BLUE, r, g, b);
            fb.setpixelrgb(startx+cx, starty+cy, r, g, b);
          }
#endif
        }
        else
        {
          if (graphic_mode == '0')
            printf(c_bg);
#ifdef SVGALIB
          else
            gl_setpixelrgb(startx+cx, starty+cy, 180, 0, 0);
#endif
#ifdef LINUXFB
          else
          {
            color2rgb(bg_c, r, g, b);
            fb.setpixelrgb(startx+cx, starty+cy, r, g, b);
          }
#endif

        }

        }
        //++cur_x;
        ++cx;
        ++font_w;
      }
      ++tmp;
      c = *tmp;

    }
    cx=0;
    ++cy;
    printf("\n");

    #if 0
    for (int j=0 ; j < bitmap->width ; j++)
    {
      for (int k=7 ; k >= 0 ; --k)
      {
      }
    }
    ++tmp;
    #endif
  }
}

void my_draw_bitmap_256(FT_Bitmap *bitmap,int pen_x,int pen_y)
{
  cout << "bitmap rows : " << bitmap->rows << endl;
  cout << "bitmap width : " << bitmap->width << endl;
  cout << "bitmap pitch : " << bitmap->pitch << endl;
  if (bitmap->pixel_mode==ft_pixel_mode_mono)
    cout << "bitmap pixel mode : mono" << endl;
   if (bitmap->pixel_mode==ft_pixel_mode_grays)
   {
    cout << "bitmap pixel mode : grays" << endl;
    cout << "bitmap grays level : " << bitmap->num_grays << endl;
   }
  if (graphic_mode == '0')
  {
    cout << "anti-alias mode only support graphic mode" << endl;
    return;
  }

#ifdef SVGALIB
  int startx = pen_x;
  int starty = pen_y;
  int cx=0, cy=0;
  unsigned char *tmp = bitmap->buffer;
  for (int i=0 ; i < bitmap->rows ; i++)
  {
    unsigned char c = *tmp;
    
    for (int j=0 ; j < bitmap->pitch ; j++)
    {
      //gl_setpixel(startx+cx, starty+cy, c);
      if (c==0)
        gl_setpixelrgb(startx+cx, starty+cy, 0, 120, 0);
      else
        gl_setpixelrgb(startx+cx, starty+cy, c, 0, 0);
      printf("%x, ", c); 
      ++tmp;
      c = *tmp;
      ++cx;
    }
    cx=0;
    ++cy;
    //usleep(50000);
  }
#endif
}
