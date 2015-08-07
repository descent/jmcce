
/****************************************************************************/
/*            JMCCE  - jmcce.c                                              */
/*                                                                          */
/*            	$Id: jmcce.c,v 1.3 2002/05/17 18:09:14 kids Exp $   */
/****************************************************************************/

#include "newimp.h"
#include "encoding.h"
#include "config.h"

#include <pwd.h>
#include <ctype.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <locale.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <cstring>
#include <termios.h>
#include <linux/input.h>

#include <string>

#include "jmcce.h"
#include "draw.h"
#include "hbffont.h"
#include "hztty.h"
#include "error.h"
#include "hzinput.h"
//#include <vgamouse.h>

#define STAT_FILE       "/proc/self/stat"
#define CONSOLE         "/dev/console"
#define BUFSIZE         4096
#define	CHEW_NUM	4

char copyright[] = {
  "+-----------------------------------------------+\n"
    "|               中文端機模擬程式                |\n"
    "|              本程式由曾昭明編寫               |\n"
    "|        任何重製與使用均需符合GPL之規範        |\n"
    "+-----------------------------------------------+\n"
};

char copyright101[] = {
  "+-----------------------------------------------+\n"
    "|              JMCCE 1.01 版                    |\n"
    "|                 = JMCE + CCE 簡繁兩用         |\n"
    "|        由 台灣立尼仕網路股份有限公司 撰寫     |\n"
    "|                                               |\n"
    "|        任何重製與使用均需符合GPL之規範        |\n"
    "+-----------------------------------------------+\n"
};

int console_fd;

int encode_mode = BIG5;
char encode_name[10]="UTF8";

static enum
{
  NONE,
  LEAVE_JMCCE,
  RETURN_JMCCE,
}
screen_request;

extern int gsCurrent_method;
extern int use_fb;
static struct termios save_termios;
static unsigned int old_keymap[30];
static char *paste_buffer;
int console_mode = NORMAL_MODE;
int active_console = 1;

int gFont_bytes = 32;
int gEncode;
int gDontLoadInputMethod = 0;


int mx, my;

char Item_str[81][2][7];
int Item_attr[81];

char home_dir[32];

void BarMenuInit (int xmax, int ymax);
void BarMenu (int xmax, int ymax, int leftmar, int stepwidth);



/* private function */

/* 
 * is_console:
        Check the program cotrol terminal is a graphics console or not.

 * return: none
 * side effect: none
*/

static bool
is_console (void)
{
  // ref : fbterm/input.cpp # TtyInput::createInstance()
  char buf[64];
  if (ttyname_r(STDIN_FILENO, buf, sizeof(buf))) 
  {
    fprintf(stderr, "%s: stdin isn't a tty!\n", buf);
    return false;
  }       
        
  if (!strstr(buf, "/dev/tty") && !strstr(buf, "/dev/vc")) 
  {
    fprintf(stderr, "%s: stdin isn't a interactive tty!\n", buf);
    return false;
  }
  return true;

#if 0
  int dev;
  FILE *fp;

  fp = fopen (STAT_FILE, "r");
  if (fp == NULL) {
    fprintf (stderr, "fatal error: cannot open %s\n", STAT_FILE);
    exit (-1);
  }
  fscanf (fp, "%*d %*s %*s %*d %*d %*d %d", &dev);
#if 0
  if ((MAJOR (dev) != TTY_MAJOR) || (MINOR (dev) > 64)) {
    fprintf (stderr, "fatal error: cannot open console deveice %s\n",
	     CONSOLE);
    exit (-1);
  }
#endif
  fclose (fp);
  //return MINOR (dev);
  return 1;
#endif
}

/*
 * set_console_raw:
 *      set the console to raw mode.
 * input:none
 * output:none
 * side effect: all procedure use console IO.
 */
static void
set_console_raw (void)
{
  struct termios buf;

  buf = save_termios;
  buf.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  buf.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  buf.c_cflag &= ~(CSIZE | PARENB);
  buf.c_cflag |= CS8;
  buf.c_oflag &= ~(OPOST);
  buf.c_cc[VMIN] = 1;
  buf.c_cc[VTIME] = 0;
  if (tcsetattr (console_fd, TCSAFLUSH, &buf) < 0) {
    error ("Can't set console termios\n");
    exit (-1);
  }
}

