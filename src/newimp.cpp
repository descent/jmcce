/****************************************************************************/
/*            JMCCE  - newimp.c                                             */
/*                                                                          */
/*            	$Id: newimp.c,v 1.2 2002/05/15 04:39:00 kids Exp $  */
/****************************************************************************/


#include "config.h"

#include <stdio.h>
#include <stdlib.h>
#include <cstring>
#include <fcntl.h>
#include <ncurses.h>
#include <unistd.h>
#include <ctype.h>

#include <string>

#include "jmcce.h"
#include "newimp.h"
#include "hbffont.h"
#include "hzinput.h"

#define _lcreat creat		// for DOS compatible
#define _lopen	open		// for DOS compatible
#define _llseek lseek		// for DOS compatible
#define _lread	read		// for DOS compatible
#define _lwrite write		// for DOS compatible
#define _lclose close		// for DOS compatible

extern INI_FILE_STRU gsRow_of_INI_file[99];
extern INPUT_TABLE_STRU *gsInput_table_array[80];
extern INPUT_TABLE_STRU *gsCurrent_input_table;

extern int gEncode;
extern int gInputCount;
extern int gNextPageIndex;
extern int gCurrentPageIndex;
extern int gMultiPageMode;
int gPageIndexFirst[60];
int gPage_no = 0;
int gEnd_flag = 0;
extern int Item_attr[81];

extern FILE *gMessage_log_file;
extern int gSelectingMode_flag;

char gWaitingphrase[600][256];
char gThispage_waitingphrase[10][256];
char gDefaultphrase[256];
static char gSendingphrase[256];


char gRoot_buff[MAX_INPUT_LEN + 1];
char gRoot_buff_wild[MAX_INPUT_LEN + 1];
char gCompress_gRoot_buff[MAX_INPUT_LEN + 1];



int state = 0;
int gUseExt94key;
static int gFinalkey_flag = 0;
int gWaitingPhraseCount;
int gWaitingPos;
char lastkey;

long TUI_Search (char *result_phrase, char *root_buff, int start, int p_fp,
		 long szPos[][95]);

//------------------------------------------------------------------------------------------------

#define DEBUG

int
Is_RootEmpty ()
{
  int i;

  for (i = 0; i < MAX_INPUT_LEN; i++)
    if (gRoot_buff[i] != ' ')
      return 0;

  return 1;
}



void
NEWRootEmpty ()
{
  memset (gRoot_buff, ' ', MAX_INPUT_LEN);
  gRoot_buff[MAX_INPUT_LEN] = '\0';



  gFinalkey_flag = 0;
  gEnd_flag = 0;
  gMultiPageMode = gNextPageIndex = gCurrentPageIndex = 0;
//      gInputCount = 0;

  if (gsCurrent_input_table)
    gsCurrent_input_table->CurImmDispAttr =
      gsCurrent_input_table->ImmDispAttr;

}




void
NEWDispRootArea ()
{

  int tPOS;

  int i;
  int ii;
  int vv;

  char c = ' ';
  char c1[MAX_INPUT_LEN + 1];
  char c2[MAX_INPUT_LEN + 1];
  char tFull_gRoot_buff[MAX_INPUT_LEN * 2 + 1];
  char cw[MAX_INPUT_LEN + 1];

  tPOS =
    POS_OF_INPUTNAME + (strlen (gsCurrent_input_table->cname) <
			11 ? 11 : strlen (gsCurrent_input_table->cname));

  for (i = 0; i < gsCurrent_input_table->MaxPress; i++) {

    c = (gRoot_buff[i]);
    cw[i] = (gRoot_buff_wild[i]);

    if (c == ' ') {
      c1[i] = 0x20;
      c2[i] = 0x20;
    } else {
      for (ii = 0; ii <= strlen ((const char*)gsCurrent_input_table->KeyAscii); ii++) {
	if (c == gsCurrent_input_table->KeyAscii[ii])
	  break;
      }
      vv = ii;

      c1[i] = gsCurrent_input_table->KeyBig5[vv * 2];
      c2[i] = gsCurrent_input_table->KeyBig5[vv * 2 + 1];

    }

    tFull_gRoot_buff[i * 2] = c1[i];
    tFull_gRoot_buff[i * 2 + 1] = c2[i];

  }

  tFull_gRoot_buff[i * 2] = '\0';

  if (gEncode == BIG5) {
    if (gsCurrent_input_table->TableEncode == BIG5)
      input_print_string (tPOS, 0, tFull_gRoot_buff, INPUT_FGCOLOR,
			  INPUT_BGCOLOR);
    if (gsCurrent_input_table->TableEncode == GB)
      input_print_string (tPOS, 0, string_GBtoBIG5 (tFull_gRoot_buff),
			  INPUT_FGCOLOR, INPUT_BGCOLOR);
  }
  if (gEncode == GB) {
    if (gsCurrent_input_table->TableEncode == GB)
      input_print_string (tPOS, 0, tFull_gRoot_buff, INPUT_FGCOLOR,
			  INPUT_BGCOLOR);
    if (gsCurrent_input_table->TableEncode == BIG5)
      input_print_string (tPOS, 0, string_BIG5RADICALtoGB (tFull_gRoot_buff),
			  INPUT_FGCOLOR, INPUT_BGCOLOR);
  }

  tPOS =
    POS_OF_INPUTNAME + (strlen (gsCurrent_input_table->cname) <
			11 ? 11 : strlen (gsCurrent_input_table->cname));

  for (i = 0; i < gsCurrent_input_table->MaxPress; i++) {
    if (cw[i] == '?' || cw[i] == '*' || cw[i] == '%') {
      char temp[3];

      temp[0] = c1[i];
      temp[1] = c2[i];
      temp[2] = '\0';

      if (gEncode == BIG5) {
	if (gsCurrent_input_table->TableEncode == BIG5)
	  input_print_string (tPOS + i * 2, 0, temp, INPUT_WILD_COLOR,
			      INPUT_BGCOLOR);
	if (gsCurrent_input_table->TableEncode == GB)
	  input_print_string (tPOS + i * 2, 0, string_GBtoBIG5 (temp),
			      INPUT_WILD_COLOR, INPUT_BGCOLOR);
      }
      if (gEncode == GB) {
	if (gsCurrent_input_table->TableEncode == GB)
	  input_print_string (tPOS + i * 2, 0, temp, INPUT_WILD_COLOR,
			      INPUT_BGCOLOR);
	if (gsCurrent_input_table->TableEncode == BIG5)
	  input_print_string (tPOS + i * 2, 0, string_BIG5RADICALtoGB (temp),
			      INPUT_WILD_COLOR, INPUT_BGCOLOR);
      }
    }
  }

}

void
NEWInputInit ()
{
  NEWRootEmpty ();
  NEWDispRootArea ();
  gInputCount = 0;
  ClrSelArea ();
  gSelectingMode_flag = 0;
}


void repeat_last_phrase (int tty_fd)
{
  write (tty_fd, gSendingphrase, strlen (gSendingphrase));

  NEWRootEmpty ();
  NEWDispRootArea ();
  gInputCount = 0;
  ClrSelArea ();
  gSelectingMode_flag = 0;
}


