
/****************************************************************************/
/*            JMCCE  - hzinput.c                                            */
/*                                                                          */
/*            		$Id: hzinput.c,v 1.1.1.1 2002/05/03 04:01:07 kids Exp $ */
/****************************************************************************/

//#define POS_OF_VERSIONSTRING 0
#define VERSION_STRING "descent modify jmcce 1.4 rc2 80x24"
//#define INPUT_FGCOLOR 0
//#define INPUT_BGCOLOR 0

#include "config.h"
#include "newimp.h"

#include <string>

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <ncurses.h>
#include <ctype.h>
#include <vga.h>
#include <vgagl.h>

#include "jmcce.h"
#include "hzinput.h"
#include "draw.h"
#include "hztty.h"
#include "error.h"
#include "hbffont.h"
#include "c_routine.h"
#include "fb.h"

#include "gb2big.h"
#include "big2gb.h"



/***************************************************************************
 *		 extern      variable defines				   *
 ***************************************************************************/

extern GraphicsContext *physical_screen;
extern int use_fb;
extern int gFont_bytes;
extern int gEncode;
extern int gDontLoadInputMethod;

extern INPUT_TABLE_STRU *NEW_InputTable_load (int p_row_no);
extern void NEW_hz_filter (int tty_fd, unsigned char key, int pListRightNow);
extern void NEWRootEmpty ();
extern void NEWInputInit ();

extern char home_dir[];

void BarMenuInit (int xmax, int ymax);
void BarMenu (int xmax, int ymax, int leftmar, int stepwidth);

INPUT_TABLE_STRU *Chew_Init (void);


/***************************************************************************
 *			     variable defines				   *
 ***************************************************************************/




#define BIG5 886
#define GB   86

#define Big5ToGBTblSize (sizeof(Big5ToGBTbl)/sizeof(int))
#define GBToBig5TblSize (sizeof(GBToBig5Tbl)/sizeof(int))

#define NORMAL 1
#define REVERSE 2
#define NORMALDISABLE -1
#define REVERSEDISABLE -2


extern int mx, my;

extern char Item_str[81][2][7];
extern int Item_attr[81];



INI_FILE_STRU gsRow_of_INI_file[99];

INPUT_TABLE_STRU *gsInput_table_array[80];
INPUT_TABLE_STRU *cur_table = NULL;
INPUT_TABLE_STRU *gsCurrent_input_table = NULL;

int gSelectingMode_flag = 0;
int gsCurrent_method;
FILE *gMessage_log_file;

int IsHanziInput;
static int IsFullChar;


int intcode = 0;


/***************************************************************************
 *			     一些文字常數				   *
 ***************************************************************************/

const char *half_full_strBIG5[] = { "【半形】", "【全形】" };
const char *half_full_strGB[] = { "▽圉褒▼", "▽�娃ョ�" };
const char *history_normal_strBIG5[] = { "【輸入】", "【歷史】" };
const char *history_normal_strGB[] = { "▽怀�諢�", "▽盪妢▼" };

unsigned char fullcharBIG5[] =
  "　！”＃＄％＆’（）＊＋，－．／０１２３４５６７８９：﹔＜＝＞？"
  "＠ＡＢＣＤＥＦＧＨＩＪＫＬＭＮＯＰＱＲＳＴＵＶＷＸＹＺ〔＼〕︿□"
  "‘ａｂｃｄｅｆｇｈｉｊｋｌｍｎｏｐｑｒｓｔｕｖｗｘｙｚ﹛︱﹜～  ";
unsigned char fullcharGB[] =
  "﹛ㄐ§ㄒ∠ㄔㄕ＊ㄗㄘㄙㄚㄛㄜㄝㄞㄟㄠㄡㄢㄣㄤㄥㄦㄧㄨㄩ˙ˉˊˇˋ"
  "�壓薜瞿瓊耀籟ゞラ��氿呁芄怗礿峉洷苺悈猀茛啥掁痑蛅堧晻琚產隉都煄�"
  "＆€�滹聜銫憯璉蝤鋆憌磩諴鴥瞨謑鵃艞韗礗鞶穬鶜鬕驐齮灥�������‵  ";


static char seltab[16][MAX_PHRASE_LENGTH];

static int CurSelNum = 0;	/* Current Total Selection Number */
static unsigned long InpKey[MAX_INPUT_LENGTH];


   /* Input key buffer */

int gInputCount = 0, InputMatch, StartKey, EndKey;
static int save_StartKey, save_EndKey, save_gMultiPageMode,
  save_gNextPageIndex, save_gCurrentPageIndex;


int gNextPageIndex;
int gCurrentPageIndex;
int gMultiPageMode;

int gItem_count;
int gItem_disp_off;








static unsigned long val1, val2, key1, key2;
static int IsAssociateMode;
static int CharIndex[15];

char *tabfname[10] = { NULL };



int UseAssociateMode = 1;




static unsigned long mask[] = {
  0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000, 0x00000000,
  0x3F000000, 0x3FFC0000, 0x3FFFF000, 0x3FFFFFC0, 0x3FFFFFFF, 0x3FFFFFFF,
  0x3FFFFFFF, 0x3FFFFFFF, 0x3FFFFFFF, 0x3FFFFFFF, 0x3FFFFFFF, 0x3FFFFFFF
};

/***************************************************************************
 *	       private function(low level draw functions)		   *
 ***************************************************************************/



/* Some screen fonts for draw the ascii/hanzi in the input area */



/* y=input line(0/1)	clear the line with color */
inline void
input_clear_line (int y, int color)
{
  c_clear_lines (INPUT_AREAY + y * LINE_HEIGHT, LINE_HEIGHT, color);
}


/* clear the whole input area with color */
inline void
input_clear_area (int color)
{
  c_clear_lines (INPUT_AREAY - 6, 480 - INPUT_AREAY + 6, color);
}


inline void
input_draw_ascii (int x, int y, unsigned char c, int fg, int bg)
{
  //c_draw_ascii (x, INPUT_AREAY + y * LINE_HEIGHT, &ascii_font[c][0], (bg << 8) | fg);
  vgalib_draw_ascii (x, INPUT_AREAY + y * LINE_HEIGHT, &ascii_font[c][0], fg, bg);
}


inline void
input_draw_hanzi (int x, int y, unsigned char *bitmap, int fg, int bg)
{
  //c_draw_hanzi (x, INPUT_AREAY + y * LINE_HEIGHT, bitmap, fg | (bg << 8));
  vgalib_draw_hanzi (x, INPUT_AREAY + y * LINE_HEIGHT, bitmap, fg, bg);
}



void
input_print_string (int x, int y, const unsigned char *string, int fg, int bg)
{

  unsigned char *bitmap;

  while (*string) {

    bitmap =
      hbfGetBitmap ((*string << 8) + *(string + 1), STANDARD, gFont_bytes);

    if (bitmap) {

      input_draw_hanzi (x, y, bitmap, fg, bg);

      x += 2;

      string += 2;

    }

    else
      input_draw_ascii (x++, y, *string++, fg, bg);

  }

}