static void
set_keymap (void)
{
  struct kbentry spec;
  int i;



  spec.kb_table = (1 << KG_CTRL) + (1 << KG_ALT);

    spec.kb_index = KEY_3;
    ioctl (console_fd, KDGKBENT, &spec);
    old_keymap[0] = spec.kb_value;
    spec.kb_value = CTRL_ALT_3;
    ioctl (console_fd, KDSKBENT, &spec);

    spec.kb_index = KEY_7;
    ioctl (console_fd, KDGKBENT, &spec);
    old_keymap[1] = spec.kb_value;
    spec.kb_value = CTRL_ALT_7;
    ioctl (console_fd, KDSKBENT, &spec);

    spec.kb_index = KEY_8;
    ioctl (console_fd, KDGKBENT, &spec);
    old_keymap[1] = spec.kb_value;
    spec.kb_value = CTRL_ALT_8;
    ioctl (console_fd, KDSKBENT, &spec);


#if 0
  /*  setup ctrl-alt-1 ~ ctrl-alt-9 */
  for (i = 1; i < 10; i++) {
    spec.kb_index = SCANCODE_1 + i - 1;	/* 2-10 */
    ioctl (console_fd, KDGKBENT, &spec);
    old_keymap[i] = spec.kb_value;
    spec.kb_value = K (KT_LATIN, CTRL_ALT_0 + i);
    ioctl (console_fd, KDSKBENT, &spec);
  }
#endif

#if 0
  /* setup ctrl-alt-0 */
  spec.kb_index = SCANCODE_0 /* 11 */ ;
  ioctl (console_fd, KDGKBENT, &spec);
  old_keymap[0] = spec.kb_value;
  spec.kb_value = K (KT_LATIN, CTRL_ALT_0);
  ioctl (console_fd, KDSKBENT, &spec);
#endif

  /* setup ctrl-space */
  spec.kb_table = (1 << KG_CTRL);
  spec.kb_index = KEY_SPACE;
  ioctl (console_fd, KDGKBENT, &spec);
  old_keymap[10] = spec.kb_value;
  spec.kb_value = CTRL_SPACE;
  ioctl (console_fd, KDSKBENT, &spec);

  /* setup ctrl-alt-a */
  spec.kb_table = (1 << KG_CTRL) + (1 << KG_ALT);
  spec.kb_index = KEY_A;
  ioctl (console_fd, KDGKBENT, &spec);
  old_keymap[11] = spec.kb_value;
  spec.kb_value = K (KT_LATIN, CTRL_ALT_A);
  ioctl (console_fd, KDSKBENT, &spec);

  /* setup ctrl-alt-x */
  spec.kb_table = (1 << KG_CTRL) + (1 << KG_ALT);
  spec.kb_index = KEY_X /* 45 */ ;
  ioctl (console_fd, KDGKBENT, &spec);
  old_keymap[12] = spec.kb_value;
  spec.kb_value = K (KT_LATIN, CTRL_ALT_X);
  ioctl (console_fd, KDSKBENT, &spec);

  /* setup ctrl-alt-n */
  spec.kb_index = KEY_N /* 49 */ ;
  ioctl (console_fd, KDGKBENT, &spec);
  old_keymap[13] = spec.kb_value;
  spec.kb_value = K (KT_LATIN, CTRL_ALT_N);
  ioctl (console_fd, KDSKBENT, &spec);

  /* setup ctrl-alt-p */
  spec.kb_index = KEY_P /* 25 */ ;
  ioctl (console_fd, KDGKBENT, &spec);
  old_keymap[14] = spec.kb_value;
  spec.kb_value = K (KT_LATIN, CTRL_ALT_P);
  ioctl (console_fd, KDSKBENT, &spec);

  /* setup ctrl-alt-r */
  spec.kb_index = KEY_R /* 19 */ ;
  ioctl (console_fd, KDGKBENT, &spec);
  old_keymap[15] = spec.kb_value;
  spec.kb_value = K (KT_LATIN, CTRL_ALT_R);
  ioctl (console_fd, KDSKBENT, &spec);



  /* setup scroll lock key */
  spec.kb_table = 0;
  spec.kb_index = KEY_SCROLLLOCK /* 70 */ ;
  ioctl (console_fd, KDGKBENT, &spec);
  old_keymap[16] = spec.kb_value;
  spec.kb_value = K (KT_LATIN, SCROLL_LOCK);
  ioctl (console_fd, KDSKBENT, &spec);

  /* setup shift-space */
  spec.kb_table = (1 << KG_SHIFT);
  spec.kb_index = KEY_SPACE /* 57 */ ;
  ioctl (console_fd, KDGKBENT, &spec);
  old_keymap[17] = spec.kb_value;
  spec.kb_value = K (KT_LATIN, SHIFT_SPACE);
  ioctl (console_fd, KDSKBENT, &spec);

  /* setup ctrl-alt-g */
  /* repeat input, append by Jack Gao */
  spec.kb_table = (1 << KG_CTRL) + (1 << KG_ALT);

  spec.kb_index = KEY_G /* 34 */ ;
  ioctl (console_fd, KDGKBENT, &spec);
  old_keymap[18] = spec.kb_value;
  spec.kb_value = K (KT_LATIN, CTRL_ALT_G);
  ioctl (console_fd, KDSKBENT, &spec);

}

