/* hzinput.h */


#ifndef _JMCCE_HZINPUT_H_
#define _JMCCE_HZINPUT_H_

#include <stdio.h>
#include <string>

#include "draw.h"

#define INPUT_CHOICENUMBERCOLOR   15
#define INPUT_CHOICENUMBERBGCOLOR 12
#define INPUT_DEFAULTCOLOR	  13
#define INPUT_FGCOLOR		  GRAY
#define INPUT_BGCOLOR		  BLUE
#define INPUT_WILD_COLOR          12

#define HINT_CASEHICOLOR	  14
#define HINT_CASECOLOR		   9

#define HINT_NOTFOUNDCOLOR	  14

#define LIGHTBAR_MESSAGE	  RED
#define LIGHTBAR_COLOR		  GREEN

#define PHRASE_MATCH_COLOR	  15
#define PHRASE_MATCHPART_COLOR	   7


#define INPUT_AREAY	       ((18 * 24)+10)


#define MAX_INPUT_LENGTH	20
#define MAGIC_NUMBER		"JMCCE"
#define MAX_PHRASE_LENGTH	20
#define END_KEY_LENGTH		16

#define POS_OF_FULLHALF 	 0
#define POS_OF_INPUTNAME	 8
#define POS_OF_ROOTDISPLAY	23
#define POS_OF_VERSIONSTRING	24
#define POS_OF_HISTORYMODE	64
#define POS_OF_WINDOWNO 	71


#define POS_OF_CASEHINT 	60


#define InputAreaX		16

#define MAX_SEL_COUNT		10
#define MAX_SEL_LENGTH		55

//#define VERSION_STRING	  	(PACKAGE VERSION)	/* "JMCCE 1.4 ª©" */


/* key of toggle input method */


typedef struct
{
  unsigned long key1;		/* sizeof(ITEM=12) */
  unsigned long key2;
  unsigned short ch;
  unsigned short frequency;
}
ITEM;



typedef struct
{

  char magic_number[sizeof (MAGIC_NUMBER)];	/* magic number */


  int IsExtInpMethod;
  int EnableAttr;
  int ImmDispAttr;
  int CurImmDispAttr;

  char cname[21];

  int tui_file_pointer[6];


  char ename[24];
  char last_full;
  int MaxPress;
  int AutoSelect;
  int CaseTransPolicy;



  long tui_index_table[6][95][95];


  unsigned char KeyAscii[100];
  unsigned char KeyBig5[200];
  unsigned char KeyAttr[100];
  char selkey[48];
  int selseq[48];

  char ecc[4];

  int TotalKey;			/* number of keys needed */
  int MaxDupSel;		/* how many keys used to select */
  int TotalChar;		/* Defined characters */

  char KeyMap[128];	/* Map 128 chars to 64(6 bit) key index */
  char KeyName[64];	/* Map 64 key to 128 chars */
  short KeyIndex[64];	/* 64 key first index of TotalChar */

  int PhraseNum;		/* Total Phrase Number */

  FILE *PhraseFile;		/* *.tab.phr Phrase File */
  FILE *AssocFile;		/* *.tab.lx LianXiang File */
  ITEM *item;			/* item */

  int TableEncode;
  int ChoiceCharByPhrase;
  char Encode[5];


}
INPUT_TABLE_STRU;


typedef struct
{
  char szLoadno;
  char szFullName[50];
  char szEncode[5];
  char szSimpleName[20];
  char szTuiFile[6][30];
  char szTxtFile[30];
  char szIktFile[30];
  char szRotFile[30];
  char szImmDispAttr[5];

}
INI_FILE_STRU;




/************ private functions *******************/

void input_clear_line (int y, int color);
void input_draw_ascii (int x, int y, unsigned char c, int fg, int bg);
void input_print_string (int x, int y, const unsigned char *string, int fg, int bg);
void input_print_wc(int x, int y, wchar_t wc, int fg, int bg);
void ClrRootArea (void);
void ClrSelArea (void);
void FindMatchKey (void);
void FillMatchChars (int j);
void FillAssociateChars (int index);
void FindAssociateKey (int index);


/************ public functions *********************/



INPUT_TABLE_STRU *IntCode_Init (void);
INPUT_TABLE_STRU *TUItable_load (int p_TUI_row_no);


void hz_input_done (void);
void toggle_input_method (void);
void toggle_half_full (void);
void set_active_input_method (int);
void refresh_input_method_area (void);
void DispSelection (void);


extern char *tabfname[10];

void intcode_hz_filter (int tty_fd, unsigned char key);
void IntCode_FindMatchKey (void);
void IntCode_FillMatchChars (int index);
void hz_input_init (void);
void hz_filter (int tty_fd, unsigned char key);

int BIG5toGB (unsigned char FirstByte, unsigned char SecondByte);
int GBtoBIG5 (unsigned char FirstByte, unsigned char SecondByte);
int BIG5RADICALtoGB (unsigned char FirstByte, unsigned char SecondByte);

char * string_BIG5toGB (const char *p_big5_string);
char * string_BIG5RADICALtoGB (const char *p_big5_string);
char * string_GBtoBIG5 (const char *p_gb_string);
void outchar (int tty_fd, unsigned char c);

void FANJIAN_write (int tty_fd, char *pPhrase, int pPhraseLen);

#endif /* _JMCCE_HZINPUT_H_ */