/***************************************************************************
 *				public function 			   *
 ***************************************************************************/

int GBtoBIG5 (unsigned char FirstByte, unsigned char SecondByte)
{
  int rOutChar;
  int index;

  rOutChar = 0;

  index = (FirstByte - 0xA1) * 0x5E + (SecondByte - 0xA1);

  if (index >= 0 && index < GBToBig5TblSize) {
    rOutChar = GBToBig5Tbl[index];
  }

  if (rOutChar == 0)
    rOutChar = 0xA1BD;

  return rOutChar;
}

int BIG5toGB (unsigned char FirstByte, unsigned char SecondByte)
{
  int rOutChar;
  int index;

  rOutChar = 0;

  index = (FirstByte - 0xA1) * 0x9D + SecondByte;

  if (SecondByte < 0xA1)
    index -= 0x40;
  else
    index -= 0x62;

  if (index >= 0 && index < Big5ToGBTblSize) {
    rOutChar = Big5ToGBTbl[index];
  }

  if (rOutChar == 0)
    rOutChar = 0xA1F6;


  return rOutChar;
}


int BIG5RADICALtoGB (unsigned char FirstByte, unsigned char SecondByte)
{
  int rOutChar;
  int index;
  int tRadicalCode;

  rOutChar = 0;

  tRadicalCode = (int) FirstByte *256 + SecondByte;

  switch (tRadicalCode) {
  case 0x9CCB:
    rOutChar = 0xDAA2;
    break;
  case 0x9CCC:
    rOutChar = 0xE5B2;
    break;
  case 0x9CCD:
    rOutChar = 0xDAE0;
    break;
  case 0x9CCF:
    rOutChar = 0xC4B8;
    break;
  case 0x9CD1:
    rOutChar = 0xA1F5;
    break;
  case 0x9CD6:
    rOutChar = 0xD8BC;
    break;
  case 0x9CDA:
    rOutChar = 0xA5B3;
    break;
  case 0x9CDF:
    rOutChar = 0xA3CC;
    break;
  case 0x9CE0:
    rOutChar = 0xE5C1;
    break;
  case 0x9CE1:
    rOutChar = 0xDBC8;
    break;
  case 0x9CE2:
    rOutChar = 0xD8E7;
    break;
  case 0x9CE4:
    rOutChar = 0xA9B7;
    break;
  case 0x9CE5:
    rOutChar = 0xD8AF;
    break;
  case 0x9CE8:
    rOutChar = 0xA5D5;
    break;
  case 0x9CE9:
    rOutChar = 0xDBCC;
    break;
  case 0x9CF1:
    rOutChar = 0xDAE2;
    break;
  case 0x9CF2:
    rOutChar = 0xDED0;
    break;
  case 0x9CF4:
    rOutChar = 0xD8E9;
    break;
  case 0x9CF5:
    rOutChar = 0xE2E0;
    break;
  case 0xA278:
    rOutChar = 0xD8AD;
    break;
  case 0xA2CD:
    rOutChar = 0xDCB3;
    break;
  case 0xA376:
    rOutChar = 0xA8C7;
    break;
  case 0xA377:
    rOutChar = 0xA8C8;
    break;
  case 0xA37E:
    rOutChar = 0xB3A7;
    break;
  case 0xA3A5:
    rOutChar = 0xE1DC;
    break;
  case 0xA449:
    rOutChar = 0xB6F9;
    break;
  case 0xA47B:
    rOutChar = 0xDEC3;
    break;
  case 0xC940:
    rOutChar = 0xA1C1;
    break;

  default:
    {
      index = (FirstByte - 0xA1) * 0x9D + SecondByte;

      if (SecondByte < 0xA1)
	index -= 0x40;
      else
	index -= 0x62;

      if (index >= 0 && index < Big5ToGBTblSize) {
	rOutChar = Big5ToGBTbl[index];
      }
    }
    break;
  }

  return rOutChar;
}


char * string_BIG5toGB (const char *p_big5_string)
{

  int i;
  int r_gb_widechar;
  static char r_gb_string[256];

  for (i = 0; i <= strlen (p_big5_string); i++) {

    if (p_big5_string[i] >= 0xA1) {

      r_gb_widechar = BIG5toGB (p_big5_string[i], p_big5_string[i + 1]);

      r_gb_string[i] = HIBYTE (r_gb_widechar);
      r_gb_string[i + 1] = LOBYTE (r_gb_widechar);

      i++;

    } else {
      r_gb_string[i] = p_big5_string[i];
    }

  }

  return r_gb_string;

}


char * string_GBtoBIG5 (const char *p_gb_string)
{

  int i;
  int r_big5_widechar;
  static char r_big5_string[256];

  for (i = 0; i <= strlen (p_gb_string); i++) {

    if (p_gb_string[i] >= 0xA1) {

      r_big5_widechar = GBtoBIG5 (p_gb_string[i], p_gb_string[i + 1]);

      r_big5_string[i] = HIBYTE (r_big5_widechar);
      r_big5_string[i + 1] = LOBYTE (r_big5_widechar);

      i++;

    } else {
      r_big5_string[i] = p_gb_string[i];
    }

  }

  return r_big5_string;
}


char * string_BIG5RADICALtoGB (const char *p_big5_string)
{

  int i;
  int r_gb_widechar;
  static char r_gb_string[256];

  for (i = 0; i <= strlen (p_big5_string); i++) {

    if (p_big5_string[i] >= 0x9C) {

      r_gb_widechar =
	BIG5RADICALtoGB (p_big5_string[i], p_big5_string[i + 1]);

      r_gb_string[i] = HIBYTE (r_gb_widechar);
      r_gb_string[i + 1] = LOBYTE (r_gb_widechar);

      i++;

    } else {
      r_gb_string[i] = p_big5_string[i];
    }

  }

  return r_gb_string;

}





INPUT_TABLE_STRU *
load_input_method (const char *filename)
{
  int nread;
  FILE *fd;
  char phrase_filename[100], assoc_filename[100];
  INPUT_TABLE_STRU *table;

  table = (INPUT_TABLE_STRU*)malloc (sizeof (INPUT_TABLE_STRU));

  if (table == NULL) {
    fprintf (stderr, "%c", 0x7);
    exit (0);
  }
//    out_of_memory(__FILE__, "load_input_method", __LINE__);

  fd = fopen (filename, "r");

  if (fd == NULL) {
    error ("error: Cannot open input method %s", filename);
    fclose (fd);
    free (table);
    return NULL;
  }
  nread = fread (table, sizeof (INPUT_TABLE_STRU), 1, fd);

  if (nread != 1) {
    error ("error: cannot read file header %s", filename);
    return NULL;
  }

  if (strcmp (MAGIC_NUMBER, table->magic_number)) {
    printf ("is not a valid tab file\n\n");

    return NULL;
  }
  table->item = (ITEM *) malloc (sizeof (ITEM) * table->TotalChar);
  if (table->item == NULL) {
    error ("Gosh, can't malloc enough memory");

    return NULL;
  }
  fread (table->item, sizeof (ITEM), table->TotalChar, fd);

  fclose (fd);

  if (table->PhraseNum > 0) {
    strcpy (phrase_filename, filename);
    strcat (phrase_filename, ".phr");
    strcpy (assoc_filename, filename);
    strcat (assoc_filename, ".lx");
    table->PhraseFile = fopen (phrase_filename, "r");
    table->AssocFile = fopen (assoc_filename, "r");

    if (table->PhraseFile == NULL || table->AssocFile == NULL) {
      printf ("Load Phrase/Assoc File error!\n");
      free (table);
      return NULL;
    }
  }
  return table;
}