static void
restore_keymap (void)
{
  struct kbentry spec;
  int i;


#if 0
  spec.kb_table = (1 << KG_CTRL) + (1 << KG_ALT);
  for (i = 1; i < 10; i++) {
    spec.kb_index = KEY_1 + i;
    spec.kb_value = old_keymap[i];
    ioctl (console_fd, KDSKBENT, &spec);
  }

  /* restore ctrl-alt-0 */
  spec.kb_index = SCANCODE_0 /* 11 */ ;
  spec.kb_value = old_keymap[0];
  ioctl (console_fd, KDSKBENT, &spec);
#endif

  /* restore ctrl-space */
  spec.kb_table = (1 << KG_CTRL);
  spec.kb_index = KEY_SPACE /* 57 */ ;
  spec.kb_value = old_keymap[10];
  ioctl (console_fd, KDSKBENT, &spec);

  /* restore ctrl-alt-a */
  spec.kb_table = (1 << KG_CTRL) + (1 << KG_ALT);
  spec.kb_index = KEY_A /* 30 */ ;
  spec.kb_value = old_keymap[11];
  ioctl (console_fd, KDSKBENT, &spec);

  /* restore ctrl-alt-x */
  spec.kb_table = (1 << KG_CTRL) + (1 << KG_ALT);
  spec.kb_index = KEY_X /* 45 */ ;
  spec.kb_value = old_keymap[12];
  ioctl (console_fd, KDSKBENT, &spec);

  /* restore ctrl-alt-n */
  spec.kb_index = KEY_N /* 49 */ ;
  spec.kb_value = old_keymap[13];
  ioctl (console_fd, KDSKBENT, &spec);

  /* restore ctrl-alt-p */
  spec.kb_index = KEY_P /* 25 */ ;
  spec.kb_value = old_keymap[14];
  ioctl (console_fd, KDSKBENT, &spec);

  /* restore ctrl-alt-r */
  spec.kb_index = KEY_R /* 19 */ ;
  spec.kb_value = old_keymap[15];
  ioctl (console_fd, KDSKBENT, &spec);

  /* restore SCROLL_LOCK */
  spec.kb_table = 0;
  spec.kb_index = KEY_SCROLLLOCK /* 70 */ ;
  spec.kb_value = old_keymap[16];
  ioctl (console_fd, KDSKBENT, &spec);

  /* restore SHIFT_SPACE */
  spec.kb_table = (1 << KG_SHIFT);
  spec.kb_index = KEY_SPACE /* 57 */ ;
  spec.kb_value = old_keymap[17];
  ioctl (console_fd, KDSKBENT, &spec);

  /* restore ctrl-alt-g */
  /* append by Jack Gao */
  spec.kb_table = (1 << KG_CTRL) + (1 << KG_ALT);

  spec.kb_index = KEY_G /* 34 */ ;
  spec.kb_value = old_keymap[18];
  ioctl (console_fd, KDSKBENT, &spec);


}


/*
   We just record the occures of console switch,and switch when we return
   to ioloop.This will avoid switch change when draw graphics in screen
*/

static void
signal_release_vt (int signo)
{
  if (screen_request == NONE)
    screen_request = LEAVE_JMCCE;
}

static void
signal_acquire_vt (int signo)
{
  if (screen_request == NONE)
    screen_request = RETURN_JMCCE;
}

