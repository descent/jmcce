// kpchen 2000.3.11      $Id: hash.c,v 1.1.1.1 2002/05/03 04:01:07 kids Exp $

#include "hash.h"
#include "global.h"
#include <assert.h>
#include <string.h>
#include <sys/stat.h>


#ifdef DEBUG
extern FILE *fp_g;
#endif
extern int lifetime ;
static HASH_ITEM *hashtable[HASH_TABLE_SIZE] ;
static char formatstring[30] ;
static char hashfilename[200] ;
//2000.3.17
int AlcUserPhraseSeq(UserPhraseData *pData, int len)
{
  pData->phoneSeq = ALC(uint16, len+1) ;
  pData->wordSeq = ALC(char, len*2 + 1) ;
  return pData->phoneSeq && pData->wordSeq ;
}

static int PhoneSeqTheSame(const uint16 p1[],const uint16 p2[])
{
  int i ;

  for(i=0; p1[i]!=0 && p2[i]!=0; i++) {
    if(p1[i] != p2[i]) return 0 ;
  }
  if(p1[i] != p2[i]) return 0 ;
  return 1 ;
}

static unsigned int HashFunc(const uint16 phoneSeq[])
{
  int i, value=0 ;

  for(i=0; phoneSeq[i] != 0; i++)
    value ^= phoneSeq[i] ;
  return ( value & (HASH_TABLE_SIZE - 1));
}

HASH_ITEM *HashFindPhonePhrase(const uint16 phoneSeq[], HASH_ITEM *pItemLast)
{
  HASH_ITEM *pNow ;

  if( !pItemLast)
    pNow=hashtable[HashFunc(phoneSeq)] ;
  else
    pNow = pItemLast->next ;

  for(; pNow; pNow=pNow->next) 
    if(PhoneSeqTheSame(pNow->data.phoneSeq, phoneSeq))
      return pNow ;
  return NULL ;
}

HASH_ITEM *HashFindEntry(const uint16 phoneSeq[], const char wordSeq[])
{
  HASH_ITEM *pItem ;
  int hashvalue ;

  hashvalue = HashFunc(phoneSeq) ;

  for(pItem=hashtable[hashvalue]; pItem ; pItem=pItem->next) {
    if( !strcmp(pItem->data.wordSeq, wordSeq) && 
	PhoneSeqTheSame(pItem->data.phoneSeq, phoneSeq)) {
      return pItem ;
    }
  }
  return NULL ;
}

HASH_ITEM *HashInsert(UserPhraseData *pData)
{
  int hashvalue, len ;
  HASH_ITEM *pItem ;


  pItem = HashFindEntry(pData->phoneSeq, pData->wordSeq) ;
  if( pItem != NULL) return pItem ;

  pItem = ALC(HASH_ITEM, 1) ;
  if( ! pItem ) return NULL ;  // Error occurs
  len = strlen(pData->wordSeq) / 2 ;
  if( ! AlcUserPhraseSeq( &(pItem->data), len ) ) return NULL ; // Error occurs

  hashvalue = HashFunc(pData->phoneSeq) ;
  // set the new element
  pItem->next = hashtable[hashvalue] ;
  
  memcpy( &(pItem->data), pData, sizeof(pItem->data)) ;
  pItem->item_index = -1 ;

  // set link to the new element
  hashtable[hashvalue] = pItem ;

  return pItem ;
}

// 2000.3.17
static void HashItem2String(char *str, HASH_ITEM *pItem)
{
  int i, len ;
  char buf[FIELD_SIZE] ;

  sprintf(str, "%s ", pItem->data.wordSeq) ;
  len = strlen(pItem->data.wordSeq) / 2 ;
  for(i=0; i<len; i++) {
    sprintf(buf, "%hu ", pItem->data.phoneSeq[i]) ;
    strcat(str, buf) ;
  }
  sprintf(buf, "%d %d %d %d", pItem->data.userfreq, pItem->data.recentTime
	  	, pItem->data.maxfreq, pItem->data.origfreq) ;
  strcat(str, buf) ;
}

