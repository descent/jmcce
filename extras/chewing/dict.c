#include "dict.h"
#include <stdio.h>
#include <assert.h>
#include <string.h>

static int begin[PHONE_PHRASE_NUM+1] ;
static FILE *dictfile ;
static int end_pos ;

char* fgettab(char *buf,int maxlen,FILE *fp)
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

int InitDict(const char *prefix)
{
  FILE *indexfile ;
  int i ;
  char filename[100] ;

  strcpy(filename, prefix) ;
  strcat(filename, "/");
  strcat(filename, DICT_FILE ) ;
  dictfile = fopen(filename, "r") ;	

  strcpy(filename, prefix) ;
  strcat(filename, "/");
  strcat(filename, PH_INDEX_FILE ) ;
  indexfile = fopen(filename, "r") ;
  assert(dictfile && indexfile) ;
  //for(i=0; i<=PHONE_PHRASE_NUM; i++)
  i=0;
  while( !feof(indexfile) )
    fscanf(indexfile, "%d", &begin[i++]) ;
  fclose(indexfile) ;	
  return 1 ;
}

void Str2Phrase(Phrase *phr_ptr)
{
  char buf[1000] ;

  fgettab(buf, 1000, dictfile) ;
  sscanf(buf, "%s %d", phr_ptr->phrase, &(phr_ptr->freq)) ;
}

int GetPhraseFirst(Phrase *phr_ptr, int phone_phr_id) 
{
  assert(0<=phone_phr_id && phone_phr_id<PHONE_PHRASE_NUM) ;

  fseek(dictfile, begin[phone_phr_id], SEEK_SET) ;
  end_pos = begin[phone_phr_id+1] ;
  Str2Phrase(phr_ptr) ;
  return 1 ;
}

int GetPhraseNext (Phrase *phr_ptr) 
{
  if(ftell(dictfile) >= end_pos) return 0 ;
  Str2Phrase(phr_ptr) ;
  return 1 ;
}

/*
  // for testing the above functions 
  void main()
  {
  Phrase phr ;
  int id ;

  printf("Phone Phrase id : ") ;
  scanf("%d", &id) ;

  InitDict() ;
  GetFirst(&phr, id) ;
  printf("%s %d\n", phr.phrase, phr.freq) ;
  while(GetNext(&phr))
  printf("%s %d\n", phr.phrase, phr.freq) ;
  }
*/