static void
signal_ignore (int signo)
{

}


/*
static void signal_alarm(int signo)
{
  if (screen_request == NONE)
    screen_request = SCREEN_FLUSH;
}
*/

static void
clean_up (void)
{
  restore_keymap ();
  if (tcsetattr (console_fd, TCSAFLUSH, &save_termios) < 0)
    error ("Can't not reset console termios\n");
  screen_done ();
  error_done ();
  write (console_fd, "\033c", 2);
}

static void
signal_catch (int signo)
{
  if (!use_fb && signo == SIGINT) {
    /* FIX for SVGALib , not good idea. */
    screen_request = RETURN_JMCCE;
    kill (hztty_list->pid, signo);
    return;
  }
  error ("Signal No %d is catched", signo);
  clean_up ();
  exit (-1);
}

/*
int x=0,y=0,color=0,button,leftpressed;
char ttt[50];
void update_mouse()
{
        gl_fillbox( 20 , 16, 8, 16, 2);

        mouse_update();
        x = mouse_getx();
        y = mouse_gety();
        button = mouse_getbutton();
        
        sprintf(ttt,"X=%d Y=%d",x,y);
        print_string(5,10,ttt,7,0);
        if (button & MOUSE_LEFTBUTTON) 
        {
            if (!leftpressed) 
            {
                color = (color+1) % 16;
                leftpressed = 1;
            }
        } 
        else leftpressed = 0;
 }
*/

void
process_screen_event (void)
{

  //update_mouse(); 
  switch (screen_request) {
  case LEAVE_JMCCE:
    screen_flipaway ();
    //restore_keymap ();
    ioctl (console_fd, VT_RELDISP, 1);
    screen_request = NONE;
    break;
  case RETURN_JMCCE:
    set_keymap ();
    screen_return ();
    set_console_raw ();
    repaint_hztty ();
    on_off_cursor (hztty_list->cur_x, hztty_list->cur_y);
    refresh_input_method_area();
    screen_request = NONE;
    break;
  case NONE:
    break;			/* do nothing */
  }
}

/* public function */

/*
 * get_key:
 *      get a keyboard input from console.
 * param: int wait -> 
 *              true for wait keyboard input, false for checking input
 * return: -1 for no keyboard input otherwise the ascii value
 * side effect: unknown
 */

int
get_key (int wait)
{
  unsigned char c;

  if (!wait) {
    fd_set rset;
    struct timeval tv;

    tv.tv_sec = tv.tv_usec = 0;
    FD_ZERO (&rset);
    FD_SET (console_fd, &rset);
    select (console_fd + 1, &rset, NULL, NULL, &tv);
    if (!FD_ISSET (console_fd, &rset))
      return -1;
  }
  read (console_fd, &c, sizeof (unsigned char));
  return c;
}