void
NEW_repeat_last_phrase (int tty_fd)
{
  write (tty_fd, gSendingphrase, strlen (gSendingphrase));

  NEWRootEmpty ();
  NEWDispRootArea ();
  gInputCount = 0;
  ClrSelArea ();
  gSelectingMode_flag = 0;
}





INPUT_TABLE_STRU *
NEW_InputTable_load (int p_loadno)
{

  INPUT_TABLE_STRU *temp_table;

  TUI_Header tui_header;

  char tTUI_filename[6][100];

  char ikt_filename[100];
  FILE *ikt_file_pointer;

  int ikt_line_count;
  char temponeline[256];

  int i;
  int key_defined;
  int selkey_count;
  char tInputName[21];

  int c5;

  temp_table = (INPUT_TABLE_STRU*)malloc (sizeof (INPUT_TABLE_STRU));
  if (temp_table == NULL) {
    fprintf (stderr, "%c", 0x7);
    exit (0);
  }
//   out_of_memory(__FILE__, "load_input_method", __LINE__);


  if (strcmp (gsRow_of_INI_file[p_loadno].szTuiFile[1], "...") == 0) {
    temp_table->IsExtInpMethod = 0;
    temp_table->EnableAttr = 0;
    Item_attr[p_loadno] = 0;

    return temp_table;
  }

  std::string jmcce_conf_path(jmcce_path);

  for (c5 = 1; c5 <= 5; c5++) {
    strcpy (tTUI_filename[c5], (std::string(jmcce_conf_path + "/../inputref/")).c_str() );
    //strcpy (tTUI_filename[c5], JMCCE_DATADIR"/");
    strcat (tTUI_filename[c5], gsRow_of_INI_file[p_loadno].szTuiFile[c5]);
  }



  printf("jmcce_path: %s\n", jmcce_path);
  strcpy (ikt_filename, (std::string(jmcce_conf_path + "/../inputref/")).c_str() );
  printf("ikt_filename: %s\n", ikt_filename);
  //strcpy (ikt_filename, JMCCE_DATADIR"/");

  strcat (ikt_filename, gsRow_of_INI_file[p_loadno].szIktFile);

  if (strcmp (gsRow_of_INI_file[p_loadno].szEncode, "BIG5") == 0) {
    temp_table->TableEncode = BIG5;
  }

  if (strcmp (gsRow_of_INI_file[p_loadno].szEncode, "GB") == 0) {
    temp_table->TableEncode = GB;
  }

  temp_table->IsExtInpMethod = 1;
  temp_table->EnableAttr = 1;
  Item_attr[p_loadno] = 1;


  ikt_file_pointer = fopen (ikt_filename, "r");

  if (ikt_filename[strlen (ikt_filename) - 3] == 'e') {
    gUseExt94key = 1;
    ikt_line_count = 94;
  } else {
    gUseExt94key = 0;
    ikt_line_count = 68;
  }


  temp_table->KeyAscii[0] = (char) '\001';

  key_defined = 0;
  selkey_count = 0;

  strcpy (temp_table->selkey, "");

  for (i = 1; i <= ikt_line_count; i++) {
    fgets (temponeline, 13, ikt_file_pointer);
    temp_table->KeyAscii[i] = temponeline[0];
    temp_table->KeyBig5[i * 2] = temponeline[2];
    temp_table->KeyBig5[i * 2 + 1] = temponeline[3];

    temp_table->KeyAttr[i] = temponeline[5];

    if (temponeline[2] != ' ') {
      key_defined++;
    }

    if ((strlen (temponeline) > 8)) {
      temp_table->selkey[selkey_count] = temponeline[0];
      if (temponeline[7] == '0') {
	temp_table->selseq[selkey_count] = (int) (temponeline[7]) - 39;
      } else {
	temp_table->selseq[selkey_count] = (int) (temponeline[7]) - 49;
      }

      selkey_count++;
    }


  }
  temp_table->selkey[selkey_count] = '\0';


  fclose (ikt_file_pointer);

  strcpy (tInputName, gsRow_of_INI_file[p_loadno].szFullName);
  strcat (tInputName, ":");
  strcpy (temp_table->cname, tInputName);

  if (strcmp (gsRow_of_INI_file[p_loadno].szImmDispAttr, "1") == 0) {
    temp_table->ImmDispAttr = 1;
  } else {
    temp_table->ImmDispAttr = 0;
  }
  temp_table->CurImmDispAttr = temp_table->ImmDispAttr;

  //  UseAssociateMode = 1;  /* force to no associate */

  if (ikt_filename[strlen (ikt_filename) - 3] != 'v') {

    (temp_table->tui_file_pointer[1]) = _lopen (tTUI_filename[1], 0);

    if ((temp_table->tui_file_pointer[1]) == -1) {
      temp_table->IsExtInpMethod = 0;
      temp_table->EnableAttr = 0;
      Item_attr[p_loadno] = 0;
    }

    fprintf (gMessage_log_file, "reading %d bytes\n",
	     _lread (temp_table->tui_file_pointer[1], &tui_header,
		     sizeof (tui_header)));

    temp_table->MaxPress = (int) tui_header.szMaxKeyLenFullLimit;
    temp_table->last_full = (int) tui_header.szMaxKeyLenFull;
    temp_table->CaseTransPolicy = (int) tui_header.szCaseTransPolicy;
    temp_table->AutoSelect = (int) (tui_header.szAutoSelect);
    temp_table->ChoiceCharByPhrase = (int) (tui_header.szChoiceCharByPhrase);

    strcpy (temp_table->ename, tui_header.szEngName);	/* 讀取輸入法之英文名稱 */


    _lread (temp_table->tui_file_pointer[1], (char *) (temp_table->tui_index_table[1]), 95 * 95 * 4);	/* 讀取並載入 95*95 個長整數之向量索引表 */
    /* 並載入 temp_table->tui_index_table 全域 95x95 陣列中 */
    for (c5 = 2; c5 <= 5; c5++) {

      if (((temp_table->tui_file_pointer[c5]) =
	   _lopen (tTUI_filename[c5], 0)) != -1) {

	_llseek (temp_table->tui_file_pointer[c5], 256, 0);
	_lread (temp_table->tui_file_pointer[c5],
		(char *) (temp_table->tui_index_table[c5]), 95 * 95 * 4);
      }

    }

#ifdef	DEBUG

    fprintf (gMessage_log_file, "InputMethod: [%s:%s] is loaded! \n",
	     temp_table->cname, temp_table->ename);
    fprintf (gMessage_log_file, "ImmDispAttr = %d\n\
CurImmDispAttr=%d\n\
last_full=%d (%c)\n\
MaxPress=%d\n\
selkey=%s\n\
TotalKey=%d\n\
MaxDupSel=%d\n\
TotalChar=%d\n\
", temp_table->ImmDispAttr, temp_table->CurImmDispAttr, temp_table->last_full, temp_table->last_full, temp_table->MaxPress, temp_table->selkey, temp_table->TotalKey, temp_table->MaxDupSel, temp_table->TotalChar);



#endif

    if (p_loadno != 10) {
      if (temp_table->EnableAttr == 1) {
	fprintf (stderr, "InputMethod: [%s] is loaded!  %d keys defined..\n",
		 tTUI_filename[1], key_defined);
	fprintf (gMessage_log_file,
		 "InputMethod: [%s] is loaded!  %d keys defined..\n",
		 tTUI_filename[1], key_defined);
      } else {
	fprintf (stderr, ">> error: Input ref file load error! file =%s\n",
		 tTUI_filename[1]);
	fprintf (gMessage_log_file,
		 ">> error: Input ref file load error! file =%s\n",
		 tTUI_filename[1]);
      }
    } else {
      fprintf (stderr,
	       "Intercode InputMethod is working! 16 keys defined..\n");

      fprintf (gMessage_log_file,
	       "Intercode InputMethod is working! 16 keys defined..\n");

    }

  } else {

    temp_table->MaxPress = 5;
    temp_table->last_full = 1;
    temp_table->CaseTransPolicy = 1;
    temp_table->AutoSelect = 1;
    strcpy (temp_table->ename, "virtual");

    fprintf (stderr, "InputMethod: [%s] is loaded!  %d keys defined..\n",
	     ikt_filename, key_defined);

    fprintf (gMessage_log_file,
	     "InputMethod: [%s] is loaded!  %d keys defined..\n",
	     ikt_filename, key_defined);

  }

  strcpy (temp_table->magic_number, MAGIC_NUMBER);

  return temp_table;

}


