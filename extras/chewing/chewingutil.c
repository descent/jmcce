#include <ctype.h>
#include <string.h>
#include "global.h"
#include "chewingutil.h"
#include "zuin.h"
#include "userphrase.h"
#ifdef DEBUG
#include <stdio.h>
extern FILE *fp_g ;
#endif

extern const char *ph_pho[]; 
static void MakePreferInterval(ChewingData *pgdata) ;
static void ShiftInterval(ChewingOutput *pgo, ChewingData *pgdata) ;

/* put lifetime from chewing.c -- Cd Chen */
int lifetime;

void SetUpdatePhraseMsg(ChewingData *pgdata, char *addWordSeq, int len, int state)
{
  char *insert = "加入－", *modify = "已有－" ;
  int begin = 3, i ;

  pgdata->showMsgLen = begin + len ;
  if(state == USER_UPDATE_INSERT) {
    pgdata->showMsg[0].s[0] = insert[0] ;
    pgdata->showMsg[0].s[1] = insert[1] ;
    pgdata->showMsg[0].s[2] = 0 ;
    pgdata->showMsg[1].s[0] = insert[2] ;
    pgdata->showMsg[1].s[1] = insert[3] ;
    pgdata->showMsg[1].s[2] = 0 ;
    pgdata->showMsg[2].s[0] = insert[4] ;
    pgdata->showMsg[2].s[1] = insert[5] ;
    pgdata->showMsg[2].s[2] = 0 ;
  }
  else {
    pgdata->showMsg[0].s[0] = modify[0] ;
    pgdata->showMsg[0].s[1] = modify[1] ;
    pgdata->showMsg[0].s[2] = 0 ;
    pgdata->showMsg[1].s[0] = modify[2] ;
    pgdata->showMsg[1].s[1] = modify[3] ;
    pgdata->showMsg[1].s[2] = 0 ;
    pgdata->showMsg[2].s[0] = modify[4] ;
    pgdata->showMsg[2].s[1] = modify[5] ;
    pgdata->showMsg[2].s[2] = 0 ;
  }
  for(i=0; i<len; i++) {
    pgdata->showMsg[begin+i].s[0] = addWordSeq[i*2] ;
    pgdata->showMsg[begin+i].s[1] = addWordSeq[i*2+1] ;
    pgdata->showMsg[begin+i].s[2] = 0 ;
  }
  pgdata->showMsg[begin+i].s[0] = 0;
  pgdata->showMsg[begin+i].s[1] = 0;
  pgdata->showMsg[begin+i].s[2] = 0;
}

int NoSymbolBetween(ChewingData *pgdata, int begin, int end)
{
  int i, nChi, k ;

  // find the beginning index in the chiSymbolBuf
  for(nChi=i=0; i<pgdata->chiSymbolBufLen && nChi<begin; i++)
    if(pgdata->chiSymbolBuf[i].wch == (wchar_t)0 ) // if it is Chinese word.
      nChi++ ;

  for(k=i+1; k<pgdata->chiSymbolBufLen && k<=end; k++)
    if( pgdata->chiSymbolBuf[i].wch != (wchar_t) 0 ) // if not a Chinese word.
      return 0 ;

  return 1 ;
}

int ChewingIsEntering(ChewingData *pgdata)
{
  return pgdata->chiSymbolBufLen != 0 
    || ZuinIsEntering(&(pgdata->zuinData)) ;
}