static void
RootEmpty ()
{

  intcode = 0;

  bzero (InpKey, sizeof (InpKey));
  bzero (seltab, sizeof (seltab));

  gMultiPageMode = gNextPageIndex = gCurrentPageIndex = 0;
  CurSelNum = gInputCount = InputMatch = 0;
  IsAssociateMode = 0;
}





static void
DispRootArea ()
{
  int i;
  int pos;
  char c;

  pos = POS_OF_ROOTDISPLAY;

  for (i = 0; i <= MAX_INPUT_LENGTH; i++) {

    if (i < gInputCount)
      c = gsInput_table_array[gsCurrent_method]->KeyName[InpKey[i]];

    else
      c = ' ';


    input_draw_ascii (pos++, 0, c, INPUT_FGCOLOR, INPUT_BGCOLOR);
  }

}



inline void
ClrSelArea ()
{
  input_clear_line (1, INPUT_BGCOLOR);
}



inline void
ClrRootArea ()
{
  input_clear_line (2, INPUT_BGCOLOR);
}



void
FindAssociateKey (int index)
{
  FILE *fp = cur_table->AssocFile;
  int ofs[2], offset;

  if (index < 0xB0A1) {
    StartKey = EndKey = 0;
    return;
  }

  offset = (index / 256 - 0xB0) * 94 + index % 256 - 0xA1;
  fseek (fp, offset * sizeof (int), SEEK_SET);
  fread (ofs, sizeof (int), 2, fp);
  StartKey = 72 * 94 + 1 + ofs[0];
  EndKey = 72 * 94 + 1 + ofs[1];
}

static void
load_phrase (int phrno, char *tt)
{
  FILE *fp = cur_table->PhraseFile;
  int ofs[2], len;

  fseek (fp, (phrno + 1) * 4, SEEK_SET);
  fread (ofs, 4, 2, fp);
  len = ofs[1] - ofs[0];

  if (len > 128 || len <= 0) {
    error ("phrase error %d\n", len);
    strcpy (tt, "error");
    return;
  }

  ofs[0] += (cur_table->PhraseNum + 1) * 4;
  fseek (fp, ofs[0], SEEK_SET);
  fread (tt, 1, len, fp);
  tt[len] = 0;
}

void FANJIAN_write (int tty_fd, char *pPhrase, int pPhraseLen)
{

  if (gEncode == BIG5) {
    if (gsCurrent_input_table->TableEncode == BIG5) {
      write (tty_fd, pPhrase, pPhraseLen);
    }
    if (gsCurrent_input_table->TableEncode == GB) {
      write (tty_fd, string_GBtoBIG5 (pPhrase), pPhraseLen);
    }
  }
  if (gEncode == GB) {
    if (gsCurrent_input_table->TableEncode == GB) {
      write (tty_fd, pPhrase, pPhraseLen);
    }
    if (gsCurrent_input_table->TableEncode == BIG5) {
      write (tty_fd, string_BIG5toGB (pPhrase), pPhraseLen);
    }
  }


}




void
putstr (int tty_fd, const char *p)
{
  int len = strlen (p);

  if (len != 0)
    write (tty_fd, p, len);

  ClrSelArea ();
  ClrRootArea ();

  RootEmpty ();
  DispRootArea ();
}



void outchar (int tty_fd, unsigned char c)
{

  int key;

  if (IsFullChar && c >= ' ' && c <= 127) {
    key = (c - ' ') << 1;
    if (gEncode == BIG5) {
      write (tty_fd, fullcharBIG5 + key, 1);
      write (tty_fd, fullcharBIG5 + key + 1, 1);
    }
    if (gEncode == GB) {
      write (tty_fd, fullcharGB + key, 1);
      write (tty_fd, fullcharGB + key + 1, 1);
    }
  } else
    write (tty_fd, &c, 1);

}





void
FindMatchKey (void)
{

  save_StartKey = StartKey;

  save_EndKey = EndKey;
  save_gMultiPageMode = gMultiPageMode;
  save_gNextPageIndex = gNextPageIndex;
  save_gCurrentPageIndex = gCurrentPageIndex;



  val1 =
    InpKey[4] | (InpKey[3] << 6) | (InpKey[2] << 12) | (InpKey[1] << 18) |
    (InpKey[0] << 24);

  val2 =
    InpKey[9] | (InpKey[8] << 6) | (InpKey[7] << 12) | (InpKey[6] << 18) |
    (InpKey[5] << 24);



  if (gInputCount == 1)
    StartKey = cur_table->KeyIndex[InpKey[0]];
  else
    StartKey = CharIndex[gInputCount - 1];

  EndKey = cur_table->KeyIndex[InpKey[0] + 1];



  for (; StartKey < EndKey; StartKey++) {
    key1 = (cur_table->item[StartKey].key1 & mask[gInputCount + 5]);
    key2 = (cur_table->item[StartKey].key2 & mask[gInputCount]);

    if (key1 > val1)
      break;
    if (key1 < val1)
      continue;
    if (key2 < val2)
      continue;
    break;
  }
  CharIndex[gInputCount] = StartKey;
}



void
FillAssociateChars (int index)
{

  char str[25];
  int PhraseNo, CurLen = 0;

  CurSelNum = 0;
  while (CurSelNum < cur_table->MaxDupSel && index < EndKey &&
	 CurLen < MAX_SEL_LENGTH) {

    fseek (cur_table->AssocFile, index << 2, SEEK_SET);
    fread (&PhraseNo, sizeof (int), 1, cur_table->AssocFile);
    load_phrase (PhraseNo, str);
    strcpy (seltab[CurSelNum], str + 2);
    CurLen += strlen (seltab[CurSelNum++]);
    index++;
  }

  if (index < EndKey && CurSelNum == cur_table->MaxDupSel) {
    gNextPageIndex = index;
    gMultiPageMode = 1;
  } else if (gMultiPageMode) {
    gNextPageIndex = StartKey;
  } else
    gMultiPageMode = 0;
}


