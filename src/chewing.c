
/****************************************************************************/
/*            JMCCE  - chewing.c                                            */
/*                                                                          */
/*            	$Id: chewing.c,v 1.7 2002/05/17 16:53:01 kids Exp $      */
/****************************************************************************/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>
#include <vga.h>
#include <curses.h>
#include <ctype.h>

#include "jmcce.h"
#include "hzinput.h"
#include "draw.h"
#include "hztty.h"
#include "error.h"
#include "hbffont.h"
#include "c_routine.h"
#include "fb.h"


#include "chewingio.h"
#include "global.h"
#include "char.h"
#include "dict.h"
#include "zuin.h"
#include "hash.h"


/*************************************************************************
 *			 Chewing Input Method			         *
 *************************************************************************/

enum
{ H_NORMAL, H_ESCAPE, H_FUNC_KEY, H_XXX }
h_state;

extern int UseAssociateMode;
extern int gEncode;
extern int gInputCount;
extern int IsHanziInput;

#define KEY_ESC '\033'
#define KEY_BACK '\177'
#define KEY_DEL (126)
#define KEY_SPACE       (32)
#define KEY_TAB '\t'
#define KEY_CTRL_W 23
#define KEY_CTRL_Z 26

ChewingData *pgdata;
ChewingOutput gOut;
ChewingConf Chewing_cf;

int ChewPOS;

/* put the lifetime to chewing shared library -- Cd Chen */
extern int lifetime;

void
Live ()
{
  static time_t nowtime, lasttime = 0;

  nowtime = time (NULL);
  if (lasttime != 0)
    lifetime += (nowtime - lasttime);
  lasttime = nowtime;
}

int
SetChewingConfig ()
{
  ConfigData config;
  int i;

  config.selectAreaLen = 40;
  config.maxChiSymbolLen = 20;
  for (i = 0; i < 9; i++)
    config.selKey[i] = i + '1';
  config.selKey[9] = '0';
  SetConfig (pgdata, &config);
  return 0;
}

void
ChewClrEol (int startx, int y)
{

  for (; startx < 60; startx++)
    input_draw_ascii (startx, y, ' ', INPUT_WILD_COLOR, INPUT_BGCOLOR);

}

void
ChewShowTextArea (char *out)
{
  ChewClrEol (ChewPOS, 0);
  input_print_string (ChewPOS, 0, out, INPUT_WILD_COLOR, INPUT_BGCOLOR);
}

void
ChewShowChooseArea (char *out)
{
  ChewClrEol (ChewPOS, 1);
  input_print_string (ChewPOS, 1, out, INPUT_WILD_COLOR, INPUT_BGCOLOR);
}

/* 顯示鍵盤排列方式 */
void
ChewShowStatus ()
{
  char *out[] = { "【大千】" , 
                  "【許\氏】", 
                  "【IBM 】", 
                  "【精業】",
                  "【倚天】",
                  "[倚天26]"};

  ZuinData *pZuin = &(pgdata->zuinData); 
    
  input_print_string (ChewPOS - 10, 1, out[pZuin->kbtype], INPUT_FGCOLOR, INPUT_BGCOLOR);
}

/* 設定鍵盤排列方式 */
void
SetKBINT()
{
  ZuinData *pZuin = &(pgdata->zuinData);
  if (++pZuin->kbtype > 5) pZuin->kbtype = 0;
  ChewShowStatus();
}

/* 取得 Caps Lock 狀態 */
int
GetCapsLockStatus(int fd)
{
  unsigned char ledstat = 0;
  ioctl(fd, KDGETLED, &ledstat);
  return (ledstat & 4 ? 0 : 1);
  
}

/* 顯示酷音輸入區 */

void
ShowChewingText (wch_t buf[], int len, wch_t zuinBuf[], int cursor)
{
  char out_buf[100];
  int i;


  memset (out_buf, 0, sizeof (out_buf));

  for (i = 0; i < cursor; i++)
    strcat (out_buf, buf[i].s);

  for (i = 0; i < ZUIN_SIZE; i++) {
    // assume zuinBuf terminated in '\0'
    strcat (out_buf, zuinBuf[i].s);
  }
  for (i = cursor; i < len; i++)
    strcat (out_buf, buf[i].s);

  if (cursor == len)
    strcat (out_buf, "_");

  ChewShowTextArea ((char *) out_buf);


}