void NEWtable_unload (int p_table_no)
{
  free (gsInput_table_array[p_table_no]);
}


int
NEWDispSelection (char *szOut, char *root_buff, int tty_fd, int disp_selkey)
{

  int i = 0;
  int ii = 0;

  int tPOS = 0;
  int tFind_count;
  int tFind_count2;
  char tResult_phrase[256] = "";
  int tTUI_file_pointer[6];
  int tDefault_number;
  int tList_count;
  int rPhrase_count = 0;

  int tDefault_flag = 0;
  int tPOS_defaultphrase = 0;

  char k[40];
  int tTest = 0;

  int c5;

  for (c5 = 1; c5 <= 5; c5++) {
    tTUI_file_pointer[c5] = (gsCurrent_input_table->tui_file_pointer[c5]);	/* 重要！！取得現行輸入法總資料結構中的 tui 檔案指標！ */
  }

  ClrSelArea ();

  tPOS++;

  if (gMultiPageMode && gCurrentPageIndex != 0) {
    input_print_string (1, 1, "< ", INPUT_FGCOLOR, INPUT_BGCOLOR);
    tPOS += 2;
  }

  tTest = 0;

  if ((TUI_Search
       (tResult_phrase, root_buff, 0, tTUI_file_pointer[1],
	gsCurrent_input_table->tui_index_table[1]) == 1)
      ||
      (TUI_Search
       (tResult_phrase, root_buff, 0, tTUI_file_pointer[2],
	gsCurrent_input_table->tui_index_table[2]) == 1)
      ||
      (TUI_Search
       (tResult_phrase, root_buff, 0, tTUI_file_pointer[3],
	gsCurrent_input_table->tui_index_table[3]) == 1)
      ||
      (TUI_Search
       (tResult_phrase, root_buff, 0, tTUI_file_pointer[4],
	gsCurrent_input_table->tui_index_table[4]) == 1)
      ||
      (TUI_Search
       (tResult_phrase, root_buff, 0, tTUI_file_pointer[5],
	gsCurrent_input_table->tui_index_table[5]) == 1)) {
    tTest = 1;
  }


  if (tTest == 1) {
    if ((tFind_count =
	 TUI_Search (tResult_phrase, root_buff, 0, tTUI_file_pointer[1],
		     gsCurrent_input_table->tui_index_table[1])) > 0) {
      rPhrase_count = 1;

      strcpy (gWaitingphrase[i], tResult_phrase);

      while (tFind_count >= 1) {


	tFind_count =
	  TUI_Search (tResult_phrase, root_buff, 1, tTUI_file_pointer[1],
		      gsCurrent_input_table->tui_index_table[1]);

	if ((tFind_count) == 0 && rPhrase_count == 1) {
	  //  return rPhrase_count;      
	}
	if ((tFind_count) != 0) {
	  rPhrase_count++;
	  i++;
	  strcpy (gWaitingphrase[i], tResult_phrase);
	}
      }
      i++;			/* for next append */

    }

    for (c5 = 2; c5 <= 5; c5++) {

      if ((tFind_count =
	   TUI_Search (tResult_phrase, root_buff, 0, tTUI_file_pointer[c5],
		       gsCurrent_input_table->tui_index_table[c5])) > 0) {

	rPhrase_count += 1;

	strcpy (gWaitingphrase[i], tResult_phrase);

	while (tFind_count >= 1) {
	  tFind_count =
	    TUI_Search (tResult_phrase, root_buff, 1, tTUI_file_pointer[c5],
			gsCurrent_input_table->tui_index_table[c5]);

	  if ((tFind_count) == 0 && rPhrase_count == 1) {
	    return rPhrase_count;
	  }

	  if ((tFind_count) != 0) {
	    rPhrase_count++;
	    i++;
	    strcpy (gWaitingphrase[i], tResult_phrase);
	  }
	}
	i++;
      }

    }

  } else {
    rPhrase_count = 0;
    return rPhrase_count;
  }

  tList_count = 0;

  gPageIndexFirst[gPage_no] = gCurrentPageIndex;

  for (ii = gCurrentPageIndex; ii < rPhrase_count; ii++) {

    if ((tList_count >= MAX_SEL_COUNT) || (tPOS >= MAX_SEL_LENGTH)) {	/* 又找到了但是已經到達 10 個了，或長度超過了 */
      gCurrentPageIndex = gPageIndexFirst[gPage_no];

      gMultiPageMode = 1;

      break;
    }


/* get default phrase *///  if(i<2)  default_number=i;   else  default_number=2;

    tDefault_number = 0;


//         if(ii!=tDefault_number)
    {
      int j;

      for (j = 0; j < 21; j++) {
	if ((gsCurrent_input_table->selseq[j]) == tList_count)
	  break;
      }

      if (disp_selkey == 1)
	input_draw_ascii (tPOS, 1, gsCurrent_input_table->selkey[j],
			  INPUT_CHOICENUMBERCOLOR, INPUT_CHOICENUMBERBGCOLOR);
      tPOS++;

      strcpy (gThispage_waitingphrase[tList_count], gWaitingphrase[ii]);

      if (ii == tDefault_number) {
	tDefault_flag = 1;
	tPOS_defaultphrase = tPOS;
      }


      if (gEncode == BIG5) {
	if (gsCurrent_input_table->TableEncode == BIG5) {
	  input_print_string (tPOS, 1, gWaitingphrase[ii], INPUT_FGCOLOR,
			      INPUT_BGCOLOR);

	}
	if (gsCurrent_input_table->TableEncode == GB) {
	  input_print_string (tPOS, 1, string_GBtoBIG5 (gWaitingphrase[ii]),
			      INPUT_FGCOLOR, INPUT_BGCOLOR);

	}
      }
      if (gEncode == GB) {
	if (gsCurrent_input_table->TableEncode == GB) {
	  input_print_string (tPOS, 1, gWaitingphrase[ii], INPUT_FGCOLOR,
			      INPUT_BGCOLOR);

	}
	if (gsCurrent_input_table->TableEncode == BIG5) {
	  input_print_string (tPOS, 1, string_BIG5toGB (gWaitingphrase[ii]),
			      INPUT_FGCOLOR, INPUT_BGCOLOR);

	}
      }

      tPOS += (strlen (gWaitingphrase[ii]) + 2);
    }

    tList_count++;
    gCurrentPageIndex++;
    gNextPageIndex++;
  }

  if (tDefault_flag == 1 && gMultiPageMode == 0) {
    strcpy (gDefaultphrase, gWaitingphrase[tDefault_number]);

    if (gEncode == BIG5) {
      if (gsCurrent_input_table->TableEncode == BIG5) {
	input_print_string (tPOS_defaultphrase, 1,
			    gWaitingphrase[tDefault_number],
			    INPUT_DEFAULTCOLOR, INPUT_BGCOLOR);
      }
      if (gsCurrent_input_table->TableEncode == GB) {
	input_print_string (tPOS_defaultphrase, 1,
			    string_GBtoBIG5 (gWaitingphrase[tDefault_number]),
			    INPUT_DEFAULTCOLOR, INPUT_BGCOLOR);
      }
    }
    if (gEncode == GB) {
      if (gsCurrent_input_table->TableEncode == GB) {
	input_print_string (tPOS_defaultphrase, 1,
			    gWaitingphrase[tDefault_number],
			    INPUT_DEFAULTCOLOR, INPUT_BGCOLOR);

      }
      if (gsCurrent_input_table->TableEncode == BIG5) {
	input_print_string (tPOS_defaultphrase, 1,
			    string_BIG5toGB (gWaitingphrase[tDefault_number]),
			    INPUT_DEFAULTCOLOR, INPUT_BGCOLOR);

      }
    }

    tDefault_flag = 0;

  }

  if (ii >= rPhrase_count) {
    gEnd_flag = 1;
  }


  if (gMultiPageMode && gEnd_flag == 0) {
    input_print_string (tPOS, 1, "> ", INPUT_FGCOLOR, INPUT_BGCOLOR);
  }

  return rPhrase_count;
}


