/* 
   $Id: char.c,v 1.1.1.1 2002/05/03 04:01:07 kids Exp $ 
   word data file
 */
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

#include "char.h"

static uint16 arrPhone[PHONE_NUM+1];
static int begin[PHONE_NUM+1] ;
static FILE *dictfile ;
static int end_pos ;

static char* fgettab(char *buf,int maxlen,FILE *fp)
{
  int i;
	
  for(i=0; i < maxlen;i++) {
    buf[i] = (char)fgetc(fp);
    if(feof(fp)) break ;
    if(buf[i]=='\t')
      break ;
  }
  if(feof(fp))
    return 0;
  buf[i] = '\0';
  return buf;
}

int CompUint16(const uint16 *pa,const uint16 *pb)
{
  return (*pa) - (*pb);
}

int InitChar(const char *prefix)
{
  FILE *indexfile ;
  int i ;
  char filename[100] ;
  
  // make attention for the "/"
  strcpy(filename, prefix) ;
  strcat(filename, "/");
  strcat(filename, CHAR_FILE) ;
  dictfile = fopen(filename, "r") ;	

  strcpy(filename, prefix) ;
  strcat(filename, "/");
  strcat(filename, CHAR_INDEX_FILE) ;
  indexfile = fopen(filename, "r") ;
  assert(dictfile && indexfile) ;
  for(i=0; i<=PHONE_NUM; i++)
    fscanf(indexfile, "%hu %d",&arrPhone[i], &begin[i]) ;
  fclose(indexfile) ;	
  return 1 ;
}

void Str2Word(Word *wrd_ptr)
{
  char buf[1000] ;
  uint16 sh ;
	
  fgettab(buf, 1000, dictfile) ;
  sscanf(buf, "%hu %s", &sh, wrd_ptr->word) ;
}

int GetCharFirst(Word *wrd_ptr, uint16 phoneid) 
{
  uint16 *pinx ;

  pinx = bsearch(&phoneid,arrPhone,PHONE_NUM,sizeof(uint16),(CompFuncType)CompUint16);
  if(!pinx) return 0 ;

  fseek(dictfile, begin[pinx-arrPhone], SEEK_SET) ;
  end_pos = begin[pinx-arrPhone+1] ;
  Str2Word(wrd_ptr) ;
  return 1 ;
}

int GetCharNext(Word *wrd_ptr) 
{
  if(ftell(dictfile) >= end_pos) return 0 ;
  Str2Word(wrd_ptr) ;
  return 1 ;
}

/*
  // for testing the above functions 
  void main()
  {
  Word phr ;
  uint16 id ;

  printf("Phone id : ") ;
  scanf("%hu", &id) ;

  InitChar() ;
  GetCharFirst(&phr, id) ;
  printf("%s %d\n", phr.word, phr.freq) ;
  while(GetCharNext(&phr))
  printf("%s %d\n", phr.word, phr.freq) ;
  }
*/