void
FillMatchChars (int j)
{

  int SelNum = 0, CurLen = 0;

  while ((cur_table->item[j].key1 & mask[gInputCount + 5]) == val1 &&
	 (cur_table->item[j].key2 & mask[gInputCount]) == val2 &&
	 SelNum < cur_table->MaxDupSel && j < EndKey &&
	 CurLen < MAX_SEL_LENGTH) {
    if (cur_table->item[j].ch < 0xA1A1)
      load_phrase (cur_table->item[j].ch, seltab[SelNum]);
    else {
      memcpy (&seltab[SelNum], &(cur_table->item[j].ch), 2);
      seltab[SelNum][2] = '\0';
    }
    CurLen += strlen (seltab[SelNum++]);
    j++;
  }

  if (SelNum == 0) {		/* some match found */
    StartKey = save_StartKey;
    EndKey = save_EndKey;
    gMultiPageMode = save_gMultiPageMode;
    gNextPageIndex = save_gNextPageIndex;
    gCurrentPageIndex = save_gCurrentPageIndex;
    return;			/* keep the original selection */
  }

  CurSelNum = SelNum;

  for (SelNum = CurSelNum; SelNum < 16; SelNum++)
    seltab[SelNum][0] = '\0';	/* zero out the unused area */
  InputMatch = gInputCount;	/* until now we have some matches */

  /* check if more than one page */
  if (j < EndKey && (cur_table->item[j].key1 & mask[gInputCount + 5]) == val1
      && (cur_table->item[j].key2 & mask[gInputCount]) == val2
      && CurSelNum == cur_table->MaxDupSel) {
    /* has another matched key, so enter gMultiPageMode, has more pages */
    gNextPageIndex = j;
    gMultiPageMode = 1;
  } else if (gMultiPageMode) {
    gNextPageIndex = StartKey;	/* rotate selection */
  } else
    gMultiPageMode = 0;
}






void hz_filter (int tty_fd, unsigned char key)
{
  if (!IsHanziInput)
    return outchar (tty_fd, key);

  if (gsCurrent_method == 0) {
    return intcode_hz_filter (tty_fd, key);
  } 
#ifdef CHEWING
  else if (gsCurrent_method == 7) 
  {
    return chewing_hz_filter (tty_fd, key);	/* Chewing */
  } 
#endif
  else {
    return NEW_hz_filter (tty_fd, key, gsCurrent_input_table->CurImmDispAttr);
  }
}



void
hz_filter1 (int tty_fd, unsigned char key)
{
  int inkey = 0, vv;
  char *is_sel_key = (char *) 0;

  if (!IsHanziInput)
    return outchar (tty_fd, key);

  if (gsCurrent_method == 0)
    return intcode_hz_filter (tty_fd, key);

  switch (key) {
  case '\010':			/* BackSpace Ctrl+H */
  case '\177':			/* BackSpace */
    if (gInputCount > 0) {
      InpKey[--gInputCount] = 0;
      if (gInputCount == 0) {
	RootEmpty ();
	ClrSelArea ();
      } else if (gInputCount < InputMatch) {
	FindMatchKey ();
	gMultiPageMode = 0;
	gCurrentPageIndex = StartKey;
	FillMatchChars (StartKey);
	DispSelection ();
      }
      DispRootArea ();
    } else
      outchar (tty_fd, key);
    break;

  case '\033':			/* ESCAPE */
    if (gInputCount > 0) {
      ClrSelArea ();
      ClrRootArea ();

      RootEmpty ();
      DispRootArea ();

    } else
      outchar (tty_fd, key);
    break;

  case '<':
  case '-':
  case ',':
  case '[':
    if (gMultiPageMode) {
      if (gCurrentPageIndex > StartKey)
	gCurrentPageIndex = gCurrentPageIndex - cur_table->MaxDupSel;
      else
	gCurrentPageIndex = StartKey;
      if (IsAssociateMode)
	FillAssociateChars (gCurrentPageIndex);
      else
	FillMatchChars (gCurrentPageIndex);
      DispSelection ();
    } else
      outchar (tty_fd, key);
    break;

  case '>':
  case ']':
  case '.':
  case '=':
    if (gMultiPageMode) {
      gCurrentPageIndex = gNextPageIndex;
      if (IsAssociateMode)
	FillAssociateChars (gCurrentPageIndex);
      else
	FillMatchChars (gCurrentPageIndex);
      DispSelection ();
    } else
      outchar (tty_fd, key);
    break;

  case ' ':
    if (CurSelNum == 0)
      outchar (tty_fd, key);
    if (seltab[0][0])
      putstr (tty_fd, seltab[0]);
    break;

  default:

    inkey = cur_table->KeyMap[key];
    is_sel_key = strchr (cur_table->selkey, key);
    vv = is_sel_key - cur_table->selkey;

    if ((!inkey && !is_sel_key) ||
	(!inkey && is_sel_key && (CurSelNum == 0 || seltab[vv][0] == 0))) {
      IsAssociateMode = 0;

      RootEmpty ();
      ClrSelArea ();
      ClrRootArea ();
      outchar (tty_fd, key);
      return;
    }

    if (is_sel_key && CurSelNum > 0 && seltab[vv][0]) {
      putstr (tty_fd, seltab[vv]);
      return;
    }

    /* now it must be inkey? */
    IsAssociateMode = 0;
    if (inkey >= 1 && gInputCount < MAX_INPUT_LENGTH)
      InpKey[gInputCount++] = inkey;

    if (gInputCount <= InputMatch + 1) {
      FindMatchKey ();
      gCurrentPageIndex = StartKey;
      gMultiPageMode = 0;
      FillMatchChars (StartKey);
      if (gInputCount >= cur_table->MaxPress && CurSelNum == 1 && cur_table->last_full) {	// left only one selection
	return putstr (tty_fd, seltab[0]);
      }
      DispSelection ();
    }
    DispRootArea ();
    break;

  }				/* switch */
}




void
DispSelection ()
{

  int i, pos = 1, len;



  ClrSelArea ();

  if (gMultiPageMode && gCurrentPageIndex != StartKey) {
    input_print_string (pos, 1, "< ", INPUT_FGCOLOR, INPUT_BGCOLOR);

    pos += 2;
  }

  for (i = 0; i < CurSelNum; i++) {
    if (!seltab[i][0]) {
      if (gsCurrent_method == 0 && i == 0)
	continue;
      else
	break;
    }

    input_draw_ascii (pos++, 1,
		      gsInput_table_array[gsCurrent_method]->selkey[i],
		      INPUT_FGCOLOR, INPUT_BGCOLOR);

    len = strlen (seltab[i]);
    input_print_string (pos, 1, seltab[i], INPUT_FGCOLOR, INPUT_BGCOLOR);
    pos += len + 1;
  }

  if (gMultiPageMode && gNextPageIndex != StartKey) {
    input_print_string (pos, 1, "> ", INPUT_FGCOLOR, INPUT_BGCOLOR);
  }
}


/*************************************************************************

 *			  public function				 *

 *************************************************************************/



void
load_gsInput_table_array (int i, const char *filename)
{
  gsInput_table_array[i] = load_input_method (filename);
}


void
free_input_method (INPUT_TABLE_STRU * table)
{
  free (table);
}


/* for cce only */
void
unload_gsInput_table_array (int i)
{
  if (((i >= 0) && (i < 10)) && gsInput_table_array[i]) {
    if (i == gsCurrent_method) {
      if (IsHanziInput)
	toggle_input_method ();
      gsCurrent_method = 0;
    }
    free_input_method (gsInput_table_array[i]);
    gsInput_table_array[i] = NULL;
  }
}