void
init (void)
{
  struct sigaction act;
  int i=0;
  unsigned char *bitmap;

  //int ttyminor;

  paste_buffer = NULL;
  error_init ();
  if (is_console() == false)
  {
    exit(0);
  }
  console_fd = open (CONSOLE, O_RDWR);

  if (console_fd == -1) {
    fprintf (stderr, "fatal error: Cannot open console device\n");
    exit (-1);
  }

  font_init ();

#ifdef DRAW_TEST
  unsigned char *ascii = ascii_font['A'];
  for (i=0 ; i < 18 ; ++i)
  {
    char c = ascii[i];
    int j=0;
    //printf("%x ", ascii[i]);

    for (j=7 ; j>=0 ; --j)
    {
      if (((c >> j) & 0x1) == 1)
        printf("*");
      else
        printf("_");
    }
    printf("\n");

  }
  printf("\n");
#endif
  hz_input_init ();

  if (screen_init() == false)
  {
    printf("screen_init() fail!!\n");
    exit(-1);
  }
#ifdef FB_TEST
  void c_draw_ascii (int col, int y, unsigned char *bitmap, int color1);
  void c_draw_hanzi (int col, int y, unsigned char *bitmap, int color1);

  c_draw_ascii(10, 10, ascii_font['B'], 5);
  #if 1
  bitmap = hbfGetBitmap (0xa7da, STANDARD, gFont_bytes);
  if (bitmap != NULL)
  {
    int k=0;
    c_draw_hanzi(10, 30, bitmap, 7);
  }
  #endif
  sleep(8);
  exit(0);
#endif

#ifdef DRAW_TEST

  vgalib_draw_ascii(10, 10, ascii_font['B'], 5);
  bitmap = hbfGetBitmap (0xa7da, STANDARD, gFont_bytes);
  if (bitmap != NULL)
  {
    int k=0;
    vgalib_draw_hanzi(10, 30, bitmap, 7);
    #if 0

  for (i=0 ; i < 18 ; ++i)
  {
    for (k=0 ; k < 2 ; ++k)
    {

    char c = *bitmap++;
    int j=0;
    //printf("%x ", ascii[i]);

    for (j=7 ; j>=0 ; --j)
    {
      if (((c >> j) & 0x1) == 1)
        printf("*");
      else
        printf("_");
    }
    }
    printf("\n");

  }
  printf("\n");
  #endif
  }

#endif
#if 1

  /*
     vga_setmousesupport(1);
     gl_setcontextvga(G640x480x16);
     gl_enableclipping();
   */

  screen_request = NONE;

  act.sa_handler = signal_release_vt;
  act.sa_flags = 0;
  sigemptyset (&act.sa_mask);
  sigaddset (&act.sa_mask, SIGACQ);
  sigaction (SIGUSR1, &act, NULL);

  act.sa_handler = signal_acquire_vt;
  sigemptyset (&act.sa_mask);
  sigaddset (&act.sa_mask, SIGREL);
  sigaction (SIGUSR2, &act, NULL);
  //sigaction(SIGINT, &act, signal_ignore);
  /*
     //  act.sa_handler = signal_alarm;
     sigemptyset(&act.sa_mask);
     sigaddset(&act.sa_mask,SIGUSR1);  
     sigaddset(&act.sa_mask,SIGUSR2);  
     //  sigaction(SIGALRM,&act,NULL); 
   */

  hztty_init ();
  if (tcgetattr (console_fd, &save_termios) < 0) {
    error ("Can't get console termios\n");
    exit (-1);
  }
  set_console_raw ();
  set_keymap ();

/* catch serious signal and do clean up before exit */
  act.sa_handler = signal_catch;
  act.sa_flags = 0;
  sigemptyset (&act.sa_mask);
  sigaction (SIGHUP, &act, NULL);
  sigaction (SIGINT, &act, NULL);
  sigaction (SIGQUIT, &act, NULL);
  sigaction (SIGILL, &act, NULL);
  sigaction (SIGABRT, &act, NULL);
  sigaction (SIGIOT, &act, NULL);
  sigaction (SIGBUS, &act, NULL);
  sigaction (SIGFPE, &act, NULL);
  sigaction (SIGTERM, &act, NULL);

  //asm_set_split ();		/* support split-window */

#endif
}

enum
{ H_NORMAL, H_ESCAPE, H_FUNC_KEY, H_XXX }
h_state = H_NORMAL;
enum
{ HOME, END, PGUP, PGDN }
key = PGUP;

void
ProcessNormalModeKey (unsigned char c, fd_set * p_rrset)
{
  switch (c) {
  case CTRL_ALT_0:
  case CTRL_ALT_1:
  case CTRL_ALT_2:
  case CTRL_ALT_3:
  case CTRL_ALT_4:
  case CTRL_ALT_5:
  case CTRL_ALT_6:
  case CTRL_ALT_7:
  case CTRL_ALT_9:
    if ((c - CTRL_ALT_0) == 0) {
      mx = 10;
      my = 1;
      set_active_input_method (0);
    } else {
      mx = (c - CTRL_ALT_0);
      my = 1;
      set_active_input_method (mx);
    }
    BarMenuInit (10, 1);
    BarMenu (10, 1, 8, 6);
    break;
  case CTRL_ALT_A:
    hztty_open (NUM_OF_ROW * HISTORY_PAGE);
    FD_SET (hztty_list->tty_fd, p_rrset);
    refresh_input_method_area ();
    break;
  case CTRL_ALT_X:
    if (hztty_list->terminate == 0)
      kill (hztty_list->pid, SIGHUP);
    break;
  case CTRL_ALT_N:
    change_hztty (1);
    break;
  case CTRL_ALT_P:
    change_hztty (0);
    break;
  case CTRL_ALT_R:
    hztty_redraw ();
    refresh_input_method_area ();	/* + */
    break;
  case CTRL_ALT_G:		/* repeat key, append by Jack Gao */
    repeat_last_phrase (hztty_list->tty_fd);
    break;
  case CTRL_SPACE:
    toggle_input_method ();
    break;
  case SHIFT_SPACE:
    toggle_half_full ();
    break;
  case SCROLL_LOCK:
    ioctl (console_fd, KDSETLED, 0x1);
    console_mode = HISTORY_MODE;
    h_state = H_NORMAL;
    enter_history_mode ();
    refresh_input_method_area();
    break;
  case CTRL_ALT_8:
  {
    switch (encode_mode)
    {
      case BIG5:
      {
        encode_mode = UTF8;
        setenv ("LC_ALL", "en_US.UTF-8", 1);
        setlocale(LC_ALL, "en_US.UTF-8");
        sprintf(encode_name, "utf8");
        break;
      }
      case UTF8:
      {
        encode_mode = BIG5;
        setenv ("LC_ALL", "zh_TW.Big5", 1);
        setlocale(LC_ALL, "zh_TW.Big5");
        sprintf(encode_name, "big5");
        break;
      }
    }
    input_print_string (76, 1, encode_name, RED, INPUT_BGCOLOR); 

    break;
  }
  default:
    hz_filter (hztty_list->tty_fd, c);
    break;
  }
}