int SpecialSymbolInput(int key, ChewingData *pgdata)
{
  static char keybuf[] = { '[', ']', '{', '}', '\'','<', ':', '\"', '>',
			   '~', '!', '@', '#', '$', '%', '^', '&', '*', 
			   '(', ')', '_', '+', '=','\\', '|', '?'} ;

  static char *chibuf[] = {"「","」","『","』","、","，","：","；","。",
			   "～","！","＠","＃","＄","％","︿","＆","＊",
			   "（","）","﹍","＋","＝","＼","｜","？"} ;

  static int nSpecial = 26 ;

  int i, rtn = ZUIN_IGNORE; // very strange , and very difficult to understand

  for(i=0; i<nSpecial; i++) {
    if(key == keybuf[i]) {
      rtn = ZUIN_ABSORB ;
      memmove( &(pgdata->chiSymbolBuf[pgdata->chiSymbolCursor + 1]),
	       &(pgdata->chiSymbolBuf[pgdata->chiSymbolCursor] ) ,
	       sizeof(wch_t)*
	       (pgdata->chiSymbolBufLen - pgdata->chiSymbolCursor) ) ;
      
      pgdata->chiSymbolBuf[pgdata->chiSymbolCursor].wch = (wchar_t) 0 ;
      pgdata->chiSymbolBuf[pgdata->chiSymbolCursor].s[0] = chibuf[i][0] ;
      pgdata->chiSymbolBuf[pgdata->chiSymbolCursor].s[1] = chibuf[i][1] ;
      pgdata->chiSymbolCursor ++ ;
      pgdata->chiSymbolBufLen ++ ;
      pgdata->bUserArrCnnct[pgdata->cursor] = 0 ; // 2000.7.5 
      break ;
    }
  }

  return rtn ;
}

int SymbolInput(int key, ChewingData *pgdata)
{
  if(isprint( (char)key)) { // other character was ignored
    memmove( &(pgdata->chiSymbolBuf[pgdata->chiSymbolCursor + 1]),
	     &(pgdata->chiSymbolBuf[pgdata->chiSymbolCursor] ) ,
	     sizeof(wch_t)*
	     (pgdata->chiSymbolBufLen - pgdata->chiSymbolCursor) ) ;
    
    pgdata->chiSymbolBuf[pgdata->chiSymbolCursor].wch = (wchar_t) 0 ;
    pgdata->chiSymbolBuf[pgdata->chiSymbolCursor].s[0] = (char) key ;
    pgdata->chiSymbolCursor ++ ;
    pgdata->chiSymbolBufLen ++ ;
    pgdata->bUserArrCnnct[pgdata->cursor] = 0 ; // 2000.7.5 
    return SYMBOL_KEY_OK ;
  }
  return SYMBOL_KEY_ERROR ;
}

static int CompInterval(const IntervalType *a, const IntervalType *b)
{
  int cmp = a->from - b->from ;
  if(cmp) return cmp ;
  return a->to - b->to ;
}

int FindIntervalFrom(int from, IntervalType inte[], int nInte)
{
  int i ;

  for(i=0; i<nInte; i++)
    if(inte[i].from == from)
      return i ;
  return -1 ;
}

int WriteChiSymbolToBuf(wch_t csBuf[], int csBufLen, ChewingData *pgdata)
{
  int i, phoneseq_i=0 ;

  for(i=0; i<csBufLen; i++) {
    if(ChewingIsChiAt(i, pgdata)) {
      csBuf[i].wch = (wchar_t)0;
      memcpy(csBuf[i].s, &(pgdata->phrOut.chiBuf[phoneseq_i]), 2) ;
      phoneseq_i += 2 ;
    }
    else 
      csBuf[i].wch = pgdata->chiSymbolBuf[i].wch ;
  }
  return 0 ;
}

int CountReleaseNum(ChewingData *pgdata)
{
  int remain, i ;
  
  // reserve 4 positions for Zuin 
  remain = pgdata->chiSymbolBufLen + 4 - pgdata->config.maxChiSymbolLen ; 
  if( remain <= 0) return 0 ; 
 
  qsort(pgdata->preferInterval, 
        pgdata->nPrefer, 
        sizeof(IntervalType), 
        (CompFuncType) CompInterval ) ; 
 
  if( ! ChewingIsChiAt(0, pgdata)) 
    return 1 ;
  i = FindIntervalFrom(0, pgdata->preferInterval, 
		       pgdata->nPrefer ) ;
  if(i >= 0) {
    return ( pgdata->preferInterval[i].to - 
	     pgdata->preferInterval[i].from ) ; 
  }
  else
    return 1 ;
}

