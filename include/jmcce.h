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

#define CTRL_ALT_0	(150)
#define CTRL_ALT_1	(151)
#define CTRL_ALT_2	(152)
#define CTRL_ALT_3	(153)
#define CTRL_ALT_4	(154)
#define CTRL_ALT_5	(155)
#define CTRL_ALT_6	(156)
#define CTRL_ALT_7	(157)
#define CTRL_ALT_8	(158)
#define CTRL_ALT_9	(159)
#define CTRL_SPACE	(160)
#define SHIFT_SPACE     (161)

#define CTRL_ALT_A	(170)
#define CTRL_ALT_N	(171)
#define CTRL_ALT_P	(172)
#define CTRL_ALT_R	(173)
#define CTRL_ALT_X      (174)
#define SCROLL_LOCK	(175)

#define CTRL_ALT_G      (220)
#define CURSORUPKEY	(221)

#define CTRL_PGUP       (225)


#define BIG5		(886)
#define GB		(86)


#endif /* _JMCCE_JMCCE_H_ */

#define JMCCE_LOG	"jmcce.log"