void
NEWNextPage (int tty_fd)
{
  if (gMultiPageMode) {

    if (gEnd_flag == 1) {
      gPage_no = 0;
      gNextPageIndex = 0;
      gCurrentPageIndex = 0;
      gEnd_flag = 0;
    } else {
      gPage_no++;
      gCurrentPageIndex = gNextPageIndex;
    }
  }
}


void
NEWPrevPage (int tty_fd)
{
  if (gMultiPageMode) {
    gPage_no--;

    gEnd_flag = 0;

    if (gPage_no <= 0) {
      gPage_no = 0;
      gNextPageIndex = 0;
    }

    gNextPageIndex = gPageIndexFirst[gPage_no];
    gCurrentPageIndex = gPageIndexFirst[gPage_no];
  }
}



void
NEW_hz_filter (int tty_fd, unsigned char key, int pListRightNow)
{

  int i;
  int ii;
  int jj;
  int c5;
  int valid_flag;

  int tPointer;

  gsCurrent_input_table->CurImmDispAttr = gsCurrent_input_table->ImmDispAttr;

  switch (state) {
  case 0:

    switch (key) {

    case '\010':		/* BackSpace Ctrl+H */
    case '\177':		/* BackSpace */

      for (ii = gsCurrent_input_table->MaxPress - 1; ii >= 0; ii--) {	/* 因為放入字根緩衝區的位置可能不連續，因此取得不連續位置中最大的有字根位置 */
	if (gRoot_buff[ii] != ' ')
	  break;
      }
      gInputCount = ii + 1;

      if (gInputCount > 0) {


	gInputCount--;

	for (i = gsCurrent_input_table->MaxPress - 1; i >= 0; i--) {
	  if (gRoot_buff[i] != ' ') {
	    gRoot_buff[i] = ' ';
	    break;		/* for i=max-1..0 */
	  }
	}
	NEWDispRootArea ();


	if (gInputCount == 0 || Is_RootEmpty ()) {
	  NEWRootEmpty ();
	  NEWDispRootArea ();
	  gInputCount = 0;
	  ClrSelArea ();
	  gPage_no = 0;
	  gSelectingMode_flag = 0;
	} else {

	  gsCurrent_input_table->CurImmDispAttr =
	    gsCurrent_input_table->ImmDispAttr;


	  if (pListRightNow == 1) {
	    int k;
	    int t;
	    int tt;


	    gSelectingMode_flag = 0;

	    tt = 0;
	    for (t = 0; t <= gInputCount; t++) {
	      switch (gRoot_buff_wild[t]) {
	      case '?':
		gCompress_gRoot_buff[tt] = 0x10;
		gsCurrent_input_table->ImmDispAttr = 0;
		break;
	      case '*':
		gCompress_gRoot_buff[tt] = 0x11;
		gsCurrent_input_table->ImmDispAttr = 0;
		break;
	      case '%':
		gCompress_gRoot_buff[tt] = 0x12;
		gsCurrent_input_table->ImmDispAttr = 0;
		break;
	      default:
		gCompress_gRoot_buff[tt] = gRoot_buff[t];
		break;
	      }
	      tt++;

	    }

	    // gRoot_buff[gInputCount]='\0';

	    gCompress_gRoot_buff[tt] = '\0';

	    NEWDispRootArea ();
	    ClrSelArea ();

	    gWaitingPhraseCount = 0;
	    gWaitingPos = 1;

	    for (c5 = 1; c5 <= 5; c5++) {
	      k =
		NEWIncrePreview (gSendingphrase, gCompress_gRoot_buff, tty_fd,
				 0,
				 (gsCurrent_input_table->
				  tui_file_pointer[c5]), c5);
	    }


	  }


	}

      } else
	outchar (tty_fd, key);

      break;

    case '\033':		/* ESCAPE */
      state = 1;
      {
	NEWRootEmpty ();
	NEWDispRootArea ();
	gInputCount = 0;
	ClrSelArea ();
	gPage_no = 0;
      }
      break;

    case '>':

      if (gMultiPageMode) {
	NEWNextPage (tty_fd);
	NEWDispSelection (gSendingphrase, gCompress_gRoot_buff, tty_fd, 1);
      } else
	outchar (tty_fd, key);


      break;

    case '<':

      if (gMultiPageMode) {
	NEWPrevPage (tty_fd);
	NEWDispSelection (gSendingphrase, gCompress_gRoot_buff, tty_fd, 1);
      } else
	outchar (tty_fd, key);

      break;

    default:

      if (gsCurrent_input_table->CaseTransPolicy == 1)
	key = toupper (key);	/* 檔案製作時是大寫，小寫一律轉大寫再找 */

      if (gsCurrent_input_table->CaseTransPolicy == 2)
	key = tolower (key);	/* 檔案製作時是小寫，大寫一律轉小寫再找 */



      for (jj = 1; jj <= gUseExt94key == 1 ? 94 : 68; jj++) {
	if (key == (gsCurrent_input_table->KeyAscii[jj])) {
	  valid_flag = 1;
	  break;
	}
      }


      if (!gSelectingMode_flag) {
	char tempstr[3];

	switch (gsCurrent_input_table->KeyAttr[jj]) {
	case 'T':
	  outchar (tty_fd, key);
	  return;
	  break;

	case 'A':
	  tempstr[0] = gsCurrent_input_table->KeyBig5[jj * 2];
	  tempstr[1] = gsCurrent_input_table->KeyBig5[jj * 2 + 1];
	  tempstr[2] = '\0';
	  write (tty_fd, tempstr, 3);
	  return;
	  break;


	case 'N':
	  return;
	  break;

	case 'F':
	  //                         gFinalkey_flag=1;

	case '*':
	case '?':
	case '%':
	case '0':
	case '1':
	case '2':
	case '3':
	  break;

	default:

	  if (key != ' ') {
	    outchar (tty_fd, key);
	    return;
	  }
	}
      }


      if (gSelectingMode_flag) {
	if (key == ' ') {
	  if (gMultiPageMode) {
	    NEWNextPage (tty_fd);
	    NEWDispSelection (gSendingphrase, gCompress_gRoot_buff, tty_fd,
			      1);
	  } else {
	    strcpy (gSendingphrase, gDefaultphrase);
	    FANJIAN_write (tty_fd, gSendingphrase, strlen (gSendingphrase));

	    NEWRootEmpty ();
	    NEWDispRootArea ();
	    gInputCount = 0;
	    ClrSelArea ();
	    gPage_no = 0;
	    gSelectingMode_flag = 0;

	  }

	  return;
	}

	if (strchr (gsCurrent_input_table->selkey, key)) {

	  int ii;

	  for (ii = 0; ii <= strlen (gsCurrent_input_table->selkey); ii++) {

	    if (key == gsCurrent_input_table->selkey[ii])
	      break;
	  }


	  strcpy (gSendingphrase,
		  gThispage_waitingphrase[gsCurrent_input_table->selseq[ii]]);
	  FANJIAN_write (tty_fd, gSendingphrase, strlen (gSendingphrase));

	  NEWRootEmpty ();
	  NEWDispRootArea ();
	  gInputCount = 0;
	  ClrSelArea ();
	  gPage_no = 0;
	  gSelectingMode_flag = 0;

	  return;
	} else {
	  if ((int) (gsCurrent_input_table->AutoSelect) == 1) {

	    strcpy (gSendingphrase, gDefaultphrase);
	    FANJIAN_write (tty_fd, gSendingphrase, strlen (gSendingphrase));
	    NEWRootEmpty ();
	    NEWDispRootArea ();
	    gInputCount = 0;
	    ClrSelArea ();
	    gPage_no = 0;
	    gSelectingMode_flag = 0;

	  }

	}
      }

      if (key != ' ') {

	if (gInputCount == 0) {
	  NEWRootEmpty ();
	  NEWDispRootArea ();
	  //   gInputCount=0;
	  ClrSelArea ();
	  gPage_no = 0;
	}

	if (gInputCount <= gsCurrent_input_table->MaxPress - 1) {
	  char position;

	  position = gsCurrent_input_table->KeyAttr[jj];

	  if (position == '0') {

	    gRoot_buff[gInputCount] = (key);
	    gRoot_buff_wild[gInputCount] = '0';
	    gInputCount++;

	  } else if (position == '*' || position == '?' || position == '%') {

	    gRoot_buff[gInputCount] = (key);
	    gRoot_buff_wild[gInputCount] = position;

	    gsCurrent_input_table->CurImmDispAttr = 0;

	    gInputCount++;
	  } else {
	    if (position != 'F') {
	      gRoot_buff[position - 49] = (key);
	    } else {
	      gRoot_buff[gsCurrent_input_table->MaxPress - 1] = (key);	/* 放入最末一個按鍵的字根緩衝區位置 */
	      gFinalkey_flag = 1;
	    }

	    for (ii = gsCurrent_input_table->MaxPress; ii >= 0; ii--) {	/* 因為放入字根緩衝區的位置可能不連續，因此取得不連續位置中最大的有字根位置 */
	      if (gRoot_buff[ii] != ' ')
		break;
	    }
	    gInputCount = ii + 1;
	  }
	} else {
	  if (gEncode == BIG5) {
	    input_print_string (POS_OF_INPUTNAME +
				strlen (gsCurrent_input_table->cname), 1,
				"超過允許\碼長無效...", HINT_NOTFOUNDCOLOR,
				INPUT_BGCOLOR);
	  }

	  if (gEncode == GB) {
	    input_print_string (POS_OF_INPUTNAME +
				strlen (gsCurrent_input_table->cname), 1,
				string_BIG5toGB ("已超過允許\碼長無效..."),
				HINT_NOTFOUNDCOLOR, INPUT_BGCOLOR);
	  }

	}

      }

      if (key == ' ' || gFinalkey_flag == 1) {
	if (gInputCount) {
	  int k;
	  int t;
	  int tt;

	  gSelectingMode_flag = 1;

	  tt = 0;
	  for (t = 0; t <= gInputCount; t++) {
	    if (gRoot_buff[t] != ' ') {

	      switch (gRoot_buff_wild[t]) {
	      case '?':
		gCompress_gRoot_buff[tt] = 0x10;
		break;
	      case '*':
		gCompress_gRoot_buff[tt] = 0x11;
		break;
	      case '%':
		gCompress_gRoot_buff[tt] = 0x12;
		break;
	      default:
		gCompress_gRoot_buff[tt] = gRoot_buff[t];
		break;
	      }
	      tt++;
	    }


	  }

	  // gRoot_buff[gInputCount]='\0';

	  gCompress_gRoot_buff[tt] = '\0';

	  k =
	    NEWDispSelection (gSendingphrase, gCompress_gRoot_buff, tty_fd,
			      1);

	  if (k == 1)
	    /* find single phrase */
	  {

	    strcpy (gSendingphrase, gWaitingphrase[0]);
	    FANJIAN_write (tty_fd, gSendingphrase, strlen (gSendingphrase));

	    NEWRootEmpty ();
	    NEWDispRootArea ();
	    gInputCount = 0;
	    ClrSelArea ();
	    gPage_no = 0;
	    gSelectingMode_flag = 0;

	    return;
	  }

	  if (k == 0) {
	    fprintf (stderr, "%c", 0x7);

	    if (gEncode == BIG5) {
	      input_print_string (POS_OF_INPUTNAME +
				  strlen (gsCurrent_input_table->cname), 1,
				  "此鍵入碼無字詞...", HINT_NOTFOUNDCOLOR,
				  INPUT_BGCOLOR);
	    }

	    if (gEncode == GB) {
	      input_print_string (POS_OF_INPUTNAME +
				  strlen (gsCurrent_input_table->cname), 1,
				  string_BIG5toGB ("此鍵入碼無字詞..."),
				  HINT_NOTFOUNDCOLOR, INPUT_BGCOLOR);
	    }


	    gInputCount = 0;
	    gPage_no = 0;
	    gSelectingMode_flag = 0;

	    return;
	  }
	} else {
	  outchar (tty_fd, key);
	  return;
	}
      }

      if ((int) (gsCurrent_input_table->last_full) == 1) {

	if (gInputCount == (gsCurrent_input_table->MaxPress)) {
	  int k;
	  int t;
	  int tt;

	  gSelectingMode_flag = 1;

	  tt = 0;
	  for (t = 0; t <= gInputCount; t++) {
	    if (gRoot_buff[t] != ' ') {

	      switch (gRoot_buff_wild[t]) {
	      case '?':
		gCompress_gRoot_buff[tt] = 0x10;
		break;
	      case '*':
		gCompress_gRoot_buff[tt] = 0x11;
		break;
	      case '%':
		gCompress_gRoot_buff[tt] = 0x12;
		break;
	      default:
		gCompress_gRoot_buff[tt] = gRoot_buff[t];
		break;
	      }
	      tt++;
	    }


	  }

	  // gRoot_buff[gInputCount]='\0';

	  gCompress_gRoot_buff[tt] = '\0';

	  k =
	    NEWDispSelection (gSendingphrase, gCompress_gRoot_buff, tty_fd,
			      1);

	  if (k == 1)
	    /* find single phrase */
	  {

	    strcpy (gSendingphrase, gWaitingphrase[0]);
	    FANJIAN_write (tty_fd, gSendingphrase, strlen (gSendingphrase));

	    NEWRootEmpty ();
	    NEWDispRootArea ();
	    gInputCount = 0;
	    ClrSelArea ();
	    gPage_no = 0;
	    gSelectingMode_flag = 0;

	    return;
	  }

	  if (k == 0) {
	    fprintf (stderr, "%c", 0x7);

	    if (gEncode == BIG5) {
	      input_print_string (POS_OF_INPUTNAME +
				  strlen (gsCurrent_input_table->cname), 1,
				  "此鍵入碼無字詞...", HINT_NOTFOUNDCOLOR,
				  INPUT_BGCOLOR);
	    }

	    if (gEncode == GB) {
	      input_print_string (POS_OF_INPUTNAME +
				  strlen (gsCurrent_input_table->cname), 1,
				  string_BIG5toGB ("此鍵入碼無字詞..."),
				  HINT_NOTFOUNDCOLOR, INPUT_BGCOLOR);
	    }

	    gInputCount = 0;
	    gPage_no = 0;
	    gSelectingMode_flag = 0;

	    return;
	  }
	}
      }


      NEWDispRootArea ();	/* 顯示現已輸入的字根 */

      if (pListRightNow == 1) {

	if (gSelectingMode_flag == 0) {

	  int k;
	  int t;
	  int tt;

	  tt = 0;
	  for (t = 0; t <= gInputCount; t++) {
	    if (gRoot_buff[t] != ' ') {

	      switch (gRoot_buff_wild[t]) {
	      case '?':
		gCompress_gRoot_buff[tt] = 0x10;
		break;
	      case '*':
		gCompress_gRoot_buff[tt] = 0x11;
		break;
	      case '%':
		gCompress_gRoot_buff[tt] = 0x12;
		break;
	      default:
		gCompress_gRoot_buff[tt] = gRoot_buff[t];
		break;
	      }
	      tt++;
	    }

	  }

	  // gRoot_buff[gInputCount]='\0';

	  gCompress_gRoot_buff[tt] = '\0';

	  NEWDispRootArea ();
	  ClrSelArea ();

	  gWaitingPhraseCount = 0;
	  gWaitingPos = 1;
	  for (c5 = 1; c5 <= 5; c5++) {
	    k =
	      NEWIncrePreview (gSendingphrase, gCompress_gRoot_buff, tty_fd,
			       0,
			       (gsCurrent_input_table->tui_file_pointer[c5]),
			       c5);
	  }

	}
      }


      if (gsCurrent_input_table->ChoiceCharByPhrase == 1) {
	if (strchr ("1234567890", gRoot_buff[4]) != 0) {
	  int tChoiceNum;
	  char tWaitingPhraseComb[500];
	  char tChoiceRoot;

	  strcpy (tWaitingPhraseComb, gWaitingphrase[0]);
	  strcat (tWaitingPhraseComb, gWaitingphrase[1]);
	  strcat (tWaitingPhraseComb, gWaitingphrase[2]);
	  strcat (tWaitingPhraseComb, gWaitingphrase[3]);
	  strcat (tWaitingPhraseComb, gWaitingphrase[4]);
	  strcat (tWaitingPhraseComb, gWaitingphrase[5]);

	  if (gRoot_buff[2] == '\\') {
	    tChoiceRoot = gRoot_buff[3];
	  } else {
	    tChoiceRoot = gRoot_buff[4];
	  }

	  if (tChoiceRoot != '\\') {
	    if (tChoiceRoot == '0') {
	      tChoiceNum = 10;
	    } else {
	      tChoiceNum = (int) tChoiceRoot - 48;
	    }
	  }


	  strncpy (gSendingphrase, tWaitingPhraseComb + (tChoiceNum - 1) * 2,
		   2);
	  gSendingphrase[2] = '\0';

	  FANJIAN_write (tty_fd, gSendingphrase, strlen (gSendingphrase));

	  NEWRootEmpty ();
	  NEWDispRootArea ();
	  gInputCount = 0;
	  ClrSelArea ();
	  gPage_no = 0;
	  gSelectingMode_flag = 0;

	  return;
	}
      }

      break;
    }


    break;


  case 1:
    if (key == '[') {
      state = 2;
    } else {
      outchar (tty_fd, '\033');
      outchar (tty_fd, key);
      state = 0;
    }

    break;
  case 2:
    switch (key) {
    case '[':
      state = 3;
      break;
    case '1':
      state = 4;
      lastkey = key;
      break;
    case '2':
      state = 5;
      lastkey = key;
      break;
    case '3':
      state = 9;
      lastkey = key;
      break;
    case '4':
      state = 9;
      lastkey = key;
      break;
    case '5':
      state = 9;
      lastkey = key;
      break;
    case '6':
      state = 9;
      lastkey = key;
      break;
    case 'A':
    case 'B':
    case 'C':
    case 'D':
      outchar (tty_fd, '\033');
      outchar (tty_fd, '[');
      outchar (tty_fd, key);
      state = 0;
      break;
    }
    break;

  case 3:

    outchar (tty_fd, '\033');
    outchar (tty_fd, '[');
    outchar (tty_fd, '[');
    outchar (tty_fd, key);
    state = 0;

    break;
  case 4:
    if ((key == '7') || (key == '8') || (key == '9')) {
      lastkey = key;
      state = 6;
    } else if (key == '~') {
      outchar (tty_fd, '\033');
      outchar (tty_fd, '[');
      outchar (tty_fd, lastkey);
      outchar (tty_fd, '~');
      state = 0;
    } else {
      state = 0;
    }
    break;
  case 5:
    if ((key == '0') || (key == '1') || (key == '3') || (key == '4')) {
      lastkey = key;
      state = 7;
    } else if (key == '~') {
      outchar (tty_fd, '\033');
      outchar (tty_fd, '[');
      outchar (tty_fd, lastkey);
      outchar (tty_fd, '~');
      state = 0;
    } else {
      state = 0;
    }
    break;
  case 6:
    if (key == '~') {
      outchar (tty_fd, '\033');
      outchar (tty_fd, '[');
      outchar (tty_fd, '1');
      outchar (tty_fd, lastkey);
      outchar (tty_fd, '~');
    }
    state = 0;
    break;
  case 7:
    if (key == '~') {
      outchar (tty_fd, '\033');
      outchar (tty_fd, '[');
      outchar (tty_fd, '2');
      outchar (tty_fd, lastkey);
      outchar (tty_fd, '~');
    }
    state = 0;
    break;

  case 9:
    if (key == '~') {
      outchar (tty_fd, '\033');
      outchar (tty_fd, '[');
      outchar (tty_fd, lastkey);
      outchar (tty_fd, '~');
    }
    state = 0;
    break;
  }

}