/* 顯示斷字狀況 */

void
ChewShowInterval (wch_t buf[], int bufLen, IntervalType arrInter[],
		  int nInter)
{
  int i, count;
  int arrPos[MAX_PHONE_SEQ_LEN];
  char out_buf[100];

  memset (out_buf, 0, sizeof (out_buf));

  count = 0;
  for (i = 0; i < bufLen; i++) {
    arrPos[i] = count;
    count += strlen ((buf[i]).s);
  }
  arrPos[i] = count;

  memset (out_buf, ' ', count * (sizeof (char)));
  out_buf[count] = '\0';

  for (i = 0; i < nInter; i++) {
    out_buf[arrPos[arrInter[i].from]] = '[';
    out_buf[arrPos[arrInter[i].to] - 1] = ']';
    memset (&out_buf[arrPos[arrInter[i].from] + 1], '-',
	    arrPos[arrInter[i].to] - arrPos[arrInter[i].from] - 2);
  }

  ChewShowChooseArea (out_buf);

}

/* 顯示可選的字和詞 */

void
ChewShowChoose (ChoiceInfo * pci, ChewingOutput * pgo)
{
  // 1.XXXX 2.XXXX 3.XXXX  XX/XX
  int i, no;
  char out_buf[100];
  char tempStr[] = "1.";

  memset (out_buf, 0, sizeof (out_buf));

  out_buf[0] = '\0';

  no = pci->pageNo * pci->nChoicePerPage;
  for (i = 0; i < pci->nChoicePerPage; no++, i++) {
    if (no >= pci->nTotalChoice)
      break;

    tempStr[0] = pgo->selKey[i];

    strcat (out_buf, tempStr);
    strcat (out_buf, pci->totalChoiceStr[no]);
    strcat (out_buf, " ");
  }

  sprintf (&out_buf[strlen (out_buf)], "%d/%d", pci->pageNo + 1, pci->nPage);
  ChewShowChooseArea (out_buf);
}

/* 顯示酷音游標 */

void
ShowCursor (wch_t buf[], int cursor)
{
  int i;
  static int count = 0;
  char out_buf[2];

  for (count = 0, i = 0; i < cursor; i++) {
    count += strlen (buf[i].s);
  }

  strcpy (out_buf, buf[i].s);

  input_print_string (ChewPOS + count, 0, out_buf,
		      INPUT_WILD_COLOR, LIGHTBAR_COLOR);
}

void
ChewReDraw (ChewingOutput * pgo)
{

  ShowChewingText (pgo->chiSymbolBuf,
		   pgo->chiSymbolBufLen, pgo->zuinBuf, pgo->chiSymbolCursor);

  if (pgo->pci->nPage == 0) {

    ChewShowInterval (pgo->chiSymbolBuf,
		      pgo->chiSymbolBufLen,
		      pgo->dispInterval, pgo->nDispInterval);
  } else {

    ChewShowChoose (pgo->pci, pgo);
  }

  ChewShowStatus();
  ShowCursor (pgo->chiSymbolBuf, pgo->chiSymbolCursor);
}

void
ChewCommitString (int tty_fd, ChewingOutput * pgo)
{
  int i, nread;
  static unsigned char buf[4096];

  for (i = 0; i < pgo->nCommitStr; i++) {

    write (tty_fd, pgo->commitStr[i].s, 2);
  }

  pgo->nCommitStr = 0;

  ChewReDraw (pgo);
}


INPUT_TABLE_STRU *
Chew_Init (void)
{
  INPUT_TABLE_STRU *table;
  int i, index;

  UseAssociateMode = 1;		/* force to no associate */

  Chewing_cf.kb_type = KB_DEFAULT;

  table = malloc (sizeof (INPUT_TABLE_STRU));
  if (table == NULL)
    out_of_memory (__FILE__, "load_input_method", __LINE__);

  strcpy (table->magic_number, MAGIC_NUMBER);
  strcpy (table->ename, "Chewing");
  if (gEncode == BIG5) {
    strcpy (table->cname, "【酷音】");
  }
  if (gEncode == GB) {
    strcpy (table->cname, (char *) string_BIG5toGB ("【酷音】"));
  }

  ChewPOS = strlen (table->cname) + POS_OF_INPUTNAME + 2;

  table->CaseTransPolicy = 1;

  table->last_full = 1;
  table->IsExtInpMethod = 1;
  table->MaxPress = 40;
  table->EnableAttr = 1;
  table->CurImmDispAttr = 1;
  table->TableEncode = BIG5;
  table->TotalKey = table->MaxDupSel = table->TotalChar = 0;
  strcpy (table->selkey, "0123456789");


  ReadTree (JMCCE_DATADIR"/chewing");
  InitChar (JMCCE_DATADIR"/chewing");
  InitDict (JMCCE_DATADIR"/chewing");

  pgdata = ALC (ChewingData, 1);
  InitChewing (pgdata, &Chewing_cf);
  SetChewingConfig ();
  ReadHash ();

  return table;
}


