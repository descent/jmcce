
/****************************************************************************/
/*            JMCCE  - hztty.c                                              */
/*                                                                          */
/*                      $Id: hztty.c,v 1.1.1.1 2002/05/03 04:01:07 kids Exp $   */
/****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#define OPEN_MAX 10

#include <stdio.h>
#include <termios.h>
#include <fcntl.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <pwd.h>
#include <linux/limits.h>

#include "hztty.h"
#include "error.h"
#include "hzinput.h"
#include "hbffont.h"
#include "jmcce.h"
#include "draw.h"


hz_tty *hztty_list;
int hztty_mask[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };
int num_hztty;

/* private function */

/* FIX ME: 1. When execlp fail, how to handle ? */

static int
tty_fork (hz_tty * tty)
{
  int slave_fd;
  char *ptr1, *ptr2, pt_name[] = "/dev/ptyXX";
  struct winsize ws;

  for (ptr1 = "pqrst"; *ptr1; ptr1++) {
    pt_name[8] = *ptr1;
    for (ptr2 = "0123456789abcdef"; *ptr2; ptr2++) {
      pt_name[9] = *ptr2;
      tty->tty_fd = open (pt_name, O_RDWR);
      if (tty->tty_fd == -1) {
	if (errno == ENOENT) {
	  error ("Unable to open pseodo terminal.");
	  return 0;
	} else
	  continue;
      } else
	goto found;
    }
  }
found:
  pt_name[5] = 't';
  strcpy (tty->tty_name, pt_name + 5);
  tty->pid = fork ();
  if (tty->pid == -1) {
    close (tty->tty_fd);
    error ("system call fork fail");
    return 0;
  }

  if (tty->pid == 0) {		/* in child process */
    int i;
    struct passwd *userpd;

    if ((userpd = getpwuid (getuid ())) == NULL) {
      fprintf (stderr, "error Can't not get user shell.");
      exit (-1);
    }
    /* --UGLY-- */
    /* close all opened file */
    for (i = 3; i < OPEN_MAX; i++)
      close (i);
    /* -------- */
    /* THE ERROR CONDICTION IS NOT IMPLEMENT */
    setsid ();
    pt_name[5] = 't';
    slave_fd = open (pt_name, O_RDWR);
    if (slave_fd == -1) {
      close (tty->tty_fd);
      exit (-1);
    }
    ws.ws_row = NUM_OF_ROW;
    ws.ws_col = NUM_OF_COL;
    ioctl (slave_fd, TIOCSCTTY, 0);
    ioctl (slave_fd, TIOCSWINSZ, &ws);
    seteuid (getuid ());	/* for security */
    dup2 (slave_fd, fileno (stdin));
    dup2 (slave_fd, fileno (stdout));
    dup2 (slave_fd, fileno (stderr));
    close (slave_fd);
    execlp (userpd->pw_shell, userpd->pw_shell, (char *) 0);

    exit (-1);
  }
  return 1;
}

static void
sigcld_handler (int signo)
{
  int status;
  hz_tty *tty;

  tty = hztty_list;
  do {
    if (waitpid (tty->pid, &status, WNOHANG))
      tty->terminate = 1;
    tty = tty->next_hztty;
  } while (tty != hztty_list);
}

#ifdef DEBUG
void
print_list (void)
{
  hz_tty *tty;
  tty = hztty_list;
  debug ("<---->\n");
  if (tty) {
    do {
      debug ("next list -- %p\n", tty);
      tty = tty->next_hztty;
    } while (tty != hztty_list);
    do {
      debug ("prev list -- %p\n", tty);
      tty = tty->prev_hztty;
    } while (tty != hztty_list);
  }
  debug ("<---->\n");
}
#endif

void
hztty_open (int history_line)
{
  hz_tty *tty;
  sigset_t mask;
  int i;

  if (num_hztty < NR_HZTTY) {
    tty = malloc (sizeof (hz_tty));
    if (tty == NULL)
      out_of_memory (__FILE__, "hztty_open", __LINE__);
    if (tty_fork (tty)) {
      tty->terminate = 0;
      tty->buf_size = NUM_OF_COL * history_line;
      tty->text_buf = calloc (tty->buf_size, 1);
      tty->attr_buf = calloc (tty->buf_size, 1);
      tty->pos = tty->origin = 0;
      if (!tty->text_buf || !tty->attr_buf)
	out_of_memory (__FILE__, "hztty_open", __LINE__);

      sigemptyset (&mask);
      sigaddset (&mask, SIGCHLD);
      sigprocmask (SIG_BLOCK, &mask, NULL);

      if (hztty_list) {
	tty->prev_hztty = hztty_list;
	tty->next_hztty = hztty_list->next_hztty;
	hztty_list->next_hztty->prev_hztty = tty;
	hztty_list->next_hztty = tty;
      } else
	hztty_list = tty->prev_hztty = tty->next_hztty = tty;
      num_hztty++;

      for (i = 0; i <= 9; i++)
	if (hztty_mask[i] == 0) {
	  tty->index = i;
	  hztty_mask[i] = 1;
	  break;
	}
      /* find a unused window */
      sigprocmask (SIG_UNBLOCK, &mask, NULL);
      hztty_list = tty;
      reset_terminal (tty, 1);
      on_off_cursor (tty->cur_x, tty->cur_y);
    } else
      free (tty);
  } else
    error ("Too many tty opened!");
}

/*
 * return: next tty or NULL if all tty was closed.
 */

void
hztty_close (hz_tty * tty)
{
  sigset_t mask;

  if (tty == NULL) {
    error ("bug in %s(%d).try to free null tty pointer.", __FILE__, __LINE__);
    exit (-1);
  }
  sigemptyset (&mask);
  sigaddset (&mask, SIGCHLD);
  sigprocmask (SIG_BLOCK, &mask, NULL);

  if (tty != tty->next_hztty) {	/* is all hztty is close? */
    tty->prev_hztty->next_hztty = tty->next_hztty;
    tty->next_hztty->prev_hztty = tty->prev_hztty;
  } else
    hztty_list = NULL;
  num_hztty--;
  hztty_mask[tty->index] = 0;

  sigprocmask (SIG_UNBLOCK, &mask, NULL);
  if (hztty_list == tty)
    change_hztty (1);
  close (tty->tty_fd);
  free (tty->text_buf);
  free (tty->attr_buf);
  free (tty);
}

/*
 * input: direct = 1 when change to next hztty or change to prev hztty
 * potential bug: does not check (hztty_list == NULL)
 */

void
change_hztty (int direct)
{
  if (direct)
    hztty_list = hztty_list->next_hztty;
  else
    hztty_list = hztty_list->prev_hztty;
  hztty_redraw ();
  refresh_input_method_area ();
}

void
hztty_init (void)
{
  struct sigaction act;

  num_hztty = 0;
  hztty_list = NULL;
  act.sa_handler = &sigcld_handler;
  sigemptyset (&act.sa_mask);
  act.sa_flags = 0;
  sigaction (SIGCHLD, &act, NULL);
}

void
hztty_done (void)
{
  while (hztty_list)
    hztty_close (hztty_list);
}