//---------------------------------------------------------------------------------------
//---------------------------------------------------------------------------------------

long
Find_BlockLength (char i0, char i1, long szPos[][95])
{
  int s0 = (int) i0 - 32;
  int s1 = (int) i1 - 32;

  long start_offset = szPos[s0][s1];

  int next_offset = 0;
  int read_len = 0;

  int x, y;

  y = s0;
  for (x = s1 + 1; x < 95; x++) {
    if (szPos[y][x] > 0) {
      next_offset = szPos[y][x];
      goto find;
    }
  }

  for (y = s0 + 1; y < 95; y++)
    for (x = 0; x < 95; x++) {
      if (szPos[y][x] > 0) {
	next_offset = szPos[y][x];
	goto find;
      }
    }

find:

  if (next_offset > 0)
    read_len = next_offset - start_offset;
  else
    read_len = 999999999L;	//檔案最後

  return read_len;
}


long
g_TUI_Search_Exact (char *result_phrase, char *root_buff,
		    int start, int p_fp, long szPos[][95])
{
  static long s_read_len = 0;

  char szSearch[8] = "";
  char szPhrase[100];
  char szMisc[20];

  char ch;
  char len;
  int n;
  int nbyte = 0;

  if (start == 0) {
    int s0 = (int) root_buff[0] - 32;
    int s1 = (int) root_buff[1] - 32;

    long start_offset = szPos[s0][s1];

    char id;
    int n;

    if (start_offset == 0) {
      return 0;
    }

    _llseek (p_fp, start_offset, SEEK_SET);

    s_read_len = Find_BlockLength (root_buff[0], root_buff[1], szPos);	//取得到下一組兩碼詞鍵間有多少總長度

    n = _lread (p_fp, (char *) &id, 1);

    s_read_len -= n;

  }

  result_phrase[0] = '\0';

  while (s_read_len > 0) {
    result_phrase[0] = '\0';

    nbyte = 0;

    szSearch[0] = root_buff[0];
    szSearch[1] = root_buff[1];
    szSearch[2] = '\0';

    //----------------------------------------------------
    n = _lread (p_fp, (char *) &ch, 1);
    if (n < 1)
      break;
    nbyte += n;

    if (ch > 2) {
      n = _lread (p_fp, (char *) szMisc, (ch - 2));
      nbyte += n;

      szMisc[ch - 2] = '\0';
      strcat (szSearch, szMisc);
    }
    n = _lread (p_fp, (char *) &len, 1);
    nbyte += n;

    n = _lread (p_fp, (char *) szPhrase, len);
    szPhrase[len] = '\0';
    nbyte += n;

    //-------------------------------

    s_read_len -= nbyte;

    //-------------------------------

    if (strcmp (root_buff, szSearch) == 0) {
      strcat (result_phrase, szPhrase);
      break;
    }
  }

  if (strlen (result_phrase) > 0)
    return 1;
  else
    return 0;
}