void
ChewReDrawText ()
{

  if (gOut.chiSymbolBufLen > 0 || *gOut.zuinBuf[0].s != 0 ||
       *gOut.zuinBuf[1].s != 0 || *gOut.zuinBuf[2].s != 0)
    ChewReDraw (&gOut);
  else
    ChewShowStatus(gOut.bChiSym);
}

void
chewing_hz_filter (int tty_fd, unsigned char key)
{
  static unsigned char last_key = 0;

  Live ();

  if (h_state == H_NORMAL && key == KEY_ESC) {
    h_state = H_ESCAPE;

    if (gOut.chiSymbolBufLen == 0)
      write (tty_fd, &key, sizeof (key));

    return;
  }

  if (h_state == H_ESCAPE) {

    if (gOut.chiSymbolBufLen == 0)
      write (tty_fd, &key, sizeof (key));
  
    if (last_key == KEY_ESC && key == '[') {
      h_state = H_FUNC_KEY;      
      return;
    }
    if (key == '[')
      h_state = H_FUNC_KEY;
    else if (key == KEY_ESC) {
      last_key = key;
      return;
    } else
      h_state = H_NORMAL;

    last_key = key;
    return;
  }

  if (h_state == H_FUNC_KEY) {

    if (gOut.chiSymbolBufLen == 0)  {
      write (tty_fd, &key, sizeof (key));
      h_state = H_NORMAL;
      return;
    }
    
    if (last_key == KEY_ESC)
      OnKeyEsc (pgdata, &gOut);

    switch (key) {
    case 'A':			/* UP */

      OnKeyUp (pgdata, &gOut);
      h_state = H_NORMAL;
      break;

    case 'B':			/* DOWN */

      OnKeyDown (pgdata, &gOut);
      h_state = H_NORMAL;
      break;

    case 'C':			/* RIGHT */

      OnKeyRight (pgdata, &gOut);
      h_state = H_NORMAL;
      break;

    case 'D':			/* LEFT */

      OnKeyLeft (pgdata, &gOut);
      h_state = H_NORMAL;
      break;

    default:

      h_state = H_NORMAL;

      break;


    }				/* switch */

    ChewReDraw (&gOut);
    last_key = key;
    return;

  }

  if (GetChiEngMode(pgdata) != GetCapsLockStatus(tty_fd))
    OnKeyCapslock (pgdata, &gOut);
  

  /* if */

  switch (key) {

  case 13:			/* ENTER */


    if (gOut.chiSymbolCursor == 0)
      write (tty_fd, &key, sizeof (key));
    OnKeyEnter (pgdata, &gOut);
    break;


  case 127:			/* BackSpace */


    if (gOut.chiSymbolBufLen == 0 && 
       (!*gOut.zuinBuf[0].s && !*gOut.zuinBuf[1].s && !*gOut.zuinBuf[2].s)) {
      write (tty_fd, &key, sizeof (key));
      return;
    }
    
    OnKeyBackspace (pgdata, &gOut);
    break;

  case KEY_DEL:

    OnKeyDel (pgdata, &gOut);
    break;


  case KEY_TAB:

    OnKeyTab (pgdata, &gOut);
    break;

  case KEY_CTRL_W:	/* 設定鍵盤排列方式 */

    SetKBINT();
    return;	
    break;

  default:
   
    if (key < KEY_CTRL_Z)	/* 如果是 CTRL+A ~ CTRL+Z 則直接輸出 */
      write (tty_fd, &key, sizeof (key));
 
    OnKeyDefault (pgdata, key, &gOut);
      
  }				/* switch */

  if (gOut.nCommitStr)
    ChewCommitString (tty_fd, &gOut);
  else
    ChewReDraw (&gOut);

  last_key = key;
  return;

}
