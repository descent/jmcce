/*
 $Id: global.h,v 1.1.1.1 2002/05/03 04:01:08 kids Exp $
*/
#ifndef _GLOBAL_H
#define _GLOBAL_H
//#define DEBUG
#include<stdio.h>
#include<stdlib.h>

#define PHONE_TREE_FILE		"fonetree.dat"
#define DICT_FILE		"dict.dat"
#define PH_INDEX_FILE		"ph_index.dat"
#define CHAR_FILE		"us_freq.dat"
#define CHAR_INDEX_FILE		"ch_index.dat"

#ifdef DEBUG
extern FILE *fp_g;
#endif

#define IS_USER_PHRASE 1
#define IS_DICT_PHRASE 0
#define CHINESE_MODE 1
#define SYMBOL_MODE 0

#define MAX_KBTYPE 10
#define MAX_SELKEY 10
#define TREE_SIZE (112500)
#define WCH_SIZE 4
#define ZUIN_SIZE 4
#define MAX_PHRASE_LEN 10
#define MAX_PHONE_SEQ_LEN 50
#define MAX_INTERVAL ((MAX_PHONE_SEQ_LEN+1)*MAX_PHONE_SEQ_LEN/2)
#define MAX_CHOICE (150)
#define MAX_CHOICE_BUF (50)                   /* max length of the choise buffer */
#ifndef max
#define max(a,b) ((a)>(b)?(a):(b))
#endif
#ifndef min
#define min(a,b) ((a)<(b)?(a):(b))
#endif
#define ALC(type,size)  ((type *) malloc(sizeof(type)*size) )

typedef unsigned short uint16;

typedef struct {
  char *inp_cname ;
  char *inp_ename ;
  int kb_type ;
} ChewingConf ;

typedef union {
    unsigned char s[WCH_SIZE];
    wchar_t wch;
} wch_t;

typedef struct {
  uint16 phone_id ;
  int phrase_id ;
  int child_begin, child_end ;
} TreeType ;

typedef struct {
  int from, to ;
} IntervalType ;

typedef struct {
  char chiBuf[MAX_PHONE_SEQ_LEN*2+1] ;
  IntervalType dispInterval[MAX_INTERVAL] ;
  int nDispInterval ;
} PhrasingOutput ;

typedef struct {
  int kbtype;
  int pho_inx[4];
  uint16 phone; 	
} ZuinData;

typedef struct {
  struct {
    int len ;
    int id ;
  } avail[MAX_PHRASE_LEN] ;  // all kinds of lengths of available phrases.
  int nAvail ;
  int currentAvail ;
} AvailInfo ;

typedef struct {
  int nPage ;
  int pageNo ;
  int nChoicePerPage ;
  char totalChoiceStr[MAX_CHOICE][MAX_PHRASE_LEN*2 + 1] ;
  int nTotalChoice ;
  int oldCursor, oldChiSymbolCursor ;
} ChoiceInfo ;

typedef struct {
  int selectAreaLen ;
  int maxChiSymbolLen ;
  int selKey[MAX_SELKEY] ;
} ConfigData ;

typedef struct {
  AvailInfo availInfo ;
  ChoiceInfo choiceInfo ;
  PhrasingOutput phrOut ;
  ZuinData zuinData ;
  ConfigData config ;
  wch_t chiSymbolBuf[MAX_PHONE_SEQ_LEN] ;  // content==0 means Chinese code
  int chiSymbolCursor ;
  int chiSymbolBufLen ;
  wch_t showMsg[MAX_PHONE_SEQ_LEN] ;
  int showMsgLen ;

  uint16 phoneSeq[MAX_PHONE_SEQ_LEN] ;
  int nPhoneSeq ;
  int cursor ;
  char selectStr[MAX_PHONE_SEQ_LEN][MAX_PHONE_SEQ_LEN*2+1] ;
  IntervalType selectInterval[MAX_PHONE_SEQ_LEN] ;
  int nSelect ;
  IntervalType preferInterval[MAX_INTERVAL] ; // add connect points
  int nPrefer ;
  int bUserArrCnnct[MAX_PHONE_SEQ_LEN+1] ;
  int bUserArrBrkpt[MAX_PHONE_SEQ_LEN+1] ;   
  int bArrBrkpt[MAX_PHONE_SEQ_LEN+1] ;
  int bSymbolArrBrkpt[MAX_PHONE_SEQ_LEN+1] ;
  // "bArrBrkpt[10]=True" means "it breaks between 9 and 10"
  int bChiSym, bSelect, bCaseChange, bFirstKey ;

} ChewingData ;

typedef struct {
  wch_t chiSymbolBuf[MAX_PHONE_SEQ_LEN] ;
  int chiSymbolBufLen ;
  int chiSymbolCursor ;
  wch_t zuinBuf[ZUIN_SIZE] ;
  IntervalType dispInterval[MAX_INTERVAL] ; // from prefer ,considering symbol
  int nDispInterval ;
  int dispBrkpt[MAX_PHONE_SEQ_LEN+1] ;
  wch_t commitStr[MAX_PHONE_SEQ_LEN] ;
  int nCommitStr ;
  ChoiceInfo* pci;
  int bChiSym ;
  int selKey[MAX_SELKEY] ;
  int keystrokeRtn ;
  int bShowMsg ;
  wch_t showMsg[MAX_PHONE_SEQ_LEN] ;
  int showMsgLen ;
} ChewingOutput ;


typedef int (*CompFuncType)(const void *, const void *) ;

typedef struct {
  char phrase[MAX_PHRASE_LEN*2+1] ;
  int freq ;
} Phrase ;

typedef struct {
  int from , to, pho_id, source ;
  Phrase *p_phr;
} PhraseIntervalType ;

/* function prototype */
// xcin_chewing.c
void Live() ;

// tree.c
int Phrasing(PhrasingOutput *ppo, uint16 phoneSeq[], int nPhoneSeq, 
	     char selectStr[][MAX_PHONE_SEQ_LEN*2+1], 
	     IntervalType selectInterval[], int nSelect, 
	     int bArrBrkpt[], int bUserArrCnnct[]) ;
int IsContain(IntervalType, IntervalType) ;
int IsIntersect(IntervalType, IntervalType) ;
void ReadTree(const char *);
int TreeFindPhrase(int begin, int end, const uint16 *phoneSeq) ;

// key2pho.c 
uint16 PhoneBg2Uint(const char *phone);
uint16 PhoneInx2Uint(const int ph_inx[]);
int Key2Pho(char *pho,const char *inputkey, int kbtype, int searchTimes);
int Key2PhoneInx(int key, int type, int kbtype, int searchTimes);

// choice module
int ChoiceFirstAvail(ChewingData *) ;
int ChoiceNextAvail(ChewingData *) ;
int ChoicePrevAvail(ChewingData *) ;
int ChoiceSelect(ChewingData *, int selectNo) ;
int ChoiceEndChoice(ChewingData *);

#endif













