#ifndef _JMCCE_HZTTY_H_
#define _JMCCE__HZTTY_H_

#define DEFAULT_BACKGROUND_COLOR	(0)
#define DEFAULT_FOREGROUND_COLOR	(7)

#define NUM_OF_ROW		(24)
#define NUM_OF_COL		(80)

#define NR_HZTTY		(10)
#define NPAR			(16)

#define MAX_PATH		(256)

/* normal key */

#define 	CHAR_BEEP	(0x07)
#define		CHAR_BS		(0x08)
#define		CHAR_VT		(0x09)
#define		CHAR_LF		(0x0A)
#define		CHAR_CR		(0x0D)
#define		CHAR_S0		(0x0E)
#define		CHAR_S1		(0x0F)
#define		CHAR_ESC	(0x1B)
#define		CHAR_SPC	(0x20)
#define 	CHAR_DEL	(0x7F)

enum
{
  STATE_NORMAL,
  STATE_HZCODE,
  STATE_ESCAPE,
  STATE_SQUARE,
  STATE_FUNCKEY,
  STATE_GETPARS,
  STATE_GOTPARS,
  STATE_HASH,
  STATE_SETG0,
  STATE_SETG1
};

#define COLOR_COMPOSE(fg,bg)		(((bg) << 4) + (fg))
#define FGCOLOR(x)			((x) & 0xF)
#define BGCOLOR(x)			((x) >> 4)

typedef struct _hz_tty
{
  struct _hz_tty *prev_hztty;
  struct _hz_tty *next_hztty;
  char tty_name[6];
  int index;
  int tty_fd;
  int pid;
  unsigned int terminate:1;
  int buf_size;
  unsigned char *text_buf;
  unsigned char *attr_buf;
  int origin;
  int pos;
  int cur_x, cur_y, saved_cur_x, saved_cur_y;
  char *Translate;
  char *G0_charset;
  char *G1_charset;
  char *saved_G0;
  char *saved_G1;
  int top, bottom;
  int foreground_color, saved_fg_color, fg_color;
  int background_color, saved_bg_color, bg_color;
  int terminal_state;
  int need_wrap;
  unsigned int tab_stop[5];
  unsigned int npar;
  union
  {
    unsigned int par[NPAR];
    //unsigned char filename[MAX_PATH];
  }
  param;
  /* set when it is active tty & active console */
  unsigned int screen_write_ok:1;

  /* special mode */
  /* have to change console mode for tty change */
  unsigned int autorepeat:1;
  unsigned int dec_cursor_keys:1;
  unsigned int lf_mode;
  unsigned int applic;
  /* ------------------------------------ */

  unsigned int video_erase_color;
  unsigned int charset:1;
  unsigned int s_charset:1;
  unsigned int invert_mode:1;
  unsigned int origin_mode:1;
  unsigned int autowrap_mode:1;
  unsigned int cursor_visible:1;
  unsigned int insert_mode:1;
  unsigned int intensity:2;	/* 0=halt-bright, 1=normal, 2=bold */
  unsigned int s_intensity:2;
  unsigned int underline:1;
  unsigned int s_underline:1;
  unsigned int blink:1;
  unsigned int s_blink:1;
  unsigned int reverse:1;
  unsigned int s_reverse:1;
  unsigned int ques:1;
  /* terminal state */
}
hz_tty;


/* pty function function & variable */

extern hz_tty *hztty_list;
extern int num_hztty;

void hztty_init (void);
void hztty_done (void);
void hztty_open (int histtory_line);
void hztty_close (hz_tty * tty);
void hztty_write (hz_tty * tty, unsigned char *buf, int n);
void change_hztty (int direct);
void hztty_redraw (void);

void reset_terminal (hz_tty * tty, int do_clear);
void repaint_one_line (hz_tty * tty, int line);
void repaint_hztty (void);

void enter_history_mode (void);
void leave_history_mode (void);
void history_up (void);
void history_down (void);
void history_page_down (void);
void history_page_up (void);
void history_right (void);
void history_left (void);
void history_home (void);
void history_end (void);
void buffer_copy (void);

int setup_delay_timer ();

#endif /* _JMCCE_HZTTY_H_ */