void
toggle_input_method (void)
{
  if (gsInput_table_array[gsCurrent_method]) {
    IsHanziInput ^= 1;

    gsCurrent_input_table = gsInput_table_array[gsCurrent_method];
//  RootEmpty();

    refresh_input_method_area ();

  }
}



void
toggle_half_full (void)
{

  IsFullChar = 1 - IsFullChar;
  refresh_input_method_area ();
}


void
INIfile_load (char *pINI_file_name)
{

  FILE *temp_fp;
  char temp_line[500];
  int temp_Loadno;
  int j;
  int tSymbol_line_count;

  tSymbol_line_count = 11;


  for (j = 1; j <= 9; j++) {
    strcpy (gsRow_of_INI_file[j].szTuiFile[1], "...");
    strcpy (gsRow_of_INI_file[j].szFullName, "");

    strcpy (gsRow_of_INI_file[j].szSimpleName, "____");
  }

  temp_fp = fopen (pINI_file_name, "r");
  if (temp_fp == NULL) {
    fprintf (stderr, "%c Error! Cannot find file: %s\n", 0x7, pINI_file_name);
    exit (0);
  }

  while (fscanf (temp_fp, "%s", temp_line) != EOF) {

    if (temp_line[0] == '\073') {
      fgets (temp_line, 400, temp_fp);
      continue;
    }

    if (temp_line[0] != 'S' && temp_line[0] != 'Y') {
      temp_Loadno = (int) temp_line[0] - 48;
    } else {
      temp_Loadno = tSymbol_line_count++;
    }

    gsRow_of_INI_file[temp_Loadno].szLoadno = (char) temp_Loadno;

    if (fscanf (temp_fp, "%s", gsRow_of_INI_file[temp_Loadno].szFullName) ==
	EOF)
      break;

    if (fscanf (temp_fp, "%s", gsRow_of_INI_file[temp_Loadno].szEncode) ==
	EOF)
      break;
    if (fscanf (temp_fp, "%s", gsRow_of_INI_file[temp_Loadno].szSimpleName) ==
	EOF)
      break;

    if (fscanf (temp_fp, "%s", gsRow_of_INI_file[temp_Loadno].szTuiFile[1]) ==
	EOF)
      break;
    if (fscanf (temp_fp, "%s", gsRow_of_INI_file[temp_Loadno].szTuiFile[2]) ==
	EOF)
      break;
    if (fscanf (temp_fp, "%s", gsRow_of_INI_file[temp_Loadno].szTuiFile[3]) ==
	EOF)
      break;
    if (fscanf (temp_fp, "%s", gsRow_of_INI_file[temp_Loadno].szTuiFile[4]) ==
	EOF)
      break;
    if (fscanf (temp_fp, "%s", gsRow_of_INI_file[temp_Loadno].szTuiFile[5]) ==
	EOF)
      break;

    if (fscanf (temp_fp, "%s", gsRow_of_INI_file[temp_Loadno].szTxtFile) ==
	EOF)
      break;
    if (fscanf (temp_fp, "%s", gsRow_of_INI_file[temp_Loadno].szIktFile) ==
	EOF)
      break;
    if (fscanf (temp_fp, "%s", gsRow_of_INI_file[temp_Loadno].szRotFile) ==
	EOF)
      break;
    if (fscanf (temp_fp, "%s", gsRow_of_INI_file[temp_Loadno].szImmDispAttr)
	== EOF)
      break;


  }

  gItem_count = tSymbol_line_count - 1;

  fclose (temp_fp);
}



void hz_input_init (void)
{
  int i, len;
  char fn[128];

  IsHanziInput = 0;		// default is ascii input
  IsFullChar = 0;
  gsCurrent_method = 1;

  std::string jmcce_conf_path(jmcce_path);

  if (gDontLoadInputMethod != 1) {

    if (gEncode == BIG5) {
      INIfile_load((std::string(jmcce_conf_path + "/../conf/jmcceb5.ini")).c_str());
    }
    #if 0
    if (gEncode == GB) {
      INIfile_load (JMCCE_CONFDIR"/jmccegb.ini");
    }
    #endif

    sprintf (fn, "%s/%s", home_dir, JMCCE_LOG);

    if ((gMessage_log_file = fopen (fn, "w+")) == NULL) {
      fprintf (stderr, "Warning: can't open log file\n");
      return;
    }

    for (i = 0; i <= gItem_count; i++)
      gsInput_table_array[i] = NULL;

    gsInput_table_array[0] = IntCode_Init ();

    for (i = 1; i <= gItem_count; i++) {
      gsInput_table_array[i] = NEW_InputTable_load (i);
    }
    //exit(0);

#ifdef CHEWING
    gsInput_table_array[7] = Chew_Init ();	/* Chewing */
#endif
    fclose (gMessage_log_file);


  }



}



void
hz_input_init1 (void)
{
  int i, len;
  char fn[128];

  IsHanziInput = 0;		// default is ascii input
  IsFullChar = 0;
  gsCurrent_method = 1;
  gsInput_table_array[0] = IntCode_Init ();

  for (i = 1; i < 10; i++)
    if (tabfname[i] != NULL) {
      //fn[strlen(fn) - 1] = '0' + i;
      len = strlen (tabfname[i]);
      strcpy (fn, "/usr/lib/jmcce/");
      strcat (fn, tabfname[i]);
      if (len < 4 || strcmp (tabfname[i] + len - 4, ".tab") != 0)
	strcat (fn, ".tab");

      printf ("Input Method %d: %s\n", i, fn);
      if (access (fn, R_OK) == 0)
	gsInput_table_array[i] = load_input_method (fn);
      else
	gsInput_table_array[i] = NULL;
    }
}

void
hz_input_done (void)
{
  int i;

  for (i = 0; i < 10; i++)
    if (gsInput_table_array[i]) {
      if (gsInput_table_array[i]->IsExtInpMethod)
	NEWtable_unload (i);

    }
}


void
hz_input_done1 (void)
{
  int i;
  for (i = 0; i < 10; i++)
    if (gsInput_table_array[i])
      free_input_method (gsInput_table_array[i]);
}


void
set_active_input_method (int active)
{

  gsCurrent_method = active;
  gsCurrent_input_table = gsInput_table_array[active];

  if (gsCurrent_method != 0)
    UseAssociateMode = 1;
  else
    UseAssociateMode = 0;

  if (!IsHanziInput)
    toggle_input_method ();
  else
    refresh_input_method_area ();
}



