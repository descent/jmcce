/*
	$Id: chewingio.c,v 1.1.1.1 2002/05/03 04:01:07 kids Exp $
	Implement some routines for chewing
*/
#include "chewingio.h"
#include "global.h"
#include "zuin.h"
#include "chewingutil.h"
#include "userphrase.h"
#include <string.h>
#include <ctype.h>
#ifdef DEBUG
#include <stdio.h>
extern FILE *fp_g ;
#endif

char *kb_type_str[]={ 
"KB_DEFAULT",
"KB_HSU",
"KB_IBM",
"KB_GIN_YIEH",
"KB_ET",
"KB_ET26"
};

// 1999.12.13
int KBStr2Num(char str[])
{
  int i;
  
  for(i=0;i<KB_TYPE_NUM;i++) {
    if(!strcmp(str, kb_type_str[i]))
      return i;
  }
  return KB_DEFAULT ;
}

void SetKBType(ZuinData *pZuin, int kbtype)
{
  pZuin->kbtype = kbtype ;
}

int InitChewing(void *iccf, ChewingConf *cf)
{
  ChewingData *pgdata = (ChewingData *)iccf ;

  // zuinData
  memset(&(pgdata->zuinData), 0, sizeof(ZuinData)) ;

  SetKBType( &(pgdata->zuinData), cf->kb_type) ;

  // choiceInfo
  memset(&(pgdata->choiceInfo), 0, sizeof(ChoiceInfo)) ;

  pgdata->chiSymbolCursor = 0 ;
  pgdata->chiSymbolBufLen = 0 ;
  pgdata->nPhoneSeq = 0 ;
  pgdata->cursor = 0 ;
  memset(pgdata->bUserArrCnnct, 0, sizeof(int)*(MAX_PHONE_SEQ_LEN+1)) ; // 2000.5.26
  memset(pgdata->bUserArrBrkpt, 0, sizeof(int)*(MAX_PHONE_SEQ_LEN+1)) ;
  pgdata->bChiSym = CHINESE_MODE ;   
  pgdata->bSelect = 0 ;
  pgdata->nSelect = 0 ;
  return 0 ;
}

int SetConfig(void *iccf, ConfigData *pcd)
{
  ChewingData *pgdata = (ChewingData *)iccf ;
  
  pgdata->config.selectAreaLen = pcd->selectAreaLen ;
  pgdata->config.maxChiSymbolLen = pcd->maxChiSymbolLen ;
  memcpy(pgdata->config.selKey,
	 pcd->selKey, sizeof(pcd->selKey[0])*MAX_SELKEY) ;
  return 0;
}

void SetChiEngMode(void *iccf,int mode)
{
  ChewingData *pgdata = (ChewingData *)iccf ;

  if( pgdata->bFirstKey == 0 ) {
    pgdata->bChiSym = mode;
    pgdata->bCaseChange = ( mode == CHINESE_MODE ? 0 : 1 );
    pgdata->bFirstKey = 1;
  }
}

int GetChiEngMode(void *iccf) 
{
  ChewingData *pgdata = (ChewingData *)iccf ;

  return pgdata->bChiSym;
}

int OnKeyEsc(void *iccf, ChewingOutput *pgo)
{
  ChewingData *pgdata = (ChewingData *)iccf ;
  int keystrokeRtn = KEYSTROKE_ABSORB ;

  if( ! ChewingIsEntering(pgdata)) {
    keystrokeRtn = KEYSTROKE_IGNORE ;
  }
  else if(pgdata->bSelect) {
    ChoiceEndChoice(pgdata) ;
  }
  else if(ZuinIsEntering(&(pgdata->zuinData))) {
    ZuinRemoveAll(&(pgdata->zuinData)) ;
  }
 
  MakeOutputWithRtn(pgo, pgdata, keystrokeRtn) ;
  return 0 ;
}

