#ifndef _JMCCE_ERROR_H_
#define _JMCCE_ERROR_H_

/* error.h: define the error code */

#define ERROR_LINE 1
#define MAX_ERR_STRLEN	"72"

#ifdef DEBUG
void debug (char *fmt, ...);
#else
#	define debug()
#endif /* DEBUG */

#define ERROR_FGCOLOR   (7)
#define ERROR_BGCOLOR   (4)

void error_init (void);
void error_done (void);
void error (char *fmt, ...);
void error_line (void);
void out_of_memory (char *file_name, char *func_name, int line_no);


#endif /* _JMCCE_ERROR_H_ */
