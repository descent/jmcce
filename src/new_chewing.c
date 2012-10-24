#include "new_chewing.h"
#include "hzinput.h"

#include "chewing.h"
#if 0
/* Only used by calculating char position */
#include "internal/chewing-utf8-util.h"
#endif

#include "encoding.h"

#include <cstdlib>

#include <string>
#include <vector>

#include <unistd.h>

#define KEY_ESC '\033'
#define KEY_BACK '\177'
#define KEY_DEL (126)
#define KEY_SPACE       (32)
#define KEY_TAB '\t'
#define KEY_CTRL_W 23
#define KEY_CTRL_Z 26


ChewingContext* ct_;

const int MAX_PHONE_SEQ_LEN = 50;
unsigned short g_lastPhoneSeq[MAX_PHONE_SEQ_LEN] = {0};

namespace
{
const int ZUIN_POS_X=16;
const int ZUIN_POS_Y=0;
const int CHOOSE_BUFFER_POS_X=1;
const int CHOOSE_BUFFER_POS_Y=1;
const int EDIT_BUFFER_POS_X=24;
const int EDIT_BUFFER_POS_Y=0;
const int KEYBOARD_LAYOUT_POS_X=60;
const int KEYBOARD_LAYOUT_POS_Y=1;

enum { H_NORMAL, H_ESCAPE, H_FUNC_KEY, H_XXX } h_state;
}

int selKeys[] = {'1', '2', '3', '4', '5', '6', '7', '8', '9', 0};

extern FILE *fs;

bool new_chew_init()
{
  chewing_Init("/usr/lib/i386-linux-gnu/libchewing3/chewing", ".");
  ct_ = chewing_new();

  /* Set keyboard type */
  //chewing_set_KBType( ct_, chewing_KBStr2Num( "KB_DEFAULT" ) );
  chewing_set_KBType( ct_, chewing_KBStr2Num( "KB_ET26" ) );

  // ref: http://starforcefield.wordpress.com/2012/08/13/%E6%8E%A2%E7%B4%A2%E6%96%B0%E9%85%B7%E9%9F%B3%E8%BC%B8%E5%85%A5%E6%B3%95%EF%BC%9A%E4%BD%BF%E7%94%A8libchewing/
  /* 設定選擇候選字的快速鍵。如果不設定就不能選擇候選字的樣子。*/
  chewing_set_selKey(ct_, selKeys, 9);


        /* Fill configuration values */
        chewing_set_candPerPage( ct_, 9 );
        chewing_set_maxChiSymbolLen( ct_, 16 );
        chewing_set_addPhraseDirection( ct_, 1 );
        chewing_set_spaceAsSelection( ct_, 1 );
        chewing_set_autoShiftCur( ct_, 1 );
        chewing_set_phraseChoiceRearward( ct_, 1 );


}



void show_interval_buffer(ChewingContext *ctx )
{
#if 0
	char *buf;
	char *p;
	int buf_len;
	char out_buf[ 100 ];
	int i, count;
	int arrPos[ 50 ];
	IntervalType it;


	/* Check if buffer is available. */
	if ( ! chewing_buffer_Check( ctx ) ) {
		return;
	}

	buf = chewing_buffer_String( ctx );
	buf_len = chewing_buffer_Len( ctx );

	p = buf;
	count = 0;
	for ( i = 0 ;i < buf_len; i++ ) {
		arrPos[ i ] = count;
		count += ueBytesFromChar(*p) <= 1 ? 1 : 2;
		p += ueBytesFromChar(*p);
	}
	arrPos[ i ] = count;

	memset( out_buf, ' ', count * ( sizeof( char ) ) );
	out_buf[ count ] = '\0';

	chewing_interval_Enumerate( ctx );
	while ( chewing_interval_hasNext( ctx ) ) {
		chewing_interval_Get( ctx, &it );
		out_buf[ arrPos[ it.from ] ] = '[';
		out_buf[ arrPos[ it.to ] - 1 ] =  ']';
		memset(
			&out_buf[ arrPos[ it.from ] + 1 ], '-',
			arrPos[ it.to ] - arrPos[ it.from ] - 2 );
	}
	//addstr( out_buf );
#endif
}