/* before call this function, must lock the console */
void
refresh_input_method_area (void)
{
  char str[25];

  int tTopY;
  int tLeftX;
  int ii, jj;

  tTopY = 458;
  tLeftX = 540;

  NEWRootEmpty ();
//NEWDispRootArea();
  input_clear_line (2, INPUT_BGCOLOR);
  gInputCount = 0;
  ClrSelArea ();
  gSelectingMode_flag = 0;


  input_clear_area (INPUT_BGCOLOR);

  if (use_fb) {
    fb_setfgcolor (15);
    fb_drawline (0, INPUT_AREAY - 7, 639, INPUT_AREAY - 7);
    fb_drawline (0, INPUT_AREAY - 4, 639, INPUT_AREAY - 4);
    for (ii = 0; ii < 100; ii++)
      fb_setfgcolor (12);
  } else {

    gl_hline(0, INPUT_AREAY - 7, WIDTH-1, GRAY);
    gl_hline(0, INPUT_AREAY - 4, WIDTH-1, GRAY);

#ifdef VGA__
    vga_setcolor (15);
    vga_drawline (0, INPUT_AREAY - 7, 639, INPUT_AREAY - 7);
    vga_drawline (0, INPUT_AREAY - 4, 639, INPUT_AREAY - 4);
    for (ii = 0; ii < 100; ii++)
      vga_setcolor (12);
#endif
  }


  if (!IsHanziInput) {
    if (gEncode == BIG5) {
      input_print_string (POS_OF_VERSIONSTRING, 0, VERSION_STRING,
			  INPUT_FGCOLOR, INPUT_BGCOLOR);
      input_print_string (POS_OF_INPUTNAME, 0, "【英數】", INPUT_FGCOLOR,
			  INPUT_BGCOLOR);
    }
    if (gEncode == GB) {
      input_print_string (POS_OF_VERSIONSTRING, 0,
			  string_BIG5toGB (VERSION_STRING), INPUT_FGCOLOR,
			  INPUT_BGCOLOR);
      input_print_string (POS_OF_INPUTNAME, 0, string_BIG5toGB ("【英數】"),
			  INPUT_FGCOLOR, INPUT_BGCOLOR);
    }
  } else {

    if ((gsCurrent_input_table->IsExtInpMethod) == 1) {
      if (gEncode == BIG5) {
	input_print_string (POS_OF_INPUTNAME, 0,
			    gsInput_table_array[gsCurrent_method]->cname,
			    INPUT_FGCOLOR, INPUT_BGCOLOR);
      }
      if (gEncode == GB) {
	input_print_string (POS_OF_INPUTNAME, 0,
			    gsInput_table_array[gsCurrent_method]->cname,
			    INPUT_FGCOLOR, INPUT_BGCOLOR);
      }
    } else {
      fprintf (stderr, "%c", 0x7);
      if (gEncode == BIG5) {
	input_print_string (POS_OF_INPUTNAME, 0, "!無輸入法!", INPUT_FGCOLOR,
			    INPUT_BGCOLOR);
      }
      if (gEncode == GB) {
	input_print_string (POS_OF_INPUTNAME, 0,
			    string_BIG5toGB ("!無輸入法!"), INPUT_FGCOLOR,
			    INPUT_BGCOLOR);
      }
    }
  }


  if (gsCurrent_input_table) {
    if (gsCurrent_input_table->CaseTransPolicy == 0) {
      input_print_string (POS_OF_CASEHINT, 0, "A", HINT_CASEHICOLOR,
			  INPUT_BGCOLOR);
      input_print_string (POS_OF_CASEHINT + 1, 0, " ", INPUT_FGCOLOR,
			  INPUT_BGCOLOR);
      input_print_string (POS_OF_CASEHINT + 2, 0, "a", HINT_CASECOLOR,
			  INPUT_BGCOLOR);
    }
    if (gsCurrent_input_table->CaseTransPolicy == 1) {
      input_print_string (POS_OF_CASEHINT, 0, "a", HINT_CASECOLOR,
			  INPUT_BGCOLOR);
      input_print_string (POS_OF_CASEHINT + 1, 0, "=", HINT_CASECOLOR,
			  INPUT_BGCOLOR);
      input_print_string (POS_OF_CASEHINT + 2, 0, "A", HINT_CASECOLOR,
			  INPUT_BGCOLOR);
    }
    if (gsCurrent_input_table->CaseTransPolicy == 2) {
      input_print_string (POS_OF_CASEHINT, 0, "A", HINT_CASECOLOR,
			  INPUT_BGCOLOR);
      input_print_string (POS_OF_CASEHINT + 1, 0, "=", HINT_CASECOLOR,
			  INPUT_BGCOLOR);
      input_print_string (POS_OF_CASEHINT + 2, 0, "a", HINT_CASECOLOR,
			  INPUT_BGCOLOR);
    }
  }


  if (gEncode == BIG5) {
    input_print_string (POS_OF_FULLHALF, 0, half_full_strBIG5[IsFullChar],
			INPUT_FGCOLOR, INPUT_BGCOLOR);
    input_print_string (POS_OF_HISTORYMODE, 0,
			history_normal_strBIG5[console_mode], INPUT_FGCOLOR,
			INPUT_BGCOLOR);
    sprintf (str, "【視窗%1d】", hztty_list->index);
  }
  if (gEncode == GB) {
    input_print_string (POS_OF_FULLHALF, 0, half_full_strGB[IsFullChar],
			INPUT_FGCOLOR, INPUT_BGCOLOR);
    input_print_string (POS_OF_HISTORYMODE, 0,
			history_normal_strGB[console_mode], INPUT_FGCOLOR,
			INPUT_BGCOLOR);
    sprintf (str, "▽敦諳%1d▼", hztty_list->index);
  }
  input_print_string (POS_OF_WINDOWNO, 0, str, INPUT_FGCOLOR, INPUT_BGCOLOR);
  if (IsHanziInput > 0 && gsCurrent_method == 7)
#ifdef CHEWING
    ChewReDrawText ();		/* Chewing */
#else
    ;
#endif

}


void
chinese_bar (int x, int y, int leftmar, int stepwidth, int disp_type,
	     char *prompt)
{
  int tFontColor;
  int tBackGroundColor;

  switch (disp_type) {
  case NORMAL:
    tFontColor = LIGHTBAR_COLOR;
    tBackGroundColor = INPUT_BGCOLOR;
    break;
  case NORMALDISABLE:
    tFontColor = MAGENTA;
    tBackGroundColor = INPUT_BGCOLOR;
    break;
  case REVERSE:
    tFontColor = INPUT_BGCOLOR;
    tBackGroundColor = LIGHTBAR_COLOR;
    break;
  case REVERSEDISABLE:
    tFontColor = 7;
    tBackGroundColor = LIGHTBAR_COLOR;
    break;
  }

  input_print_string (leftmar + (x - 1) * stepwidth, y, Item_str[x][y],
		      tFontColor, tBackGroundColor);
}

void
Up (int xmax, int ymax)
{

  if (mx == 1 && my == 1) {
    mx = xmax;
    my = ymax;
  } else {
    if (my > 1)
      my = my - 1;
    else {
      my = ymax;
      mx = mx - 1;
    }
  }
}


void
Dn (int xmax, int ymax)
{

  if (mx == xmax && my == ymax) {
    mx = 1;
    my = 1;
  } else {
    if (my < ymax)
      my = my + 1;
    else {
      my = 1;
      mx = mx + 1;
    }
  }
}



void
Lf (int xmax, int ymax)
{

  if (mx == 1 && my == 1) {
    mx = xmax;
    my = ymax;
  } else {
    if (mx > 1)
      mx = mx - 1;
    else {
      mx = xmax;
      my = my - 1;
    }
  }
}