void
ProcessHistoryModeKey (unsigned char c)
{
  switch (h_state) {
  case H_NORMAL:
    switch (c) {
    case SCROLL_LOCK:
      leave_history_mode ();
      ioctl (console_fd, KDSETLED, 0x9);
      console_mode = NORMAL_MODE;
      refresh_input_method_area();
      break;
    case '\033':
      h_state = H_ESCAPE;
      break;
    }
    break;

  case H_ESCAPE:
    if (c == '[')
      h_state = H_FUNC_KEY;
    else
      h_state = H_NORMAL;
    break;

  case H_FUNC_KEY:
    switch (c) {
    case 'A':
      history_up ();
      h_state = H_NORMAL;
      break;
    case 'B':
      history_down ();
      h_state = H_NORMAL;
      break;
    case 'C':
      history_right ();
      h_state = H_NORMAL;
      break;
    case 'D':
      history_left ();
      h_state = H_NORMAL;
      break;
    case '1':
      key = HOME;
      h_state = H_XXX;
      break;
    case '4':
      key = END;
      h_state = H_XXX;
      break;
    case '5':
      key = PGUP;
      h_state = H_XXX;
      break;
    case '6':
      key = PGDN;
      h_state = H_XXX;
      break;
    }
    break;

  case H_XXX:
    switch (key) {
    case HOME:
      history_home ();
      break;
    case END:
      history_end ();
      break;
    case PGUP:
      history_page_up ();
      break;
    case PGDN:
      history_page_down ();
      break;
    }
    h_state = H_NORMAL;
    break;
  }
}

void
run (void)
{
  extern FILE *fs;

  fd_set rset, rrset;
  static unsigned char buf[BUFSIZE];
  hz_tty *hztty;
  int nread, i;

  hztty_open (NUM_OF_ROW * HISTORY_PAGE);

  refresh_input_method_area ();

  FD_ZERO (&rrset);
  FD_SET (console_fd, &rrset);
  FD_SET (hztty_list->tty_fd, &rrset);

  while (hztty_list) {
    process_screen_event ();
    rset = rrset;		/* structure copy, unportable */
    if (select (FD_SETSIZE, &rset, NULL, NULL, NULL) == -1)
      continue;
    if (hztty_list && FD_ISSET (console_fd, &rset)) {
      nread = read (console_fd, buf, BUFSIZE);
      if (nread <= 0)
	break;
      for (i = 0; i < nread; i++) {
	switch (console_mode) {
	case NORMAL_MODE:
	  ProcessNormalModeKey (buf[i], &rrset);
	  break;
	case HISTORY_MODE:
	  ProcessHistoryModeKey (buf[i]);
	  break;
	}
      }
    }
    #ifdef VGALIB
    gl_copyscreen(physical_screen);
    #endif

    for (i = num_hztty, hztty = hztty_list; i > 0; i--) {
      if (FD_ISSET (hztty->tty_fd, &rset)) {
	nread = read (hztty->tty_fd, buf, BUFSIZE);

	if (nread > 0)
        {

    switch (encode_mode)
    {
      case BIG5:
      {
          hztty_write (hztty, buf, nread);
        break;
      }
      case UTF8:
      {
          std::string big5_str;

          if (utf8_to_big5(buf, nread, big5_str) == 0)
          {
          }
          else
          {
            hztty_write (hztty, "???", 3);
          }


	  hztty_write (hztty, big5_str.c_str(), strlen(big5_str.c_str()));
        break;
      }
    }

        }
      }
      #ifdef VGALIB
      gl_copyscreen(physical_screen);
      #endif
      hztty = hztty->next_hztty;
      if (hztty->prev_hztty->terminate) {
	FD_CLR (hztty->prev_hztty->tty_fd, &rrset);
	hztty_close (hztty->prev_hztty);
      }
    }

  }
}