void show_choose_buffer(ChewingContext *ctx )
{
  int i = 1;
  int currentPageNo;
  char str[ 20 ];
  char *cand_string;
  std::vector<std::string> select_str;
	
  if ( chewing_cand_TotalPage( ctx ) == 0 )
    return;
	
  chewing_cand_Enumerate( ctx );
  while ( chewing_cand_hasNext( ctx ) ) 
  {
    if ( i > chewing_cand_ChoicePerPage( ctx ) )
      break;
    sprintf( str, "%d.", i );

    cand_string = chewing_cand_String( ctx );
    sprintf( str, " %s ", cand_string );

    std::string big5_str;
    if (utf8_to_big5(cand_string, big5_str) == 0)
      select_str.push_back(big5_str); 
    else
      select_str.push_back("??");


    free( cand_string );
    i++;
  }

  int input_pos = CHOOSE_BUFFER_POS_X;
  int list_num = select_str.size();
  const int INPUT_SELECT_DISPLAY_MAX = 18;
  if (list_num > INPUT_SELECT_DISPLAY_MAX)
    list_num = INPUT_SELECT_DISPLAY_MAX;

  input_print_string (input_pos+1, CHOOSE_BUFFER_POS_Y, "                                   ", INPUT_FGCOLOR, INPUT_BGCOLOR);
  int index=1;
  for (int i=0 ; i < list_num ; ++i)
  {
    char index_str[4];

    sprintf(index_str, "%d", index);
    input_print_string (input_pos, CHOOSE_BUFFER_POS_Y, index_str, INPUT_FGCOLOR, INPUT_BGCOLOR);
    input_print_string (input_pos+1, CHOOSE_BUFFER_POS_Y, select_str[i].c_str(), INPUT_FGCOLOR, INPUT_BGCOLOR);
    input_pos+=4;
    ++index;
  }

  currentPageNo = chewing_cand_CurrentPage( ctx );
  if ( chewing_cand_TotalPage( ctx ) != 1 ) 
  {
    if ( currentPageNo == 0 )
			;//addstr( "  >" );
    else if ( currentPageNo == ( chewing_cand_TotalPage( ctx ) - 1 ) )
			;//addstr( "<  " );
    else
			;//addstr( "< >" );
  }
}

void show_zuin_buffer(ChewingContext *ctx )
{
	int zuin_count;
	char *zuin_string;
#if 0
	if ( chewing_get_ChiEngMode( ctx ) )
		addstr( "[中]" );
	else
		addstr( "[英]" );
	addstr( "        " );
#endif
	zuin_string = chewing_zuin_String( ctx, &zuin_count );

        input_print_string (16, 0, "       ", INPUT_FGCOLOR, INPUT_BGCOLOR);
    for (int i=0 ; i < zuin_count ; ++i)
    {
        std::string big5_str;
        if (utf8_to_big5(zuin_string, big5_str) == 0)
        {
          input_print_string (ZUIN_POS_X, ZUIN_POS_Y, big5_str.c_str(), INPUT_FGCOLOR, INPUT_BGCOLOR);
        }
    }

	//addstr( zuin_string );
	free( zuin_string );
}


void show_full_shape(ChewingContext *ctx )
{
#if 0
	move( x, y );
	addstr( "[" );
	if ( hasColor )
		attron( COLOR_PAIR( 2 ) );
	if ( chewing_get_ShapeMode( ctx ) == FULLSHAPE_MODE )
		addstr( "全形" );
	else
		addstr( "半形" );
	if ( hasColor )
		attroff( COLOR_PAIR( 2 ) );
	addstr( "]" );
#endif
}