void
Rt (int xmax, int ymax)
{

  if (mx == xmax && my == ymax) {
    mx = 1;
    my = 1;
  } else {
    if (mx < xmax)
      mx = mx + 1;
    else {
      mx = 1;
      my = my + 1;
    }
  }
}




void
BarRight (int xmax, int ymax, int leftmar, int stepwidth)
{
  int jj;
  int ii;

  char k[20];


  ii = ymax;


  if (mx == xmax && my == ymax) {


    if (gItem_disp_off < gItem_count - 10) {

      gItem_disp_off++;



      for (jj = 1; jj <= xmax; jj++) {
	strcpy (Item_str[jj][ii], " ");
	if (gEncode == BIG5) {
	  strcat (Item_str[jj][ii],
		  ((jj + gItem_disp_off) ==
		   10) ? "內碼" : gsRow_of_INI_file[jj +
						    gItem_disp_off].
		  szSimpleName);
	}

	if (gEncode == GB) {
	  strcat (Item_str[jj][ii],
		  ((jj + gItem_disp_off) ==
		   10) ? string_BIG5toGB ("內碼") : (const char
						     *) (gsRow_of_INI_file[jj
									   +
									   gItem_disp_off].
							 szSimpleName));
	}
	strcat (Item_str[jj][ii], " ");

	Item_attr[jj] =
	  ((jj + gItem_disp_off) ==
	   10) ? 1 : gsInput_table_array[jj + gItem_disp_off]->EnableAttr;



      }
    } else {
      beep ();
      mx = 1;
      my = 1;
      BarMenuInit (10, 1);
    }


    for (jj = 1; jj <= xmax; jj++)
      for (ii = 1; ii <= ymax; ii++) {
	chinese_bar (jj, ii, leftmar, stepwidth,
		     Item_attr[jj] == 1 ? NORMAL : NORMALDISABLE,
		     Item_str[jj][ii]);
      }

    chinese_bar (mx, my, leftmar, stepwidth,
		 Item_attr[mx] == 1 ? REVERSE : REVERSEDISABLE,
		 Item_str[mx][my]);

  } else {
    if (mx < xmax)
      mx = mx + 1;
    else {
      mx = 1;
      my = my + 1;
    }
  }
}


void
BarLeft (int xmax, int ymax, int leftmar, int stepwidth)
{
  int jj;
  int ii;

  ii = ymax;

  if (mx == 1 && my == 1) {

    if (gItem_disp_off > 0) {

      gItem_disp_off--;


      for (jj = 1; jj <= xmax; jj++) {
	strcpy (Item_str[jj][ii], " ");
	if (gEncode == BIG5) {
	  strcat (Item_str[jj][ii],
		  ((jj + gItem_disp_off) ==
		   10) ? "內碼" : gsRow_of_INI_file[jj +
						    gItem_disp_off].
		  szSimpleName);
	}

	if (gEncode == GB) {
	  strcat (Item_str[jj][ii],
		  ((jj + gItem_disp_off) ==
		   10) ? string_BIG5toGB ("內碼") : (const char
						     *) (gsRow_of_INI_file[jj
									   +
									   gItem_disp_off].
							 szSimpleName));
	}
	strcat (Item_str[jj][ii], " ");

	Item_attr[jj] =
	  ((jj + gItem_disp_off) ==
	   10) ? 1 : gsInput_table_array[jj + gItem_disp_off]->EnableAttr;


      }

      for (jj = 1; jj <= xmax; jj++)
	for (ii = 1; ii <= ymax; ii++) {
	  chinese_bar (jj, ii, leftmar, stepwidth,
		       Item_attr[jj] == 1 ? NORMAL : NORMALDISABLE,
		       Item_str[jj][ii]);
	}

      chinese_bar (mx, my, leftmar, stepwidth,
		   Item_attr[mx] == 1 ? REVERSE : REVERSEDISABLE,
		   Item_str[mx][my]);

    }

  } else {
    if (mx > 1) {
      mx = mx - 1;
    }
  }

}



void
BarMenuInit (int xmax, int ymax)
{
  int ii;
  int jj;

  gItem_disp_off = 0;

  for (jj = 1; jj < xmax; jj++)
    for (ii = 1; ii <= ymax; ii++) {
      strcpy (Item_str[jj][ii], " ");
      if (gEncode == BIG5) {
	strcat (Item_str[jj][ii], gsRow_of_INI_file[jj].szSimpleName);

      }

      if (gEncode == GB) {
	strcat (Item_str[jj][ii], gsRow_of_INI_file[jj].szSimpleName);

//            strcat(Item_str[jj][ii],string_BIG5toGB(gsRow_of_INI_file[jj].szSimpleName));
      }

      strcat (Item_str[jj][ii], " ");

      Item_attr[jj] = gsInput_table_array[jj]->EnableAttr;

    }

  if (gEncode == BIG5) {
    strcpy (Item_str[10][1], " 內碼 ");
    strcpy (Item_str[07][1], " 酷音 ");	/* Chewing */
  }

  if (gEncode == GB) {
    strcpy (Item_str[10][1], string_BIG5toGB (" 內碼 "));
    strcpy (Item_str[07][1], string_BIG5toGB (" 酷音 "));
  }

  for (jj = xmax + 1; jj < gItem_count; jj++)
    for (ii = 1; ii <= ymax; ii++) {
      strcpy (Item_str[jj][ii], " ");
      if (gEncode == BIG5) {
	strcat (Item_str[jj][ii], gsRow_of_INI_file[jj].szSimpleName);
      }

      if (gEncode == GB) {
	strcat (Item_str[jj][ii], gsRow_of_INI_file[jj].szSimpleName);

	//     strcat(Item_str[jj][ii],string_BIG5toGB(gsRow_of_INI_file[jj].szSimpleName));
      }

      strcat (Item_str[jj][ii], " ");

      Item_attr[jj] = gsInput_table_array[jj]->EnableAttr;

    }

}



