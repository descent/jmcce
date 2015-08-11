
/****************************************************************************/
/*            JMCCE  - fb.c                                                 */
/*                                                                          */
/*                      $Id: fb.c,v 1.1.1.1 2002/05/03 04:01:07 kids Exp $      */
/****************************************************************************/

#include <cstdio>
#include <cstdlib>
#include <cstring>		/* memmove */

#include "fb.h"
#include "draw.h"

struct fb_var_screeninfo vinfo;
void *addr;
unsigned char *addr2;
struct fb_fix_screeninfo finfo;
size_t vsize;
int fh;
int cursor_x, cursor_y;

Fb::Fb()
{
  int ret = fb_init();
}

Fb::~Fb()
{
  munmap(fbp, screensize);
  close(fbfd);
  printf("munmap\n");
}

int Fb::fb_init()
{
   int x = 0, y = 0;

   // Open the file for reading and writing
   fbfd = open("/dev/fb0", O_RDWR);
   if (fbfd == -1) {
       perror("Error: cannot open framebuffer device");
       exit(1);
   }
   printf("The framebuffer device was opened successfully.\n");

   // Get fixed screen information
   if (ioctl(fbfd, FBIOGET_FSCREENINFO, &finfo) == -1) {
       perror("Error reading fixed information");
       exit(2);
   }

   // Get variable screen information
   if (ioctl(fbfd, FBIOGET_VSCREENINFO, &vinfo) == -1) {
       perror("Error reading variable information");
       exit(3);
   }

   printf("%dx%d, %dbpp\n", vinfo.xres, vinfo.yres, vinfo.bits_per_pixel);

   // Figure out the size of the screen in bytes
   screensize = vinfo.xres * vinfo.yres * vinfo.bits_per_pixel / 8;

   // Map the device to memory
   fbp = (char *)mmap(0, screensize, PROT_READ | PROT_WRITE, MAP_SHARED,
                      fbfd, 0);
   if (fbp == (void *)-1) {
       perror("Error: failed to map framebuffer device to memory");
       exit(4);
   }
   printf("The framebuffer device was mapped to memory successfully.\n");
   addr = fbp;
   addr2 = (unsigned char *)addr;
   ::vinfo = vinfo;
   return 0;
}

void Fb::print_finfo()
{
  printf("finfo.line_length: %d\n", finfo.line_length);
}

void Fb::print_vinfo()
{
  printf("vinfo.bits_per_pixel: %d\n", vinfo.bits_per_pixel);
  printf("vinfo.xoffset: %d\n", vinfo.xoffset);
  printf("vinfo.yoffset: %d\n", vinfo.yoffset);
  printf("vinfo.xres: %d\n", vinfo.xres);
  printf("vinfo.yres: %d\n", vinfo.yres);
  // printf("vinfo.bits_per_pixel: %d\n", vinfo.bits_per_pixel);
}

void Fb::scroll_up (int sy, int ey, int line, int bg_c)
{
  unsigned char *dest2, *dest, *src;
  unsigned int bsize;

  printf("sy: %d\n", sy);
  printf("ey: %d\n", ey);
  printf("line: %d\n", line);

  dest = fbp + sy * (vinfo.xres * vinfo.bits_per_pixel / 8);
  src = fbp + (line + sy) * (vinfo.xres * vinfo.bits_per_pixel / 8);
  bsize = (ey - sy - line) * (vinfo.xres * vinfo.bits_per_pixel / 8);
  dest2 = dest + bsize;

  memmove (dest, src, bsize);
  memset (dest2, bg_c, line * (vinfo.xres * vinfo.bits_per_pixel / 8));
}

void Fb::setpixelrgb(int x, int y, int r, int g, int b)
{
  long int location = 0;

  // location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * finfo.line_length;
  //int byte_per_pixel = vinfo.bits_per_pixel/8;

  //location = (y * vinfo.xres + x) * (finfo.line_length/vinfo.xres);
  location = y * finfo.line_length + x * (vinfo.bits_per_pixel/8);
        
  printf("x: %d, y: %d\n", x, y);
  printf("location: %d\n", location);
  printf("vinfo.xres: %d\n", vinfo.xres);
  printf("finfo.line_length: %d\n", finfo.line_length);

  switch (vinfo.bits_per_pixel)
  {
    case 16:
    {
    #if 0
      int b = 10;
      int g = (x-100)/6;     // A little green
      int r = 31-(y-100)/16;    // A lot of red
    #endif
      unsigned short int t = r<<11 | g << 5 | b;
      *((unsigned short int*)(fbp + location)) = t;
      break;
    }
    case 32:
    {
      *(fbp + location) = b;        // Some blue
      *(fbp + location + 1) = g;     // A little green
      *(fbp + location + 2) = r;    // A lot of red
      *(fbp + location + 3) = 0;      // No transparency
      break;
    }
  }
}

