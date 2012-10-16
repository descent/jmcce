
/****************************************************************************/
/*            JMCCE  - error.c                                              */
/*                                                                          */
/*                      $Id: error.c,v 1.1.1.1 2002/05/03 04:01:07 kids Exp $   */
/****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */


#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "jmcce.h"
#include "error.h"
#include "hztty.h"
#include "draw.h"
#include "hzinput.h"

#ifdef DEBUG

#define DEBUG_OUTPUT "debuglog"

static FILE *debug_fp;


void
debug (char *fmt, ...)
{
  va_list ap;
  va_start (ap, fmt);
  vfprintf (debug_fp, fmt, ap);
  va_end (ap);
  fflush (debug_fp);
}

#endif /* DEBUG */

void
error (char *fmt, ...)
{
  char buf[NUM_OF_COL];
  va_list ap;
  va_start (ap, fmt);
  vsprintf (buf, fmt, ap);
  va_end (ap);
  input_clear_line (ERROR_LINE, ERROR_BGCOLOR);
  input_print_string (0, ERROR_LINE, "ERROR:", ERROR_FGCOLOR, ERROR_BGCOLOR);
  input_print_string (6, ERROR_LINE, buf, ERROR_FGCOLOR, ERROR_BGCOLOR);
  beep1 ();
  get_key (1);
  input_clear_line (ERROR_LINE, INPUT_BGCOLOR);
}


void
out_of_memory (char *file_name, char *func_name, int line_no)
{
  error ("fatal error: out of memory in file %s(%d), function %s",
	 file_name, line_no, func_name);
  error_done ();
  exit (-1);
}

void
error_init (void)
{
#ifdef DEBUG
  char buf[80];
  sprintf (buf, "%s/%s", home_dir, DEBUG_OUTPUT);
  unlink (buf);
  debug_fp = fopen (buf, "w");
  if (debug_fp == NULL) {
    printf ("ERROR: Can't open debug message log file.\n");
    exit (-1);
  }
  fprintf (debug_fp, "debug logging file.\n");
#endif
}

void
error_done (void)
{
#ifdef DEBUG
  fclose (debug_fp);
#endif
}