int
Comp_WildChar1 (char *root_buff, char *szSearch)
{
  int input_len = strlen (root_buff);
  int c = 0;
  int i;

  if (input_len == strlen (szSearch)) {
    for (i = 0; i < input_len + 1; i++) {
      if (root_buff[i] == '\0')
	return 1;
      if (root_buff[i] == 0x10)
	c++;
      else {
	if (root_buff[i] != szSearch[c])
	  return -1;
	else {
	  c++;
	}
      }
    }
  }

  return -1;
}


int
Comp_WildChar2 (const char *root_buff, const char *szSearch)
{
  int input_len = strlen (root_buff);
  int search_len = strlen (szSearch);
  int i, s = 0, star = 0;

  for (i = 0; i < input_len; i++) {
    if (root_buff[i] == 0x11) {
      star = i;
      break;
    }
  }

  //first '*ab'
  if (star == 0) {
    s = search_len - 1;
    for (i = input_len - 1; i > star; i--) {
      if (root_buff[i] != szSearch[s])
	return -1;
      s--;
    }
    return 1;
  }
  //last 'cd*'
  else if (star == input_len - 1) {
    s = 0;
    for (i = 0; i < star; i++) {
      if (root_buff[i] != szSearch[s])
	return -1;
      s++;
    }
    return 1;
  }
  //middle 'ab*c'
  else {			//before star
    s = 0;
    for (i = 0; i < star; i++) {
      if (root_buff[i] != szSearch[s])
	return -1;
      s++;
    }
    //after star
    s = search_len - 1;
    for (i = input_len - 1; i > star; i--) {
      if (root_buff[i] != szSearch[s])
	return -1;
      s--;
    }
    return 1;

  }
  return -1;
}