int OnKeyEnter(void *iccf, ChewingOutput *pgo)
{
  ChewingData *pgdata = (ChewingData *)iccf ;
  int nCommitStr = pgdata->chiSymbolBufLen ;
  int keystrokeRtn = KEYSTROKE_ABSORB ;

  if( ! ChewingIsEntering(pgdata) ) {
    keystrokeRtn = KEYSTROKE_IGNORE ;
  }
  else if( pgdata->bSelect ) {
    keystrokeRtn = KEYSTROKE_ABSORB | KEYSTROKE_BELL ;
  }
  else {
    keystrokeRtn = KEYSTROKE_COMMIT;
    WriteChiSymbolToBuf(pgo->commitStr, nCommitStr, pgdata) ;
    AutoLearnPhrase( pgdata ) ;
    CleanAllBuf(pgdata) ;  
    CallPhrasing(pgdata) ;
    pgo->nCommitStr = nCommitStr ;
  }

  MakeOutputWithRtn(pgo, pgdata, keystrokeRtn) ;
  return 0 ;
}

int OnKeyDel(void *iccf, ChewingOutput *pgo)
{
  ChewingData *pgdata = (ChewingData *)iccf ;
  int keystrokeRtn = KEYSTROKE_ABSORB ;

  if( ! ChewingIsEntering(pgdata)) {
    keystrokeRtn = KEYSTROKE_IGNORE ;
  }

  if(! pgdata->bSelect) {
    if( !ZuinIsEntering( &(pgdata->zuinData)) && 
	pgdata->chiSymbolCursor < pgdata->chiSymbolBufLen) {
      ChewingKillChar(pgdata, pgdata->cursor, pgdata->chiSymbolCursor, 
		      NONDECREASE_CURSOR) ;
    }
    CallPhrasing(pgdata);
  }
  MakeOutputWithRtn(pgo, pgdata, keystrokeRtn) ;
  return 0 ;
}

int OnKeyBackspace(void *iccf, ChewingOutput *pgo)
{
  ChewingData *pgdata = (ChewingData *)iccf ;
  int keystrokeRtn = KEYSTROKE_ABSORB ;

  if( ! ChewingIsEntering(pgdata)) {
    keystrokeRtn = KEYSTROKE_IGNORE ;
  }

  if(! pgdata->bSelect) {
    if( ZuinIsEntering(&(pgdata->zuinData))) {
      ZuinRemoveLast(&(pgdata->zuinData)) ;
    }
    else if( pgdata->chiSymbolCursor > 0 ) {
      ChewingKillChar(pgdata, pgdata->cursor-1, pgdata->chiSymbolCursor-1, 
		      DECREASE_CURSOR) ;
    }
    CallPhrasing(pgdata);
  }
  MakeOutputWithRtn(pgo, pgdata, keystrokeRtn) ;

  return 0 ;
}

int OnKeyUp(void *iccf, ChewingOutput *pgo)
{
  ChewingData *pgdata = (ChewingData *)iccf ;
  int keystrokeRtn = KEYSTROKE_ABSORB ;

  if( ! ChewingIsEntering(pgdata)) {
    keystrokeRtn = KEYSTROKE_IGNORE ;
  }

  if( pgdata->bSelect )
    ChoicePrevAvail(pgdata) ;
  
  MakeOutputWithRtn(pgo, pgdata, keystrokeRtn) ;
  return 0 ;
}

int OnKeyDown(void *iccf, ChewingOutput *pgo)
{
  ChewingData *pgdata = (ChewingData *)iccf ;
  int toSelect = 0 ;
  int keystrokeRtn = KEYSTROKE_ABSORB ;

  if( ! ChewingIsEntering(pgdata)) {
    keystrokeRtn = KEYSTROKE_IGNORE ;
  }

  // see if to select 1999.9.9
  if(pgdata->chiSymbolCursor == pgdata->chiSymbolBufLen) {
    if(ChewingIsChiAt(pgdata->chiSymbolCursor - 1, pgdata) )
      toSelect = 1 ;
  }
  else {
    if(ChewingIsChiAt(pgdata->chiSymbolCursor, pgdata) ) 
      toSelect = 1 ;
  }
     
  if(toSelect) {  
    if( ! pgdata->bSelect ) {
      ChoiceFirstAvail(pgdata) ;
    }
    else {
      ChoiceNextAvail(pgdata) ;
    }
  }

  MakeOutputWithRtn(pgo, pgdata, keystrokeRtn) ;
  return 0 ;
}

