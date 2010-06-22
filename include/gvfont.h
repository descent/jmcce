#ifndef GVFONT
#define GVFONT
//
// gvfont.h
//
/////////////////////////////////////////////////////////////////////////////

#define _MAX_PATH   	255

#define PREFONT		"/usr/local/lib/jmce/"
#define m_stdfont   	PREFONT"stdfont.15"
#define m_spcfont   	PREFONT"spcfont.15"
#define m_supfont   	PREFONT"spcfsupp.15"
#define m_ascfont 	PREFONT"ascfont.15"

FILE *fp_asc;
FILE *fp_std;
FILE *fp_spc;
FILE *fp_sup;

enum fontid			//global enum 
{
  stdfont,
  ascfont,
  spcfont,
  spcfsupp
};

int m_includechinese = 1;
int m_fontheight = 15;		/*16; */
int m_fontwidth = 16;

extern unsigned char m_byBits[32];	//jmt:#1 own chinese font

#endif
