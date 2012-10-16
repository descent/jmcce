#ifndef _JMCCE_JMCCE_H_
#define _JMCCE_JMCCE_H_

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
inline void beep1 (void);

/* special function key use to active input method */
#define NR_SPEC_KEY	(20)

#define CTRL_ALT_0	(50)
#define CTRL_ALT_1	(51)
#define CTRL_ALT_2	(52)
#define CTRL_ALT_3	(53)
#define CTRL_ALT_4	(54)
#define CTRL_ALT_5	(55)
#define CTRL_ALT_6	(56)
#define CTRL_ALT_7	(57)
#define CTRL_ALT_8	(58)
#define CTRL_ALT_9	(59)
#define CTRL_SPACE	(60)
#define SHIFT_SPACE     (61)

#define CTRL_ALT_A	(212)
#define CTRL_ALT_N	(213)
#define CTRL_ALT_P	(214)
#define CTRL_ALT_R	(215)
#define CTRL_ALT_X      (216)
#define SCROLL_LOCK	(217)

#define CTRL_ALT_G      (220)
#define CURSORUPKEY	(221)

#define CTRL_PGUP       (225)


#define BIG5		(886)
#define GB		(86)


#endif /* _JMCCE_JMCCE_H_ */

#define JMCCE_LOG	"jmcce.log"