void LFB::color2rgb(uint8_t color, uint8_t &r, uint8_t &g, uint8_t &b)
{
#ifdef LINUXFB
  switch (color)
  {
    case BLACK:
    {
      r = 0;
      g = 0;
      b = 0;
      break;
    }
    case BLUE:
    {
      r = 0;
      g = 0;
      b = 170;
      break;
    }
    case GREEN:
    {
      r = 0;
      g = 170;
      b = 0;
      break;
    }
    case CYAN:
    {
      r = 0;
      g = 170;
      b = 170;
      break;
    }
    case RED:
    {
      r = 170;
      g = 0;
      b = 0;
      break;
    }
    case MAGENTA:
    {
      r = 170;
      g = 0;
      b = 170;
      break;
    }
    case BROWN:
    {
      r = 170;
      g = 85;
      b = 0;
      break;
    }
    case GRAY:
    {
      r = 170;
      g = 170;
      b = 170;
      break;
    }
    case LIGHTBLACK:
    {
      r = 85;
      g = 85;
      b = 85;
      break;
    }
    case LIGHTBLUE:
    {
      r = 85;
      g = 85;
      b = 255;
      break;
    }
    case LIGHTGREEN:
    {
      r = 85;
      g = 255;
      b = 85;
      break;
    }
    case LIGHTCYAN:
    {
      r = 85;
      g = 255;
      b = 255;
      break;
    }
    case LIGHTRED:
    {
      r = 255;
      g = 85;
      b = 85;
      break;
    }
    case LIGHTMAGENTA:
    {
      r = 255;
      g = 85;
      b = 255;
      break;
    }
    case LIGHTBROWN:
    {
      r = 255;
      g = 255;
      b = 85;
      break;
    }
    case LIGHTWHITE:
    {
      r = 255;
      g = 255;
      b = 255;
      break;
    }
  }
#endif
}

void color2rgb(u8 color, u8 &r, u8 &g, u8 &b)
{
#ifdef LINUXFB
  switch (color)
  {
    case BLACK:
    {
      r = 0;
      g = 0;
      b = 0;
      break;
    }
    case BLUE:
    {
      r = 0;
      g = 0;
      b = 170;
      break;
    }
    case GREEN:
    {
      r = 0;
      g = 170;
      b = 0;
      break;
    }
    case CYAN:
    {
      r = 0;
      g = 170;
      b = 170;
      break;
    }
    case RED:
    {
      r = 170;
      g = 0;
      b = 0;
      break;
    }
    case MAGENTA:
    {
      r = 170;
      g = 0;
      b = 170;
      break;
    }
    case BROWN:
    {
      r = 170;
      g = 85;
      b = 0;
      break;
    }
    case GRAY:
    {
      r = 170;
      g = 170;
      b = 170;
      break;
    }
    case LIGHTBLACK:
    {
      r = 85;
      g = 85;
      b = 85;
      break;
    }
    case LIGHTBLUE:
    {
      r = 85;
      g = 85;
      b = 255;
      break;
    }
    case LIGHTGREEN:
    {
      r = 85;
      g = 255;
      b = 85;
      break;
    }
    case LIGHTCYAN:
    {
      r = 85;
      g = 255;
      b = 255;
      break;
    }
    case LIGHTRED:
    {
      r = 255;
      g = 85;
      b = 85;
      break;
    }
    case LIGHTMAGENTA:
    {
      r = 255;
      g = 85;
      b = 255;
      break;
    }
    case LIGHTBROWN:
    {
      r = 255;
      g = 255;
      b = 85;
      break;
    }
    case LIGHTWHITE:
    {
      r = 255;
      g = 255;
      b = 255;
      break;
    }
  }
#endif
}

/*for 256 color*/