void
done (void)
{
  struct sigaction act;

  /* done in procress and block all serious signal to prevent interrupt */
  act.sa_handler = SIG_IGN;
  act.sa_flags = 0;
  sigemptyset (&act.sa_mask);
  sigaction (SIGHUP, &act, NULL);
  sigaction (SIGINT, &act, NULL);
  sigaction (SIGQUIT, &act, NULL);
  sigaction (SIGILL, &act, NULL);
  sigaction (SIGABRT, &act, NULL);
  sigaction (SIGIOT, &act, NULL);
  sigaction (SIGBUS, &act, NULL);
  sigaction (SIGFPE, &act, NULL);

  restore_keymap ();

  if (tcsetattr (console_fd, TCSAFLUSH, &save_termios) < 0)
    error ("Can't not reset console termios\n");

  hztty_done ();
  //screen_done ();

  hz_input_done ();

  font_done ();
  error_done ();


  if (paste_buffer)
    free (paste_buffer);

  write (console_fd, "\033c", 2); // reset terminal
}

FILE *fs;

char jmcce_path[256];

int
main (int argc, char **argv)
{
  char *_lang;
  struct passwd *pw;

  fs=fopen("./log", "w");

  getcwd(jmcce_path, 256-1);


  _lang = setlocale (LC_CTYPE, "");

  if (_lang != NULL && strncmp (_lang, "zh_CN", 5) == 0) {
    gFont_bytes = 32;
    gEncode = GB;
    setenv ("LC_ALL", "zh_CN.GB2312", 1);
  } else {
    gFont_bytes = 30;
    gEncode = BIG5;
    setenv ("LC_ALL", "zh_TW.Big5", 1);
  }

  if (argc > 1) {
    if (strcmp (argv[1], "-d") == 0) {
      gDontLoadInputMethod = 1;
    } else if (strcmp (argv[1], "-b") == 0) {
      gFont_bytes = 30;
      gEncode = BIG5;
      setenv ("LC_ALL", "zh_TW.Big5", 1);
    } else if (strcmp (argv[1], "-db") == 0) {
      gDontLoadInputMethod = 1;
      gFont_bytes = 30;
      gEncode = BIG5;
      setenv ("LC_ALL", "zh_TW.Big5", 1);
    } else if (strcmp (argv[1], "-g") == 0) {
      gFont_bytes = 32;
      gEncode = GB;
      setenv ("LC_ALL", "zh_CN.GB2312", 1);
    } else if (strcmp (argv[1], "-dg") == 0) {
      gDontLoadInputMethod = 1;
      gFont_bytes = 32;
      gEncode = GB;
      setenv ("LC_ALL", "zh_CN.GB2312", 1);
    } else {
      printf ("\nUsage: jmcce [options]\n\n");
      printf ("       -h  show this help\n");
      printf ("       -d  display only, without loading any input methods\n");
      printf ("       -b  force to big5 mode\n");
      printf ("       -g  force to gb mode\n");
      printf ("       -db force to big5 mode display only\n");
      printf ("       -dg force to gb mode display only\n\n");
      exit (0);
    }
  }

  if (geteuid() != 0) {
  
     printf ("Jmcce : Operation not permitted, may use root permission.\n");
     exit (0);
  }
  
  pw = getpwuid (getuid ());
  strcpy (home_dir, pw->pw_dir);

  init ();
#if 1
  run ();
  unsetenv ("LC_ALL");
  done ();
#endif
  //vga_setmode (TEXT);

  return 0;
}