void KillFromLeft(ChewingData *pgdata, int nKill)
{
  int i ;

  for(i=0; i<nKill; i++)
    ChewingKillChar(pgdata, 0, 0, DECREASE_CURSOR) ;
}

void CleanAllBuf(ChewingData *pgdata)
{
  // 1
  pgdata->nPhoneSeq = 0 ;
  memset(pgdata->phoneSeq, 0, sizeof(pgdata->phoneSeq)) ;
  // 2
  pgdata->chiSymbolBufLen = 0 ;
  memset(pgdata->chiSymbolBuf, 0, sizeof(pgdata->chiSymbolBuf)) ;
  // 3
  memset(pgdata->bUserArrBrkpt, 0, sizeof(pgdata->bUserArrBrkpt)) ;
  // 4
  pgdata->nSelect = 0 ;
  // 5
  pgdata->cursor = 0 ;
  // 6
  pgdata->chiSymbolCursor = 0 ;
  // 7
  memset(pgdata->bUserArrCnnct, 0, sizeof(pgdata->bUserArrCnnct)) ;
}

int ReleaseChiSymbolBuf(ChewingData *pgdata, ChewingOutput *pgo) 
{
  int throwEnd ;
  uint16 bufPhoneSeq[MAX_PHONE_SEQ_LEN+1] ;
  char bufWordSeq[MAX_PHONE_SEQ_LEN*2+1] ;

  throwEnd = CountReleaseNum(pgdata) ;

  pgo->nCommitStr = throwEnd ;
  if(throwEnd) {
    // count how many chinese words in "chiSymbolBuf[0 .. throwEnd-1]"
    // And release from "chiSymbolBuf" && "phoneSeq"
    WriteChiSymbolToBuf(pgo->commitStr, throwEnd, pgdata) ;

    // Add to userphrase
    memcpy(bufPhoneSeq, pgdata->phoneSeq, sizeof(uint16)*throwEnd) ;
    bufPhoneSeq[throwEnd] = (uint16) 0 ;
    memcpy(bufWordSeq, pgdata->phrOut.chiBuf, sizeof(char)*throwEnd*2) ;
    bufWordSeq[throwEnd*2] = '\0' ;
    UserUpdatePhrase(bufPhoneSeq, bufWordSeq) ;
    
    KillFromLeft(pgdata, throwEnd) ;
  }
  return throwEnd ;
}

void AutoLearnPhrase(ChewingData *pgdata)
{
  uint16 bufPhoneSeq[MAX_PHONE_SEQ_LEN+1] ;
  char bufWordSeq[MAX_PHONE_SEQ_LEN*2+1] ;
  int i, from, len ;

  for(i=0; i<pgdata->nPrefer; i++) {
    from = pgdata->preferInterval[i].from ;
    len = pgdata->preferInterval[i].to - from ;
    memcpy(bufPhoneSeq, &pgdata->phoneSeq[from], sizeof(uint16)*len) ;
    bufPhoneSeq[len] = (uint16) 0 ;
    memcpy(bufWordSeq, &pgdata->phrOut.chiBuf[from*2], sizeof(char)*len*2) ;
    bufWordSeq[len*2] = '\0' ;
    UserUpdatePhrase(bufPhoneSeq, bufWordSeq) ;
  }
}