void
BarMenu (int xmax, int ymax, int leftmar, int stepwidth)
{

  int ii, jj;
  int lastx, lasty;
  int key1, key2;
  int i;
  int ok_flag = 0;



  ClrSelArea ();

  if (gEncode == BIG5) {
    input_print_string (0, 1, "請選擇->", LIGHTBAR_MESSAGE, INPUT_BGCOLOR);
  }

  if (gEncode == GB) {
    input_print_string (0, 1, string_BIG5toGB ("請選擇->"), LIGHTBAR_MESSAGE,
			INPUT_BGCOLOR);
  }

  for (jj = 1; jj <= xmax; jj++)
    for (ii = 1; ii <= ymax; ii++) {
      chinese_bar (jj, ii, leftmar, stepwidth,
		   Item_attr[jj] == 1 ? NORMAL : NORMALDISABLE,
		   Item_str[jj][ii]);
    }

  chinese_bar (mx, my, leftmar, stepwidth,
	       Item_attr[mx] == 1 ? REVERSE : REVERSEDISABLE,
	       Item_str[mx][my]);

  gl_copyscreen(physical_screen);

  initscr ();
  noecho ();
  keypad (stdscr, TRUE);


  while (ok_flag == 0) {

    key2 = getch ();
    lastx = mx;
    lasty = my;
    switch (key2) {

    case KEY_HOME:
      mx = 1;
      my = 1;
      break;
    case KEY_END:
      mx = xmax;
      my = ymax;
      break;
    case KEY_UP:
      Up (xmax, ymax);
      break;
    case KEY_LEFT:
      BarLeft (xmax, ymax, leftmar, stepwidth);
      break;
    case KEY_RIGHT:
      BarRight (xmax, ymax, leftmar, stepwidth);
      break;
    case KEY_DOWN:
      Dn (xmax, ymax);
      break;
    case 10:
    case 13:
    case KEY_ENTER:
      ok_flag = 1;
      ClrSelArea ();
      if (mx == 10 && gItem_disp_off == 0) {
	set_active_input_method (0);
      } else {
	set_active_input_method (mx + gItem_disp_off);
      }
      break;
    case 200:
    case 201:
    case 202:
    case 203:
    case 204:
    case 205:
    case 206:
    case 207:
    case 208:
    case 209:
      ok_flag = 1;
      ClrSelArea ();
      if (key2 - 200 == 0) {
	mx = 10;
	my = 1;
	set_active_input_method (0);
      } else {
	mx = key2 - 200;
	my = 1;
	set_active_input_method (mx);
      }
      BarMenuInit (10, 1);
      BarMenu (10, 1, 8, 6);
      break;
    default:
      ok_flag = 1;
      ClrSelArea ();
      if (mx == 10 && gItem_disp_off == 0) {
	set_active_input_method (0);
      } else {
	set_active_input_method (mx + gItem_disp_off);
      }

      hz_filter (hztty_list->tty_fd, key2);
      break;
    }

    if (ok_flag == 1) {
      break;
    }



    chinese_bar (mx, my, leftmar, stepwidth,
		 Item_attr[mx] == 1 ? REVERSE : REVERSEDISABLE,
		 Item_str[mx][my]);

    if (!(lasty == my && lastx == mx)) {
      chinese_bar (lastx, lasty, leftmar, stepwidth,
		   Item_attr[lastx] == 1 ? NORMAL : NORMALDISABLE,
		   Item_str[lastx][lasty]);
    }
    gl_copyscreen(physical_screen);


  }

  keypad (stdscr, FALSE);

}







/*************************************************************************
 *			 Internal Code Input Method			 *
 *************************************************************************/

INPUT_TABLE_STRU *
IntCode_Init (void)
{
  INPUT_TABLE_STRU *table;
  int i, index;

  UseAssociateMode = 1;		/* force to no associate */

  table = (INPUT_TABLE_STRU*)malloc (sizeof (INPUT_TABLE_STRU));
  if (table == NULL)
    out_of_memory (__FILE__, "load_input_method", __LINE__);

  strcpy (table->magic_number, MAGIC_NUMBER);
  strcpy (table->ename, "IntCode");
  if (gEncode == BIG5) {
    strcpy (table->cname, "內碼輸入法:");
  }
  if (gEncode == GB) {
    strcpy (table->cname, string_BIG5toGB ("內碼輸入法:"));
  }

  table->CaseTransPolicy = 1;

  strcpy (table->selkey, "0123456789ABCDEF");

  table->last_full = 1;

  table->IsExtInpMethod = 1;

  Item_attr[10] = 1;

  for (i = 0; i < 128; i++) {
    table->KeyMap[i] = 0xff;	/*!0; */

    if ((i >= '0' && i <= '9') || (i >= 'A' && i <= 'F')) {
      if (i >= '0' && i <= '9')
	index = i - '0';
      else
	index = i - 'A' + 10;

      table->KeyMap[i] = index;

      table->KeyName[index] = toupper (i);

    }
  }
  return table;
}



int
IsBig5 (int i)
{
  if ((i >= 0xa140 && i <= 0xa3bf) ||
      (i >= 0xa440 && i <= 0xc67e) ||
      (i >= 0xc940 && i <= 0xf9d5) ||
      (i >= 0xf9d6 && i <= 0xf9fe) ||
      (i >= 0xfa40 && i <= 0xfefe) ||
      (i >= 0x8e40 && i <= 0xa0fe) ||
      (i >= 0x8140 && i <= 0x8dfe) || (i >= 0xc6a1 && i <= 0xc8fe)
    )
    return 1;
  else
    return 0;
}



void
IntCode_FillMatchChars (int icode)
{
  int i;

  CurSelNum = 0;
  gMultiPageMode = 0;
  if (gInputCount < 2)
    return;

  for (i = 0; i < 16; i++)
    seltab[i][0] = '\0';
  /* zero out the unused area */

  while (CurSelNum < 16) {
    if (IsBig5 (icode)) {
      seltab[CurSelNum][0] = icode / 256;
      seltab[CurSelNum][1] = icode % 256;
      seltab[CurSelNum][2] = '\0';
    }
    CurSelNum++;
    icode++;
  }
}



void
InpKey2intcode ()
{
  int i;
  intcode = 0;

  for (i = 0; i < gInputCount; i++)
    intcode = intcode * 16 + InpKey[i];
}

void
intcode_hz_filter (int tty_fd, unsigned char key)
{
  int inkey = 0;
  char buf[30];

  if (gsCurrent_input_table->CaseTransPolicy == 1)
    key = toupper (key);

  if (gsCurrent_input_table->CaseTransPolicy == 2)
    key = tolower (key);


  switch (key) {
  case '\010':			/* BackSpace Ctrl+H */
  case '\177':			/* BackSpace */
    if (gInputCount > 0) {
      InpKey[--gInputCount] = 0;

      if (gInputCount == 2)
	ClrSelArea ();

      if (gInputCount == 0)
	RootEmpty ();

      DispRootArea ();
    } else
      outchar (tty_fd, key);
    break;

  case '\033':			/* ESCAPE */
    if (gInputCount > 0) {
      ClrSelArea ();
      ClrRootArea ();

      RootEmpty ();
      DispRootArea ();
    } else
      outchar (tty_fd, key);
    break;

  default:

    inkey = gsCurrent_input_table->KeyMap[key];


    switch (gInputCount) {
    case 0:
    case 1:
    case 2:
      if (inkey >= 0) {
	InpKey[gInputCount++] = inkey;
	DispRootArea ();

	InpKey2intcode ();

	if (gInputCount == 3) {
	  gMultiPageMode = 0;
	  IntCode_FillMatchChars (intcode * 16);
	  DispSelection ();
	}

      } else {

	RootEmpty ();
	ClrSelArea ();
	ClrRootArea ();
	outchar (tty_fd, key);
	return;
      }

      break;

    case 3:
      if (seltab[inkey][0] != '\0')
	putstr (tty_fd, seltab[inkey]);
      else {
	RootEmpty ();
	ClrSelArea ();
#if 0
	ClrRootArea ();
#else
	DispRootArea ();
#endif
	outchar (tty_fd, key);
	return;
      }

      break;

    }				/* default */
  }				/* switch */
}