int fb_drawpixel (int x, int y, int color)
{				/* ok */
  u8 r, g, b;

  color2rgb(color, r, g, b);
  return fb_drawpixel (x, y, r, g, b);
}

int fb_drawpixel (int x, int y, char r, char g, char b)
{				/* ok */
  static Fb fb;

  fb.setpixelrgb(x, y, r, g, b);

#if 0
//  addr2[(vinfo.xres * y + x)*2] = color;
//  addr2[(vinfo.xres * y + x)*2+1] = color;

  // printf("vinfo.bits_per_pixel: %d\n", vinfo.bits_per_pixel);
  switch (vinfo.bits_per_pixel)
  {
    case 8:
    {
      addr2[vinfo.xres * y + x] = fgcolor;
      break;
    }
    case 16:
    {

#if 0
                 int b = 10;
                 int g = (x-100)/6;     // A little green
                 int r = 31-(y-100)/16;    // A lot of red
                 unsigned short int t = r<<11 | g << 5 | b;
                 *((unsigned short int*)(fbp + location)) = t;
#endif
      //fb_drawpixel (k + startx, m + starty, 0xff);
      break;
    }
    case 24:
    {
      u8 r_, g_, b_;

      color2rgb(fgcolor, r_, g_, b_);
      addr2[(vinfo.xres * y + x)*3] = b_;
      addr2[(vinfo.xres * y + x)*3+1] = g_;
      addr2[(vinfo.xres * y + x)*3+2] = r_;
      break;
    }
    case 32:
    {
      u8 r_, g_, b_;
      long int location = 0;

#if 1
       location = (x+vinfo.xoffset) * (vinfo.bits_per_pixel/8) + (y+vinfo.yoffset) * finfo.line_length;

      color2rgb(fgcolor, r_, g_, b_);
  *(addr2 + location) = 170;        // Some blue
  *(addr2 + location + 1) = 0;     // A little green
  *(addr2 + location + 2) = 0;    // A lot of red
  *(addr2 + location + 3) = 0;      // No transparency

#else
      addr2[(vinfo.xres * y + x)*4] = b_;
      addr2[(vinfo.xres * y + x)*4+1] = g_;
      addr2[(vinfo.xres * y + x)*4+2] = r_;
      addr2[(vinfo.xres * y + x)*4+3] = 0;
#endif
      break;
    }
  }
#endif
  return 0;
}

int fb_drawline (int x1, int y1, int x2, int y2, int color)
{		
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
      fb_drawpixel (x, y, color);

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
      fb_drawpixel (x, y, color);

      if ((d > 0) || ((d == 0) && (sy == 1))) {
	x += sx;
	d -= ay;
      }
      y += sy;
      d += ax;
    }
  }
  fb_drawpixel (x, y, color);

  return 0;
}


void fb_clearblock (int sx, int sy, int ex, int ey, u8 bg)
{

#if 1
  for (int i = sy ; i < ey ; ++i)
  {
    // extern FILE *fs;
    // fprintf(fs, "sx: %d, i: %d, ex: %d, i: %d\n", sx, i, ex, i);
    fb_drawline(sx, i, ex, i, bg);
  }
#endif
}

void fb_clearblock (int sx, int sy, int ex, int ey)
{				/* ok */
  if (ey > (vinfo.yres - 1))
    ey = (vinfo.yres - 1);	/* jmt */

#if 0
  for (int i = sy ; i < ey ; ++i)
    fb_drawline(sx, i, sy, i);

#else
  int i;

  for (i = sy; i <= ey; i++)
    memset (addr2 + (i * vinfo.xres + sx) * vinfo.bits_per_pixel / 8,
	    bgcolor, (ex - sx) * vinfo.bits_per_pixel / 8);
#endif
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

int fb_init ()
{
#if 0
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

  switch ( vinfo.bits_per_pixel )
  {
    case 8:
    {
      printf("8 bit color\n");
      break;
    }
    case 16:
    {
      printf("don't support 16 bit color # 0565\n");
      return -1;
      break;
    }
    case 24:
    {
      printf("24 bit color\n");
      break;
    }
    case 32:
    {
      printf("32 bit color\n");
      break;
    }
    default:
      printf ("Framebuffer format not supported\n");
  }


#if 0
  if (vinfo.bits_per_pixel != 8) 
  {
    return -1;

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
#endif

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
#endif
  return 0;
}