// 2000.3.17
void HashModify(HASH_ITEM *pItem)
{
  FILE *outfile ;
  char str[FIELD_SIZE+1] ;

  outfile = fopen(hashfilename, "r+") ;

  // update "lifetime"
  fseek(outfile, 0, SEEK_SET) ;
  sprintf(str, "%d", lifetime) ;
#ifdef DEBUG
  fprintf(fp_g, "HashModify-1: formatstring='%s',printing '%s'\n", formatstring,str);
  fflush(fp_g);
#endif
  fprintf(outfile, formatstring, str) ;
 

  // update record
  if( pItem->item_index < 0) {
    fseek(outfile, 0, SEEK_END) ;
    pItem->item_index = ftell(outfile) / FIELD_SIZE ;
  }
  else {
    fseek(outfile, pItem->item_index * FIELD_SIZE, SEEK_SET) ;
  }
  HashItem2String(str, pItem) ;
#ifdef DEBUG
  fprintf(fp_g, "HashModify-2: formatstring='%s',printing '%s'\n", formatstring,str);
  fflush(fp_g);
#endif
  fprintf(outfile, formatstring, str) ;
  fclose(outfile) ;
}

int ReadHashItem(FILE *infile, HASH_ITEM *pItem, int item_index)
{
  int len, i, word_len ;
  char wordbuf[64] ;

  // read wordSeq
  if(fscanf(infile, "%s", wordbuf) != 1) return 0 ;
  word_len = strlen(wordbuf) ;
  pItem->data.wordSeq = ALC(char,word_len+1) ;
  strcpy(pItem->data.wordSeq, wordbuf) ;

  // read phoneSeq
  len = word_len / 2 ;
  pItem->data.phoneSeq = ALC(uint16, len+1) ;
  for(i=0; i<len; i++)
    if(fscanf(infile, "%hu", &(pItem->data.phoneSeq[i])) != 1) return 0 ;
  pItem->data.phoneSeq[len] = 0 ;

  // read userfreq & recentTime
  if(fscanf(infile, "%d %d %d %d", 
	    &(pItem->data.userfreq), &(pItem->data.recentTime)
		,&(pItem->data.maxfreq), &(pItem->data.origfreq) ) != 4 ) 
    return 0;

  // set item_index
  pItem->item_index = item_index ;

  return 1 ;
}

int ReadHash(void)
{
  FILE *infile ;
  HASH_ITEM item, *pItem ;
  char *ptr, homedir[256] ;
  int item_index, hashvalue ;

  sprintf(formatstring, "%%-%ds", FIELD_SIZE) ; 
  ptr = getenv("HOME") ;
  assert(ptr) ;
  strcpy(homedir, ptr) ;
  strcat(homedir, "/.xcin") ;
  mkdir(homedir, S_IRWXU) ;
  sprintf(hashfilename, "%s/%s", homedir, HASH_FILE) ;
  infile = fopen(hashfilename, "r") ;
  if( ! infile) {
    FILE *outfile ;
    outfile = fopen(hashfilename, "w") ;
    if( ! outfile) return 0 ;

    fprintf(outfile, formatstring, "0") ;
    lifetime = 0 ;
    fclose(outfile) ;
  }
  else {
    fscanf(infile, "%d", &lifetime) ;
    item_index = 0 ;
    while(ReadHashItem(infile, &item, ++item_index)) {
      // 2000.3.31
      hashvalue = HashFunc(item.data.phoneSeq) ;
      pItem = ALC(HASH_ITEM,1) ;
      memcpy(pItem, &item, sizeof(HASH_ITEM)) ;
      pItem->next = hashtable[hashvalue] ;
      hashtable[hashvalue] = pItem ;
    }
    fclose(infile) ;
  }
  return 1 ;
}