// kpchen 1999.8.17
int AddChi(uint16 phone, ChewingData *pgdata)
{
  int i ;

  // shift the selectInterval  (kpchen 2000.5.12)
  for(i=0; i<pgdata->nSelect; i++) {
    if(pgdata->selectInterval[i].from >= pgdata->cursor) {
      pgdata->selectInterval[i].from ++ ;
      pgdata->selectInterval[i].to ++ ;
    }
  }

  // shift the Brkpt 2000.2.18 & Cnnct 2000.5.26
  memmove( &(pgdata->bUserArrBrkpt[pgdata->cursor+2]),
	   &(pgdata->bUserArrBrkpt[pgdata->cursor+1]),
	   sizeof(int)*(pgdata->nPhoneSeq - pgdata->cursor)); 
  memmove( &(pgdata->bUserArrCnnct[pgdata->cursor+2]),
	   &(pgdata->bUserArrCnnct[pgdata->cursor+1]),
	   sizeof(int)*(pgdata->nPhoneSeq - pgdata->cursor)); 

  // add to phoneSeq
  memmove( &( pgdata->phoneSeq[pgdata->cursor + 1] ),
	   &( pgdata->phoneSeq[pgdata->cursor]) ,
	   sizeof(uint16)*(pgdata->nPhoneSeq - pgdata->cursor) ) ;
  pgdata->phoneSeq[pgdata->cursor] = phone ;
  pgdata->nPhoneSeq ++;
  pgdata->cursor ++;

  // add to chiSymbolBuf
  memmove( &( pgdata->chiSymbolBuf[pgdata->chiSymbolCursor + 1] ),
	   &( pgdata->chiSymbolBuf[pgdata->chiSymbolCursor]) ,
	   sizeof(wch_t)*
	   (pgdata->chiSymbolBufLen - pgdata->chiSymbolCursor) ) ;
  // "0" means Chinese word
  pgdata->chiSymbolBuf[ pgdata->chiSymbolCursor].wch = (wchar_t) 0 ;
  pgdata->chiSymbolBufLen ++ ;
  pgdata->chiSymbolCursor ++ ;

  return 0 ;
}

#ifdef DEBUG
void ShowChewingData(ChewingData *pgdata)
{
  int i ;

  fprintf(fp_g, "nPhoneSeq : %d\n"
	  "phoneSeq : ", pgdata->nPhoneSeq) ;
  for(i=0; i<pgdata->nPhoneSeq; i++)
    fprintf(fp_g, "%hu ", pgdata->phoneSeq[i]) ;
  fprintf(fp_g, "cursor : %d\n"
	  "nSelect : %d\n"
	  "selectStr       selectInterval\n", 
	  pgdata->cursor, pgdata->nSelect) ;
  for(i=0; i<pgdata->nSelect; i++) {
    fprintf(fp_g, "  %14s%4d%4d\n", pgdata->selectStr[i], 
	    pgdata->selectInterval[i].from,
	    pgdata->selectInterval[i].to) ;
  }
  /*
  fprintf(fp_g, "nPrefer : %d\n"
	  "preferInterval\n", pgdata->nPrefer) ;
  for(i=0; i<pgdata->nPrefer; i++)
    fprintf(fp_g, "  %4d%4d\n", pgdata->preferInterval[i].from, 
	    pgdata->preferInterval[i].to) ;
  */
  fprintf(fp_g, "bUserArrCnnct : ") ;
  for(i=0; i<=pgdata->nPhoneSeq; i++) 
    fprintf(fp_g, "%d ", pgdata->bUserArrCnnct[i]) ;
  fprintf(fp_g, "\n") ;

  fprintf(fp_g, "bUserArrBrkpt : ") ;
  for(i=0; i<=pgdata->nPhoneSeq; i++) 
    fprintf(fp_g, "%d ", pgdata->bUserArrBrkpt[i]) ;
  fprintf(fp_g, "\n") ;

  fprintf(fp_g, "bArrBrkpt : ") ;
  for(i=0; i<=pgdata->nPhoneSeq; i++) 
    fprintf(fp_g, "%d ", pgdata->bArrBrkpt[i]) ;
  fprintf(fp_g, "\n") ;

  fprintf(fp_g, "bChiSym : %d , bSelect : %d , bCaseChange : %d\n",
	  pgdata->bChiSym, pgdata->bSelect, pgdata->bCaseChange) ;
}

#endif