void show_commit_string(ChewingContext *ctx, bool commit_action, int tty_fd)
{
  if (chewing_commit_Check(ct_)) 
  {
    char *buf = chewing_commit_String(ct_);
    std::string big5_str;
    if (utf8_to_big5(buf, big5_str) == 0)
    {
      //input_print_string (16, 0, "        ", INPUT_FGCOLOR, INPUT_BGCOLOR);
      //input_print_string (16, 0, big5_str.c_str(), INPUT_FGCOLOR, INPUT_BGCOLOR);
      if (commit_action)
      {
        write (tty_fd, big5_str.c_str(), strlen(big5_str.c_str()));
        // clear zuin buf, choose buf, edit buf
        input_print_string (ZUIN_POS_X, ZUIN_POS_Y, "          ", INPUT_FGCOLOR, INPUT_BGCOLOR);
        input_print_string (CHOOSE_BUFFER_POS_X, CHOOSE_BUFFER_POS_Y, "                                        ", INPUT_FGCOLOR, INPUT_BGCOLOR);
        input_print_string (EDIT_BUFFER_POS_X, EDIT_BUFFER_POS_Y, "                ", INPUT_FGCOLOR, INPUT_BGCOLOR);
      }
    }

    free(buf);
  }
}
void show_userphrase(ChewingContext *ctx )
{
#if 0
	char *aux_string;
	if ( chewing_aux_Length( ctx ) == 0 )
		return;

	move( x, y );
	addstr( FILL_BLANK );
	move( x, y );
	if ( hasColor )
		attron( COLOR_PAIR( 2 ) );
	aux_string = chewing_aux_String( ctx );
	addstr( aux_string );
	free( aux_string );
	if ( hasColor )
		attroff( COLOR_PAIR( 2 ) );
#endif
}

void show_edit_buffer(ChewingContext *ctx )
{
  int i, cursor, count;
  char *buffer_string;
  char *p;
  if ( ! chewing_buffer_Check( ctx ) ) 
  {
		//move( x, y );
    return;
  }
  buffer_string = chewing_buffer_String( ctx );

  input_print_string (EDIT_BUFFER_POS_X, EDIT_BUFFER_POS_Y, "                ", INPUT_FGCOLOR, INPUT_BGCOLOR);

  std::string big5_str;
  if (utf8_to_big5(buffer_string, big5_str) == 0)
    input_print_string (EDIT_BUFFER_POS_X, EDIT_BUFFER_POS_Y, big5_str.c_str(), INPUT_FGCOLOR, INPUT_BGCOLOR);

        #if 0
	mvaddstr( x, y, buffer_string );
	cursor = chewing_cursor_Current( ctx );
	p = buffer_string;
	count = 0;
	for ( i = 0 ;i < cursor; i++ ) {
		count += ueBytesFromChar(*p) <= 1 ? 1 : 2;
		p += ueBytesFromChar(*p);
	}
	move( x, count );
        #endif
  free( buffer_string );
}