// 99.8.12 
int OnKeyLeft(void *iccf, ChewingOutput *pgo)
{
  ChewingData *pgdata = (ChewingData *)iccf ;
  int keystrokeRtn = KEYSTROKE_ABSORB ;

  if( ! ChewingIsEntering(pgdata)) {
    keystrokeRtn = KEYSTROKE_IGNORE ;
  }

  if( pgdata->bSelect) {
    if( pgdata->choiceInfo.pageNo > 0)
      pgdata->choiceInfo.pageNo -- ;
  }
  else {
    if( !ZuinIsEntering(&(pgdata->zuinData)) && pgdata->chiSymbolCursor > 0) {
      pgdata->chiSymbolCursor--;
      if( pgdata->cursor > 0 && 
	  ChewingIsChiAt(pgdata->chiSymbolCursor, pgdata) )
	pgdata->cursor--;
    }
  }
  MakeOutputWithRtn(pgo, pgdata, keystrokeRtn) ;
  return 0 ;
}

// 99.8.12 
int OnKeyRight(void *iccf, ChewingOutput *pgo)
{
  ChewingData *pgdata = (ChewingData *)iccf ;
  int keystrokeRtn = KEYSTROKE_ABSORB ;

  if( ! ChewingIsEntering(pgdata)) {
    keystrokeRtn = KEYSTROKE_IGNORE ;
  }
  
  if( pgdata->bSelect ) {
    if( pgdata->choiceInfo.pageNo < pgdata->choiceInfo.nPage - 1)
      pgdata->choiceInfo.pageNo ++ ;
  }
  else {
    if( !ZuinIsEntering(&(pgdata->zuinData)) && 
	pgdata->chiSymbolCursor < pgdata->chiSymbolBufLen) {
      if( pgdata->cursor < pgdata->nPhoneSeq && 
	  ChewingIsChiAt(pgdata->chiSymbolCursor, pgdata ) )
	pgdata->cursor++;
      pgdata->chiSymbolCursor++;
    }
  }

  MakeOutputWithRtn(pgo, pgdata, keystrokeRtn) ;
  return 0 ;
}

// 99/8/12
int OnKeyTab(void *iccf, ChewingOutput *pgo)
{
  ChewingData *pgdata = (ChewingData *)iccf ;
  int keystrokeRtn = KEYSTROKE_ABSORB ;

  if( ! ChewingIsEntering(pgdata)) {
    keystrokeRtn = KEYSTROKE_IGNORE ;
  }


  if(! pgdata->bSelect ) {
    if( ChewingIsChiAt( pgdata->chiSymbolCursor-1, pgdata)) {
      if( IsPreferIntervalConnted( pgdata->cursor, pgdata) ) {
	pgdata->bUserArrBrkpt[ pgdata->cursor] = 1;
	pgdata->bUserArrCnnct[ pgdata->cursor] = 0;
      }
      else {
	pgdata->bUserArrBrkpt[ pgdata->cursor] = 0;
	pgdata->bUserArrCnnct[ pgdata->cursor] = 1;
      }
    }
    CallPhrasing(pgdata) ;
  }
  MakeOutputWithRtn(pgo, pgdata, keystrokeRtn);
  return 0 ;
}

// 2000/5/26
int OnKeyDblTab(void *iccf, ChewingOutput *pgo)
{
  ChewingData *pgdata = (ChewingData *)iccf ;
  int keystrokeRtn = KEYSTROKE_ABSORB ;

  if( ! ChewingIsEntering(pgdata)) {
    keystrokeRtn = KEYSTROKE_IGNORE ;
  }

  if(! pgdata->bSelect ) {
    pgdata->bUserArrBrkpt[ pgdata->cursor] = 0;
    pgdata->bUserArrCnnct[ pgdata->cursor] = 0;
  }
  CallPhrasing(pgdata) ;
  
  MakeOutputWithRtn(pgo, pgdata, keystrokeRtn);
  return 0 ;
}


// 99/8/12
int OnKeyCapslock(void *iccf, ChewingOutput *pgo)
{
  ChewingData *pgdata = (ChewingData *)iccf ;

  pgdata->bChiSym =  1 - pgdata->bChiSym ;
  pgdata->bCaseChange = ( pgdata->bChiSym == CHINESE_MODE ? 0 : 1 );
  MakeOutputWithRtn(pgo,pgdata,KEYSTROKE_ABSORB);
  return 0 ;
}