// kpchen 1999.8.17
int CallPhrasing(ChewingData *pgdata)
{
  // set "bSymbolArrBrkpt" && "bArrBrkpt" 
  int i, ch_count=0;

  memcpy(pgdata->bArrBrkpt, pgdata->bUserArrBrkpt, 
	 (MAX_PHONE_SEQ_LEN+1) * sizeof(int)) ;
  memset(pgdata->bSymbolArrBrkpt, 0, 
	 (MAX_PHONE_SEQ_LEN+1) *sizeof(int)) ;

  for(i=0; i<pgdata->chiSymbolBufLen; i++) {
    if(ChewingIsChiAt(i, pgdata))
      ch_count++ ;
    else {
      pgdata->bArrBrkpt[ch_count] = 1 ;
      pgdata->bSymbolArrBrkpt[ch_count] = 1 ;
    }
  }

  // kill select interval 
  for(i=0; i<pgdata->nPhoneSeq; i++) {
    if(pgdata->bArrBrkpt[i]) {
      ChewingKillSelectIntervalAcross(i, pgdata) ;
    }
  }

#ifdef DEBUG
  //ShowChewingData(pgdata) ;
  //fflush(fp_g) ;
#endif
  // then phrasing
  Phrasing( &(pgdata->phrOut), pgdata->phoneSeq, pgdata->nPhoneSeq,
	    pgdata->selectStr, pgdata->selectInterval, pgdata->nSelect, 
	    pgdata->bArrBrkpt, pgdata->bUserArrCnnct) ;

  // and then make prefer interval (lckung) 2000.7.5
  MakePreferInterval( pgdata);

  return 0 ;
}


static void Union(int set1,int set2, int parent[])
{
  if( set1 != set2)
    parent[ max(set1,set2) ] = min(set1,set2);
}

static int SameSet(int set1,int set2, int parent[])
{
  while( parent[set1] != 0 ) {
    set1 = parent[set1];
  }
  while( parent[set2] != 0 ) {
    set2 = parent[set2];
  }
  return set1 == set2;
}

// 2000/5/26 make prefer interval from phrOut->dispInterval
static void MakePreferInterval(ChewingData *pgdata)
{
  int i,j,set_no;
  int belong_set[MAX_PHONE_SEQ_LEN+1];
  int parent[MAX_PHONE_SEQ_LEN+1];

  memset( belong_set, 0 , sizeof(int)*(MAX_PHONE_SEQ_LEN+1) );
  memset( parent, 0, sizeof(int)*(MAX_PHONE_SEQ_LEN+1) );

  // for each interval
  for(i=0;i< pgdata->phrOut.nDispInterval; i++) {
    for(j= pgdata->phrOut.dispInterval[i].from;
	j <pgdata->phrOut.dispInterval[i].to;
	j++)
      belong_set[j] = i+1;
  }
  set_no = i;
  for(i=0;i< pgdata->nPhoneSeq; i++)
    if( belong_set[i] == 0 ) 
      belong_set[i] = set_no++;

  // for each connect point
  for(i=1;i< pgdata->nPhoneSeq; i++) {
    if( pgdata->bUserArrCnnct[i] ) {
      Union(belong_set[i-1], belong_set[i], parent);
    }
  }

  // generate new intervals
  pgdata->nPrefer = 0;
  i = 0;
  while( i< pgdata->nPhoneSeq ) {
    for(j=i+1; j< pgdata->nPhoneSeq; j++)
      if(!SameSet( belong_set[i], belong_set[j], parent))
	break;

    pgdata->preferInterval[ pgdata->nPrefer].from = i;
    pgdata->preferInterval[ pgdata->nPrefer].to =  j;
    pgdata->nPrefer++;
    i = j;
  }
}

// 88.9.1 for MakeOutput
static void ShiftInterval(ChewingOutput *pgo, ChewingData *pgdata)
{
  int i, arrPos[MAX_PHONE_SEQ_LEN], k=0, from, len ;

  for(i=0; i<pgdata->chiSymbolBufLen; i++) {
    if(ChewingIsChiAt(i, pgdata)) {
      arrPos[k++] = i ;
    }
  }
  arrPos[k] = i;

  pgo->nDispInterval = pgdata->nPrefer ;
  for(i=0; i<pgdata->nPrefer; i++) {
    from = pgdata->preferInterval[i].from ;
    len = pgdata->preferInterval[i].to - from ;
    pgo->dispInterval[i].from = arrPos[from] ;
    pgo->dispInterval[i].to = arrPos[from] + len ; 
  }
}