void new_chewing_hz_filter (int tty_fd, unsigned int key)
{
  static unsigned char last_key = 0;
  input_print_string (KEYBOARD_LAYOUT_POS_X, KEYBOARD_LAYOUT_POS_Y, "KB_ET26", INPUT_FGCOLOR, INPUT_BGCOLOR);

  //Live ();
  int zuin_count=0;

  if (h_state == H_NORMAL && key == KEY_ESC) 
  {
    chewing_handle_Esc(ct_);
    if (!chewing_buffer_Check( ct_ ) ) 
    {
      write (tty_fd, &key, sizeof (key));
      h_state = H_ESCAPE;
    }
    else
      h_state = H_NORMAL;

    //chewing_zuin_String( ct_, &zuin_count );
    //if (zuin_count == 0)
    //if (gOut.chiSymbolBufLen == 0)
      //write (tty_fd, &key, sizeof (key));

    return;
  }

  if (h_state == H_ESCAPE) {

    //if (gOut.chiSymbolBufLen == 0)
      //write (tty_fd, &key, sizeof (key));
    if (!chewing_buffer_Check( ct_ ) ) 
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

  if (h_state == H_FUNC_KEY) 
  {

    chewing_zuin_String( ct_, &zuin_count );
    if (zuin_count == 0)
    {
      write (tty_fd, &key, sizeof (key));
      h_state = H_NORMAL;
      return;
    }

#if 0
    if (gOut.chiSymbolBufLen == 0)  {
      write (tty_fd, &key, sizeof (key));
      h_state = H_NORMAL;
      return;
    }
#endif
    
    if (last_key == KEY_ESC)
      chewing_handle_Esc(ct_);

    switch (key) {
    case 'A':			/* UP */

      //OnKeyUp (pgdata, &gOut);
      h_state = H_NORMAL;
      break;

    case 'B':			/* DOWN */
    {
      //OnKeyDown (pgdata, &gOut);
      chewing_handle_Down(ct_); 

      h_state = H_NORMAL;
      break;
    }
    case 'C':			/* RIGHT */

      //OnKeyRight (pgdata, &gOut);
      h_state = H_NORMAL;
      break;

    case 'D':			/* LEFT */

      //OnKeyLeft (pgdata, &gOut);
      h_state = H_NORMAL;
      break;

    default:

      h_state = H_NORMAL;

      break;


    }				/* switch */

    //ChewReDraw (&gOut);
    last_key = key;
    return;

  }

#if 0
  if (GetChiEngMode(pgdata) != GetCapsLockStatus(tty_fd))
    OnKeyCapslock (pgdata, &gOut);
#endif  

  bool commit_action = false;

  switch (key) 
  {
    case ' ':			/* SPACE */
    {
      chewing_handle_Space(ct_);
      break;
    }
    case '\010':                  /* BackSpace Ctrl+H */
    case '\177':                  /* BackSpace */
    {
#if 0
    if (gOut.chiSymbolBufLen == 0 && 
       (!*gOut.zuinBuf[0].s && !*gOut.zuinBuf[1].s && !*gOut.zuinBuf[2].s)) {
      write (tty_fd, &key, sizeof (key));
      return;
    }
#endif
      chewing_zuin_String( ct_, &zuin_count );
      if (zuin_count == 0)
      {
        write (tty_fd, &key, sizeof (key));
        return;
      }

      chewing_handle_Backspace(ct_);
      break;
    }
    case 13:			/* ENTER */
    {
    // ref: windows-chewing/ChewingServer/chewing.cpp
    //unsigned short *old_seq = chewing_get_phoneSeq(ct_);
    //int phon_seq_len = chewing_get_phoneSeqLen(ct_);

      if ( ! chewing_buffer_Check( ct_ ) ) 
        write (tty_fd, &key, sizeof (key));
      chewing_handle_Enter(ct_);
      commit_action = true;

#if 0
    if (gOut.chiSymbolCursor == 0)
      write (tty_fd, &key, sizeof (key));
    OnKeyEnter (pgdata, &gOut);
#endif
    break;

    }
#if 0
  case 127:			/* BackSpace */

    if (gOut.chiSymbolBufLen == 0 && 
       (!*gOut.zuinBuf[0].s && !*gOut.zuinBuf[1].s && !*gOut.zuinBuf[2].s)) {
      write (tty_fd, &key, sizeof (key));
      return;
    }
      //chewing_handle_Backspace(ct_);
    break;
   #endif

#if 0
  case KEY_DEL:

    //OnKeyDel (pgdata, &gOut);
    break;


  case KEY_TAB:

    //OnKeyTab (pgdata, &gOut);
    break;

  case KEY_CTRL_W:	/* ³]©wÁä½L±Æ¦C¤è¦¡ */

    //SetKBINT();
    return;	
    break;
#endif
  default:
  {

   
    //if (key < KEY_CTRL_Z)	/* ¦pªG¬O CTRL+A ~ CTRL+Z «hª½±µ¿é¥X */
      //write (tty_fd, &key, sizeof (key));
 
    //OnKeyDefault (pgdata, key, &gOut);
      chewing_handle_Default( ct_, (char)key );

    }    
  }				/* switch */


// ref: libchewing/test/gen_keystroke.c
  show_interval_buffer(ct_ );
  show_choose_buffer(ct_ );
  show_zuin_buffer(ct_ );
  show_full_shape(ct_ );
  show_commit_string(ct_, commit_action, tty_fd );
  show_userphrase(ct_ );
  show_edit_buffer( ct_ );



#if 0
#endif

#if 0
  if (gOut.nCommitStr)
    ChewCommitString (tty_fd, &gOut);
  else
    ChewReDraw (&gOut);
#endif
  last_key = key;
  return;

}