int
Comp_WildChar3 (const char *root_buff, const char *szSearch)
{
  int input_len = strlen (root_buff);
  int search_len = strlen (szSearch);
  int i, s = 0, star = -1;

  for (i = 0; i < input_len; i++) {
    if (root_buff[i] == 0x12) {
      star = i;
      break;
    }
  }

  if (star == -1)
    return -1;
  else {			//before star
    s = 0;
    for (i = 0; i < star; i++) {
      if (root_buff[i] != szSearch[s])
	return -1;
      s++;
    }
    //after star
    s = search_len - 1;
    for (i = input_len - 1; i > star; i--) {
      if (root_buff[i] != szSearch[s])
	return -1;
      s--;
    }
    return 1;

  }
  return -1;
}




long
s_TUI_Search_WildChar (char cmpChar, int ch0, int ch1,
		       char *result_phrase, char *root_buff,
		       int start, int p_fp, long szPos[][95])
{
  static long s_read_len = 0;

  char szSearch[20] = "";
  char szPhrase[256];
  char szMisc[20];

  char ch;
  char len;
  int n;
  int nbyte = 0;

  if (start == 0) {
    int s0 = (int) ch0 - 32;
    int s1 = (int) ch1 - 32;
    long start_offset = szPos[s0][s1];

    char id;
    int n;


    _llseek (p_fp, start_offset, SEEK_SET);

    s_read_len = Find_BlockLength (ch0, ch1, szPos);

    n = _lread (p_fp, (char *) &id, 1);
    if (n < 1)
      return 0;

    s_read_len -= n;

  }

  result_phrase[0] = '\0';

  while (s_read_len > 0) {
//              result_phrase[0]='\0';  

    nbyte = 0;

    szSearch[0] = ch0;
    szSearch[1] = ch1;
    szSearch[2] = '\0';

    //-------------------------------------------------------------
    n = _lread (p_fp, (char *) &ch, 1);
    nbyte += n;

    if (n < 1)
      return 0;


    if (ch > 2) {
      n = _lread (p_fp, (char *) szMisc, (ch - 2));
      nbyte += n;

      szMisc[ch - 2] = '\0';
      strcat (szSearch, szMisc);
    }
    n = _lread (p_fp, (char *) &len, 1);
    nbyte += n;

    n = _lread (p_fp, (char *) szPhrase, len);
    szPhrase[len] = '\0';
    nbyte += n;

    //-------------------------------

    s_read_len -= nbyte;

    if (cmpChar == 0x10) {
      if (Comp_WildChar1 (root_buff, szSearch) == 1) {
	strcat (result_phrase, szPhrase);
	break;
      }
    } else if (cmpChar == 0x11) {
      if (Comp_WildChar2 (root_buff, szSearch) == 1) {
	strcat (result_phrase, szPhrase);
	break;
      }
    } else if (cmpChar == 0x12) {
      if (Comp_WildChar3 (root_buff, szSearch) == 1) {
	strcat (result_phrase, szPhrase);
	break;
      }
    }
  }

  if (strlen (result_phrase) > 0)
    return 1;
  else
    return 0;
}



