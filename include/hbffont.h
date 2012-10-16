#ifndef _JMCCE_HBFFONT_H_
#define _JMCCE_HBFFONT_H_

extern unsigned char ascii_font[256][18];

#if 0
/* Using JMCCE_FONTDIR instead of. */
#define DEFAULT_ASCFONT       "/usr/share/fonts/chinese/8x16"
#define DEFAULT_HZFONT_BIG5   "/usr/share/fonts/chinese/han15.hbf"
#define DEFAULT_HZFONT_GB     "/usr/share/fonts/chinese/cclib16.hbf"
#else
#define DEFAULT_ASCFONT		(JMCCE_FONTDIR "/8x16")
#define DEFAULT_HZFONT_BIG5	(JMCCE_FONTDIR "/han15.hbf")
#define DEFAULT_HZFONT_GB	(JMCCE_FONTDIR "/cclib16.hbf")
#endif /* */

void font_init (void);
void font_done (void);
int hbfOpen (const char *filename);
void hbfClose ();
int open_ascii_font (const char *ascfont_name);
int open_hanzi_font (const char *);
unsigned char *ascGetBitmap (unsigned char c, unsigned char mode);
unsigned char *hbfGetBitmap (unsigned int ch, unsigned char mode,
			     int p_font_bytes);

#define STANDARD         (0x00)
#define UNDERLINE        (0x01)
#define BOLD             (0x02)
#define BOLD_UNDERLINE   (0x03)

#endif /* _JMCCE_HBFFONT_H_ */
