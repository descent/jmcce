#ifndef _JMCCE_DRAW_H_
#define _JMCCE_DRAW_H_

#include "config.h"
#include "ft2.h"

#ifdef VGALIB
#include <vga.h>
#include <vgagl.h>
extern GraphicsContext *physical_screen;
extern GraphicsContext *virtual_screen;
#endif

#define SCREEN_HEIGHT		(480)
#define SCREEN_WIDTH		(640)
#define LINE_SPACE		(2)
#define LINE_HEIGHT             (18)
#define FONT_HEIGHT             (16)
#define FONT_WIDTH              (16)
#define ASCII_FONT_WIDTH        (8)

/* color */
#define BLACK			(0)
#define BLUE			(1)
#define GREEN			(2)
#define CYAN			(3)
#define RED			(4)
#define MAGENTA			(5)
#define BROWN			(6) // or YELLOW
#define GRAY		        (7)

#define LIGHTBLACK      	(8)
#define LIGHTBLUE       	(9)
#define LIGHTGREEN      	(10)
#define LIGHTCYAN       	(11)
#define LIGHTRED        	(12)
#define LIGHTMAGENTA    	(13)
#define LIGHTBROWN      	(14)

#ifdef VGALIB
#define LIGHTWHITE      	(vga_white())
#else
#define LIGHTWHITE      	(255)
#endif

/*************** public functions *****************/

void clear_line ();
bool screen_init (void);
void screen_done (void);

void screen_flipaway (void);
void screen_return (void);

void screen_scroll_up (int bg_color);
void screen_scroll_down (int bg_color);
void draw_ascii_char (int x, int y, int c,
		      unsigned char fg_color, unsigned char bg_color,
		      unsigned char mode);
void draw_hanzi_char (int x, int y, unsigned char *bitmap,
		      unsigned char fg_color, unsigned char bg_color);

void print_string (int x, int y, unsigned char *string, int fg_color,
		   int bg_color);
void screen_clear_block (int x, int y, int w, int h, int color);
void screen_insert_line (int top, int bottom, int n, int bg_color);
void screen_delete_line (int top, int bottom, int n, int bg_color);

void on_off_cursor (int x, int y);

void my_draw_bitmap_mono(FT_Bitmap *bitmap,int pen_x,int pen_y, int fg_c, int bg_c);
void my_draw_bitmap_256(FT_Bitmap *bitmap,int pen_x,int pen_y);

#endif /* _JMCCE_DRAW_H_ */