long
g_TUI_Search_WildChar (char cmpChar, char *result_phrase, char *root_buff,
		       int start, int p_fp, long szPos[][95])
{

  static int new_start;
  static int s0 = 0;
  static int s1 = 0;

  if (start == 0) {
    new_start = start;
    s0 = 0;
    s1 = 0;
  } else {
    new_start = start;
  }


  for (; s0 < 95; s0++) {
    for (; s1 < 95; s1++) {
      if (szPos[s0][s1] > 0) {
	int rs = s_TUI_Search_WildChar (cmpChar, s0 + 32, s1 + 32,
					result_phrase, root_buff, new_start,
					p_fp, szPos);
	if (rs > 0)
	  return 1;
	else
	  new_start = 0;
      }
    }
    s1 = 0;
  }

  return 0;
}



long
TUI_Search (char *result_phrase, char *root_buff,
	    int start, int p_fp, long szPos[][95])
{
  int buff_count;

  int i;
  int rs;

  buff_count = strlen (root_buff);

  if (buff_count == 1) {
    root_buff[1] = '\040';
    root_buff[2] = '\0';
    buff_count = 2;
  }

  for (i = 0; i < buff_count; i++) {
    if (root_buff[i] == 0x10)
      return rs =
	g_TUI_Search_WildChar (0x10, result_phrase, root_buff, start, p_fp,
			       szPos);

    else if (root_buff[i] == 0x11)
      return rs =
	g_TUI_Search_WildChar (0x11, result_phrase, root_buff, start, p_fp,
			       szPos);

    else if (root_buff[i] == 0x12)
      return rs =
	g_TUI_Search_WildChar (0x12, result_phrase, root_buff, start, p_fp,
			       szPos);
  }

  rs = g_TUI_Search_Exact (result_phrase, root_buff, start, p_fp, szPos);

  return rs;
}


int NEWIncrePreview (char *szOut, char *root_buff, int tty_fd, int disp_selkey, int pTUI_file_pointer, int pp)
{
  int buff_count;

  static long s_read_len = 0;
  int get_count = 0;


  char szSearch[21] = "";
  char szPhrase[100] = "";
  char szMisc[19] = "";

  char ch;
  char len;
  int n;
  int nbyte = 0;
  char result_phrase[256] = "";
  int str_len;
  int temp_file_pointer;
  int default_number;
  int pos_array[10];
  int r_phrase_count;
  int start = 0;


  temp_file_pointer = pTUI_file_pointer;
  r_phrase_count = 0;
  buff_count = strlen (root_buff);

//    if( (strchr(root_buff,0x10)!=0) ||
//        (strchr(root_buff,0x11)!=0) ||
//      (strchr(root_buff,0x12)!=0) )
//    { 
  if (gsCurrent_input_table->CurImmDispAttr == 0)
    return 0;
//    }

  if (buff_count == 1) {
    root_buff[1] = '\040';
    root_buff[2] = '\0';
    buff_count = 2;
  }

  if (start == 0) {
    int s0 = (int) root_buff[0] - 32;
    int s1 = (int) root_buff[1] - 32;

    long start_offset = gsCurrent_input_table->tui_index_table[pp][s0][s1];	// 依詞鍵前兩碼取得取得索引向量表起始值

    char id;
    int n;

    s_read_len = 0;
    get_count = 0;

    if (start_offset == 0) {
      return 0;
    }

    _llseek (temp_file_pointer, start_offset, SEEK_SET);

    s_read_len = Find_BlockLength (root_buff[0], root_buff[1], gsCurrent_input_table->tui_index_table[pp]);	//取得到下一組兩碼詞鍵間有多少總長度



    n = _lread (temp_file_pointer, (char *) &id, 1);

    s_read_len -= n;

  }


  while (s_read_len > 0) {

    result_phrase[0] = '\0';

    nbyte = 0;

    szSearch[0] = root_buff[0];
    szSearch[1] = root_buff[1];
    szSearch[2] = '\0';

    //----------------------------------------------------
    n = _lread (temp_file_pointer, (char *) &ch, 1);
    if (n < 1)
      break;
    nbyte += n;

    if (ch > 2) {
      n = _lread (temp_file_pointer, (char *) szMisc, (ch - 2));
      nbyte += n;

      szMisc[ch - 2] = '\0';
      strcat (szSearch, szMisc);
    }

    n = _lread (temp_file_pointer, (char *) &len, 1);
    nbyte += n;

    n = _lread (temp_file_pointer, (char *) szPhrase, len);
    szPhrase[len] = '\0';
    nbyte += n;

    //-------------------------------

    s_read_len -= nbyte;

    //-------------------------------

    if (strncmp (root_buff, szSearch, buff_count) == 0) {

      strcpy (result_phrase, szPhrase);
      get_count++;

      strcpy (gWaitingphrase[gWaitingPhraseCount], result_phrase);
      str_len = strlen (gWaitingphrase[gWaitingPhraseCount]);


      if (gEncode == BIG5) {
	if (gsCurrent_input_table->TableEncode == BIG5) {
	  input_print_string (gWaitingPos, 1,
			      gWaitingphrase[gWaitingPhraseCount],
			      (ch ==
			       buff_count) ? PHRASE_MATCH_COLOR :
			      PHRASE_MATCHPART_COLOR, INPUT_BGCOLOR);
	}
	if (gsCurrent_input_table->TableEncode == GB) {
	  input_print_string (gWaitingPos, 1,
			      string_GBtoBIG5 (gWaitingphrase
					       [gWaitingPhraseCount]),
			      (ch ==
			       buff_count) ? PHRASE_MATCH_COLOR :
			      PHRASE_MATCHPART_COLOR, INPUT_BGCOLOR);
	}
      }
      if (gEncode == GB) {
	if (gsCurrent_input_table->TableEncode == GB) {
	  input_print_string (gWaitingPos, 1,
			      gWaitingphrase[gWaitingPhraseCount],
			      (ch ==
			       buff_count) ? PHRASE_MATCH_COLOR :
			      PHRASE_MATCHPART_COLOR, INPUT_BGCOLOR);
	}
	if (gsCurrent_input_table->TableEncode == BIG5) {
	  input_print_string (gWaitingPos, 1,
			      string_BIG5toGB (gWaitingphrase
					       [gWaitingPhraseCount]),
			      (ch ==
			       buff_count) ? PHRASE_MATCH_COLOR :
			      PHRASE_MATCHPART_COLOR, INPUT_BGCOLOR);
	}
      }

      gWaitingPos++;
      pos_array[gWaitingPhraseCount] = gWaitingPos;
      gWaitingPos += str_len + 2;
      gWaitingPhraseCount++;


      if (gWaitingPos >= MAX_SEL_LENGTH)
	break;

      if (get_count >= MAX_SEL_COUNT)
	break;


    } else {
      if (buff_count < 3) {
	fprintf (stderr, "%c", 0x7);
	break;
      }
    }
  }

  return get_count;
}
