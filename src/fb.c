
/****************************************************************************/
/*            JMCCE  - fb.c                                                 */
/*                                                                          */
/*                      $Id: fb.c,v 1.1.1.1 2002/05/03 04:01:07 kids Exp $      */
/****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */


#include <stdio.h>
#include <stdlib.h>
#include <string.h>		/* memmove */

#include "fb.h"

struct fb_var_screeninfo vinfo;
void *addr;
unsigned char *addr2;
struct fb_fix_screeninfo finfo;
size_t vsize;
int fh;
int cursor_x, cursor_y;

/*for 256 color*/

int
fb_drawpixel (int x, int y)
{				/* ok */
  addr2[vinfo.xres * y + x] = fgcolor;
  return 0;
}

int
fb_drawline (int x1, int y1, int x2, int y2)
{				/* ok */
  int dx = x2 - x1;
  int dy = y2 - y1;
  int ax = abs (dx) << 1;
  int ay = abs (dy) << 1;
  int sx = (dx >= 0) ? 1 : -1;
  int sy = (dy >= 0) ? 1 : -1;

  int x = x1;
  int y = y1;

  if (ax > ay) {
    int d = ay - (ax >> 1);
    while (x != x2) {
      fb_drawpixel (x, y);

      if ((d > 0) || ((d == 0) && (sx == 1))) {
	y += sy;
	d -= ax;
      }
      x += sx;
      d += ay;
    }
  } else {
    int d = ax - (ay >> 1);
    while (y != y2) {
      fb_drawpixel (x, y);

      if ((d > 0) || ((d == 0) && (sy == 1))) {
	x += sx;
	d -= ay;
      }
      y += sy;
      d += ax;
    }
  }
  fb_drawpixel (x, y);

  return 0;
}

void
fb_clearblock (int sx, int sy, int ex, int ey)
{				/* ok */
  int i;

  if (ey > (vinfo.yres - 1))
    ey = (vinfo.yres - 1);	/* jmt */

  for (i = sy; i <= ey; i++)
    memset (addr2 + (i * vinfo.xres + sx) * vinfo.bits_per_pixel / 8,
	    bgcolor, (ex - sx) * vinfo.bits_per_pixel / 8);
}

void
fb_setfgcolor (int c)
{				/* 256:16 */
  fgcolor = c;
}

void
fb_setbgcolor (int c)
{
  bgcolor = c;
}

int
fb_init ()
{
  int bits;

  cursor_x = cursor_y = 0;

  if ((fh = open (FB_DEV, O_RDWR)) < 0) {
    perror ("open() " FB_DEV);
    return -1;
  }

  if (ioctl (fh, FBIOGET_FSCREENINFO, &finfo)) {
    perror ("ioctl()" FB_DEV);
    close (fh);
    return -1;
  }

  ioctl (fh, FBIOGET_VSCREENINFO, &vinfo);

  printf ("fb_init: using %ix%i of %ix%i pixels, %i bits per pixel\r\n",
	  vinfo.xres, vinfo.yres,
	  vinfo.xres_virtual, vinfo.yres_virtual, vinfo.bits_per_pixel);
  if (vinfo.bits_per_pixel != 8) {
    vinfo.bits_per_pixel = 8;
    if (ioctl (fh, FBIOPUT_VSCREENINFO, &vinfo)) {
      perror ("FB change mode for depth 8");
      printf
	("jmcce require depth 8 for Frame Buffer. Use vga mode instead.\n");
      close (fh);
      return -1;
      /* exit(1); */
    }
  }

  addr = mmap (0, vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8,
	       PROT_READ | PROT_WRITE, MAP_SHARED, fh, 0);

  if (addr < 0) {
    perror ("mmap() " FB_DEV);
    close (fh);
    return -1;
  }

  bits = (finfo.type == FB_TYPE_PLANES ? 1 : vinfo.bits_per_pixel);
  vsize = (vinfo.xres * bits >> 3) * vinfo.yres;

  printf ("fb_init: %ik videoram mapped to 0x%p\r\n", vsize >> 10, addr);

  addr2 = (unsigned char *)addr;
  close (fh);

  return 0;
}
