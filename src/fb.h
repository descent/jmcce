#ifndef _JMCCE_FB_H_
#define _JMCCE_FB_H_

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <signal.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <linux/fb.h>
#include <linux/vt.h>
#include <linux/kd.h>

#include "type.h"

#define FB_DEV  "/dev/fb0"
typedef unsigned char BYTE;
extern size_t vsize;
extern int fh;
extern unsigned char fgcolor;
extern unsigned char bgcolor;

extern struct fb_var_screeninfo vinfo;
extern void *addr;
extern unsigned char *addr2;

int fb_init ();
void fb_clearblock (int sx, int sy, int ex, int ey);
void fb_clearblock (int sx, int sy, int ex, int ey, u8 bg);
void fb_setfgcolor (int);
void fb_setbgcolor (int);
int fb_drawline (int x1, int y1, int x2, int y2);	/* ok */

int fb_drawpixel (int x, int y);	/* ok */

int fb_drawpixel (int x, int y, char r, char g, char b);
void color2rgb(u8 color, u8 &r, u8 &g, u8 &b);


#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <linux/fb.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <stdint.h>

#define LINUXFB 1

#define BLACK                   (0)
#define BLUE                    (1)
#define GREEN                   (2)
#define CYAN                    (3)
#define RED                     (4)
#define MAGENTA                 (5)
#define BROWN                   (6) // or YELLOW
#define GRAY                    (7)

#define LIGHTBLACK              (8)
#define LIGHTBLUE               (9)
#define LIGHTGREEN              (10)
#define LIGHTCYAN               (11)
#define LIGHTRED                (12)
#define LIGHTMAGENTA            (13)
#define LIGHTBROWN              (14)
#define LIGHTWHITE              (255)

namespace LFB
{
  void color2rgb(uint8_t color, uint8_t &r, uint8_t &g, uint8_t &b);
}

class Fb
{
  public:
    Fb();
    ~Fb();
    int fb_init();
    void setpixelrgb(int x, int y, int r, int g, int b);
  private:
   int fbfd;
   char *fbp;
   long int screensize;
   struct fb_var_screeninfo vinfo;
   struct fb_fix_screeninfo finfo;
};

#endif /* _JMCCE_FB_H_ */