int OnKeyHome(void *iccf, ChewingOutput *pgo)
{
  ChewingData *pgdata = (ChewingData *)iccf ; 
  int keystrokeRtn = KEYSTROKE_ABSORB ; 
 
  if( ! ChewingIsEntering(pgdata)) { 
    keystrokeRtn = KEYSTROKE_IGNORE ; 
  } 
  else if( !pgdata->bSelect) {
    pgdata->chiSymbolCursor = 0; 
    pgdata->cursor = 0 ;
  }
  MakeOutputWithRtn(pgo, pgdata, keystrokeRtn); 
  return 0;
}

int OnKeyEnd(void *iccf,ChewingOutput *pgo)
{
  ChewingData *pgdata = (ChewingData *)iccf ; 
  int keystrokeRtn = KEYSTROKE_ABSORB ; 
 
  if( ! ChewingIsEntering(pgdata)) { 
    keystrokeRtn = KEYSTROKE_IGNORE ; 
  } 
  else if( !pgdata->bSelect) { 
    pgdata->chiSymbolCursor = pgdata->chiSymbolBufLen; 
    pgdata->cursor = pgdata->nPhoneSeq ; 
  } 
  MakeOutputWithRtn(pgo, pgdata, keystrokeRtn);       
  return 0;
}


int OnKeyDefault(void *iccf, int key, ChewingOutput *pgo)
{
  ChewingData *pgdata = (ChewingData *)iccf;
  int rtn, num ;
  int keystrokeRtn = KEYSTROKE_ABSORB ;
  int bQuickCommit = 0;

  // Skip the special key 2000.4.21
  if( key & 0xFF00 ) {
    keystrokeRtn = KEYSTROKE_IGNORE ;
    goto End_OnKeyDefault ;
  }
  
#ifdef DEBUG
  fprintf( fp_g, "OnKeyDefault: key=%d\n",key );
#endif

/*
  if( pgdata->bCaseChange && isalpha(key) ) {
    if(islower(key)) key = key - 'a' + 'A' ;
    else if(isupper(key)) key = key - 'A' + 'a' ;
  }
*/

  if( pgdata->bSelect ) {  // selecting
    if(key == ' ')
      return OnKeyRight(iccf, pgo) ;
    num = CountSelKeyNum(key, pgdata) ;  // num starts from 0
    if(num >= 0) {
      num += pgdata->choiceInfo.pageNo * pgdata->choiceInfo.nChoicePerPage ;

      // change the select interval & selectStr & nSelect
      AddSelect(pgdata, num) ;

      // second, call choice module
      ChoiceSelect(pgdata, num) ;
    }
  }
  else {  // editing
    if( pgdata->bChiSym == CHINESE_MODE ) {
      rtn = ZuinPhoInput( &(pgdata->zuinData), key ) ;
#ifdef DEBUG
	fprintf(fp_g, "\t\tchinese mode key, ZuinPhoInput return value = %d\n", rtn) ;
	fflush(fp_g) ;
#endif
      if(rtn == ZUIN_KEY_ERROR) rtn = SpecialSymbolInput(key, pgdata) ;
      switch(rtn) {
      case ZUIN_ABSORB:
	keystrokeRtn = KEYSTROKE_ABSORB ;
	break ;
      case ZUIN_COMMIT:
	AddChi(pgdata->zuinData.phone, pgdata) ;
	break ;
      case ZUIN_NO_WORD:
	keystrokeRtn = KEYSTROKE_BELL | KEYSTROKE_ABSORB ;
	break ;
      case ZUIN_KEY_ERROR:
      case ZUIN_IGNORE:
	  
	  // by lckung 2001.10.22
	  // change upper case into lower case
#ifdef DEBUG
	  fprintf(fp_g, "\t\tbefore isupper(key),key=%d\n", key);
#endif
	  if( isupper(key) ) 
	  	key = tolower(key);

#ifdef DEBUG
	  fprintf(fp_g, "\t\tafter isupper(key),key=%d\n", key);
#endif
	  
	  if( pgdata->chiSymbolBufLen == 0) { // if nothing in buffer
	  	bQuickCommit = 1;
	  }

	  rtn = SymbolInput(key, pgdata) ;
	  if(rtn == SYMBOL_KEY_ERROR)
	    keystrokeRtn = KEYSTROKE_IGNORE ;
	  else
	    keystrokeRtn = KEYSTROKE_ABSORB ;
	  break ;

      } // end of swtich(rtn)
    } // end of if( CHINESE_MODE)
    else {
      // English, under construction
	  if( pgdata->chiSymbolBufLen == 0) { // if nothing in buffer
	  	bQuickCommit = 1;
	  }
      rtn = SymbolInput(key, pgdata) ;
      if( rtn == SYMBOL_KEY_ERROR ) {
		keystrokeRtn = KEYSTROKE_IGNORE ;
		bQuickCommit = 0;
	  }
    }
	if( !bQuickCommit ) {
    	CallPhrasing(pgdata) ;
    	if( ReleaseChiSymbolBuf(pgdata, pgo) != 0 )
      		keystrokeRtn = KEYSTROKE_COMMIT ;
	}
	else { // Quick Commit
#ifdef DEBUG
		fprintf(fp_g, "\t\tQuick commit buf[0]=%c\n", pgdata->chiSymbolBuf[0].s[0]);
#endif
		pgo->commitStr[0].wch = pgdata->chiSymbolBuf[0].wch; 
		pgo->nCommitStr = 1;
		pgdata->chiSymbolBufLen = 0;
		pgdata->chiSymbolCursor = 0;
   		keystrokeRtn = KEYSTROKE_COMMIT ;
	}
  }

 End_OnKeyDefault:
  CallPhrasing(pgdata) ;
  MakeOutputWithRtn(pgo, pgdata, keystrokeRtn); 

  return 0 ;
}