// kpchen 1999.8.17
int MakeOutput(ChewingOutput *pgo, ChewingData *pgdata)
{
  int chi_i, chiSymbol_i, i ;

  // fill zero to chiSymbolBuf first 
  // by lckung 99/9/12
  memset( pgo->chiSymbolBuf, 0, sizeof(wch_t)*MAX_PHONE_SEQ_LEN );

  // fill chiSymbolBuf
  for(chi_i=chiSymbol_i=0; 
      chiSymbol_i < pgdata->chiSymbolBufLen; 
      chiSymbol_i ++) {
    if(pgdata->chiSymbolBuf[chiSymbol_i].wch == (wchar_t)0 ) { 
      // is Chinese, then copy from the PhrasingOutput "phrOut"
      pgo->chiSymbolBuf[chiSymbol_i].wch = (wchar_t) 0 ;
      memcpy(pgo->chiSymbolBuf[chiSymbol_i].s, 
	     &( pgdata->phrOut.chiBuf[chi_i] ), 
	     2 ) ;
      chi_i+=2 ;
    }
    else {
      // is Symbol
      pgo->chiSymbolBuf[chiSymbol_i].wch = pgdata->chiSymbolBuf[chiSymbol_i].wch ;
    }
  }

  // fill other fields
  pgo->chiSymbolBufLen = pgdata->chiSymbolBufLen ;
  pgo->chiSymbolCursor = pgdata->chiSymbolCursor ;
  // fill zuinBuf
  for(i=0;i<ZUIN_SIZE;i++) { 
    if( pgdata->zuinData.pho_inx[i] != 0) {
      memcpy( pgo->zuinBuf[i].s, 
	      & ph_pho[i][ pgdata->zuinData.pho_inx[i]*2 ], 2) ; 
      pgo->zuinBuf[i].s[2] = '\0' ;
    }
    else
      pgo->zuinBuf[i].wch = (wchar_t) 0 ;
  } 
  
  ShiftInterval(pgo, pgdata) ; // 2000.7.5
  memcpy(pgo->dispBrkpt, pgdata->bUserArrBrkpt, 
	 sizeof(pgo->dispBrkpt[0]) * (MAX_PHONE_SEQ_LEN+1) ) ;
  pgo->pci = &(pgdata->choiceInfo) ;
  pgo->bChiSym = pgdata->bChiSym ;
  memcpy(pgo->selKey, pgdata->config.selKey, sizeof(pgdata->config.selKey)) ;
  pgo->bShowMsg = 0 ; // kpchen 2000.5.12
  return 0 ;
}

// 1999.10.2
int MakeOutputWithRtn(ChewingOutput *pgo, ChewingData *pgdata, int keystrokeRtn)
{
  pgo->keystrokeRtn = keystrokeRtn ;
  return MakeOutput(pgo, pgdata) ;
}

void MakeOutputAddMsgAndCleanInterval(ChewingOutput *pgo, ChewingData *pgdata)
{
  pgo->bShowMsg = 1 ;
  memcpy(pgo->showMsg, pgdata->showMsg, sizeof(wch_t)*(pgdata->showMsgLen)) ;
  pgo->showMsgLen = pgdata->showMsgLen ;
  pgo->nDispInterval = 0 ;
}

int AddSelect(ChewingData *pgdata, int sel_i)
{
  int length, nSelect ;

  // save the typing time
  length = pgdata->availInfo.avail[pgdata->availInfo.currentAvail].len ;
  nSelect = pgdata->nSelect ;

  // change "selectStr" , "selectInterval" , and "nSelect" of ChewingData
  memcpy(pgdata->selectStr[nSelect], 
	 pgdata->choiceInfo.totalChoiceStr[sel_i],
	 length*2) ;
  pgdata->selectInterval[nSelect].from = pgdata->cursor ;
  pgdata->selectInterval[nSelect].to = pgdata->cursor + length ;
  pgdata->nSelect ++ ;
  return 0 ;
}

