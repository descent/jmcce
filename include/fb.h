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
void fb_setfgcolor (int);
void fb_setbgcolor (int);
int fb_drawline (int x1, int y1, int x2, int y2);	/* ok */
int fb_drawpixel (int x, int y);	/* ok */

#endif /* _JMCCE_FB_H_ */