int OnKeyCtrlNum(void *iccf, int key, ChewingOutput *pgo)
{
  ChewingData *pgdata = (ChewingData *)iccf; 
  int keystrokeRtn = KEYSTROKE_ABSORB ;
  int newPhraseLen ;
  int i ;
  uint16 addPhoneSeq[MAX_PHONE_SEQ_LEN] ;
  char addWordSeq[MAX_PHONE_SEQ_LEN*2+1] ;
  int phraseState ;

  CallPhrasing(pgdata) ;
  newPhraseLen = key - '0' ;
  if(newPhraseLen >= 1 && pgdata->cursor+newPhraseLen-1 <= pgdata->nPhoneSeq) {
    if(NoSymbolBetween(pgdata, pgdata->cursor, pgdata->cursor+newPhraseLen-1)) {
      // Manually add phrase to the user phrase database.  (kpchen 2000.4.28)
      memcpy(addPhoneSeq, &pgdata->phoneSeq[pgdata->cursor], 
	     sizeof(uint16)*newPhraseLen) ;
      addPhoneSeq[newPhraseLen] = 0 ;
      memcpy(addWordSeq, &pgdata->phrOut.chiBuf[pgdata->cursor*2], 
	     sizeof(char)*2*newPhraseLen) ;
      addWordSeq[newPhraseLen*2] = '\0' ;

      phraseState = UserUpdatePhrase(addPhoneSeq, addWordSeq) ;
      SetUpdatePhraseMsg(pgdata, addWordSeq, newPhraseLen, phraseState) ; // kpchen 2000.5.12

      // Clear the breakpoint between the New Phrase (kpchen 2000.5.12)
      for(i=1; i<newPhraseLen; i++)
	pgdata->bUserArrBrkpt[pgdata->cursor + i] = 0 ;
    }
  }
  CallPhrasing(pgdata) ;
  MakeOutputWithRtn(pgo, pgdata, keystrokeRtn) ;
  MakeOutputAddMsgAndCleanInterval(pgo, pgdata) ;
  return 0 ;
}

int OnKeyCtrlDefault(void *iccf,ChewingOutput *pgo)
{
  ChewingData *pgdata = (ChewingData *)iccf;

  CallPhrasing(pgdata) ;
  MakeOutputWithRtn(pgo, pgdata, KEYSTROKE_IGNORE) ;
  return 0 ;
}
