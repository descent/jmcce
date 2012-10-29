#ifndef _JMCCE_JMCCE_H_
#define _JMCCE_JMCCE_H_

#include <sys/types.h>
#include <sys/ioctl.h>

#include <linux/fs.h>
#include <linux/kd.h>
#include <linux/vt.h>
#include <linux/major.h>
#include <linux/keyboard.h>

extern int console_fd;

#define NORMAL_MODE	(0)
#define HISTORY_MODE	(1)

#define HISTORY_PAGE	(5)

#define SIGREL		SIGUSR1
#define SIGACQ		SIGUSR2
/* SVGALib use the SIGUSR1/2 to handle terminal switch */

extern int console_mode;
extern int active_console;

int get_key (int wait);
/*
 * beep:
        generate beep sound.
 * input: none
 * output: none
 * side effect: none
 */
inline void beep1 (void)
{
  ioctl (console_fd, KDMKTONE, 0x100637);
}

/* special function key use to active input method */
#define NR_SPEC_KEY	(20)

#define CTRL_ALT_0	(0x10)
#define CTRL_ALT_1	(0x11)
#define CTRL_ALT_2	(0x110)
#define CTRL_ALT_3	(0x13)
#define CTRL_ALT_4	(0x14)
#define CTRL_ALT_5	(0x15)
#define CTRL_ALT_6	(0x16)
#define CTRL_ALT_7	(0x17)
#define CTRL_ALT_8	(0x18)
#define CTRL_ALT_9	(0x19)
#define CTRL_SPACE	(0x28)
#define SHIFT_SPACE     (0x29)

#define CTRL_ALT_A	(0xf0)
#define CTRL_ALT_N	(0xf1)
#define CTRL_ALT_P	(0xf2)
#define CTRL_ALT_R	(0xf3)
#define CTRL_ALT_X      (0xf4)
#define SCROLL_LOCK	(0xf5)

#define CTRL_ALT_G      (0xf6)
#define CURSORUPKEY	(0xf7)

#define CTRL_PGUP       (225)


#define BIG5		(886)
#define GB		(86)
#define UTF8		(887)


#endif /* _JMCCE_JMCCE_H_ */

#define JMCCE_LOG	"jmcce.log"