int CountSelKeyNum(int key, ChewingData *pgdata) 
// return value starts from 0.  If less than zero : error key
{
  int i ;

  for(i=0; i<MAX_SELKEY; i++)
    if(pgdata->config.selKey[i] == key)
      return i ;
  return -1 ;
}

int ChewingIsChiAt(int chiSymbolCursor, ChewingData *pgdata)
{
  // wch == 0 means Chinese
  return ( chiSymbolCursor < pgdata->chiSymbolBufLen ) &&
    (pgdata->chiSymbolBuf[chiSymbolCursor].wch == (wchar_t)0 ) ;
}

// kpchen 1999.8.17
void RemoveSelectElement(int i, ChewingData *pgdata)
{
  if(--pgdata->nSelect == i) return ;
  pgdata->selectInterval[i] = pgdata->selectInterval[pgdata->nSelect] ;
  strcpy(pgdata->selectStr[i], pgdata->selectStr[pgdata->nSelect]) ;
}

int ChewingKillSelectIntervalAcross(int cursor, ChewingData *pgdata)
{
  int i ;
  for(i=0; i<pgdata->nSelect; i++) {
    if(pgdata->selectInterval[i].from < cursor && 
       pgdata->selectInterval[i].to > cursor) {
      RemoveSelectElement(i, pgdata) ;
      i-- ;
    }
  }
  return 0 ;
}

int KillCharInSelectIntervalAndBrkpt(ChewingData *pgdata, int cursorToKill)
{
  int i ; 
 
  for(i=0; i<pgdata->nSelect; i++) { 
    if( pgdata->selectInterval[i].from<=cursorToKill && 
	pgdata->selectInterval[i].to > cursorToKill ) { 
      RemoveSelectElement(i, pgdata) ; 
      i--;      /* the last one was swap to i, we need to recheck i */ 
    } 
    else if(pgdata->selectInterval[i].from > cursorToKill) { 
      pgdata->selectInterval[i].from -- ; 
      pgdata->selectInterval[i].to -- ; 
    } 
  } 
  memmove( &(pgdata->bUserArrBrkpt[cursorToKill]),
	   &(pgdata->bUserArrBrkpt[cursorToKill+1]),
	   sizeof(int)*(pgdata->nPhoneSeq - cursorToKill)); // kpchen 2000/2/2
  memmove( &(pgdata->bUserArrCnnct[cursorToKill]),
	   &(pgdata->bUserArrCnnct[cursorToKill+1]),
	   sizeof(int)*(pgdata->nPhoneSeq - cursorToKill)); // kpchen 2000/2/2

  return 0 ;
}

int ChewingKillChar(ChewingData *pgdata, 
		  int cursorToKill, 
		  int chiSymbolCursorToKill, 
		  int minus)
{
  if( ChewingIsChiAt(chiSymbolCursorToKill, pgdata) ) { // lckung 7/13/1999
    KillCharInSelectIntervalAndBrkpt(pgdata, cursorToKill) ; // kpchen 6/13/1999 
    memmove(&(pgdata->phoneSeq[cursorToKill]), 
	    &(pgdata->phoneSeq[cursorToKill+1]),
	    (pgdata->nPhoneSeq - cursorToKill - 1)*sizeof(uint16)) ;
    pgdata->nPhoneSeq -- ;
    pgdata->cursor -= minus ;
  }
  memmove( & pgdata->chiSymbolBuf[chiSymbolCursorToKill]
	   ,& pgdata->chiSymbolBuf[chiSymbolCursorToKill + 1]
	   , (pgdata->chiSymbolBufLen - chiSymbolCursorToKill)
	   *sizeof(wch_t));
  pgdata->chiSymbolBufLen -- ;
  pgdata->chiSymbolCursor -= minus ;
  return 0 ;
}

int IsPreferIntervalConnted( int cursor, ChewingData *pgdata)
{
  int i;

  for(i=0; i<pgdata->nPrefer; i++) {
    if( pgdata->preferInterval[i].from < cursor &&
	pgdata->preferInterval[i].to > cursor ) 
      return 1;
  }
  return 0;
}





