/*
  Read dictionary format :
      phrase   frequency   zuin1 zuin2 zuin3 ... \n

  Output format : ( Sorted by zuin's uint16 number )
      phrase   frequency   zuin1 zuin2 zuin3 ... \t

 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXLEN 98
#define MAXZUIN 9
#define MAX_FILE_NAME (256)

typedef unsigned short uint16 ;

typedef int CompFuncType (const void *, const void *) ;

typedef struct {
  char str[MAXLEN] ;
  int freq ;
  uint16 num[MAXZUIN] ;
} RECORD ;

RECORD data[120000] ;
int nData ;

const char user_msg[] = "\
sort_dic -- read chinese phrase input and generate data file for chewing\n\
usage: \n\
\tsort_dic <tsi file name> or \n\
\tsort_dic (default name is tsi.src) \n\
This program creates three new files. \n\
1.dict.dat \t-- main dictionary file \n\
2.ph_index.dat \t-- index file of phrase \n\
3.phoneid.out \t-- intermediate file for make_tree \n";

const char *ph_pho[]={
"  £t£u£v£w£x£y£z£{£|£}£~£¡£¢£££¤£¥£¦£§£¨£©£ª",		/* 5 */
"  £¸£¹£º",											/* 2 */
"  £«£¬£­£®£¯£°£±£²£³£´£µ£¶£·",						/* 4 */			
"  £»£½£¾£¿"										/* 3 */
};

const int shift[] = { 9,7,3,0 }; 

/*
	Translate a single phone into a single uint
	assume phone is a big5 null-terminated string with no spaces
*/
uint16 PhoneBg2Uint(const char *phone)
{
	int i,j,len=strlen(phone);
	uint16 result = 0;
	char temp[3];
	char *pc;
	
	for(i=0,j=0;i<len;j++) {
		temp[0] = phone[i];
		temp[1] = phone[i+1];
		temp[2] = 0;
		pc = strstr(ph_pho[j],temp);
		if( pc) {
			result |= ((pc-ph_pho[j])>>1) << shift[j];
			i+=2;
		}
	}
	return result;
}

void DataSetNum(int index)
{
  char buf[MAXLEN], *p ;
  int i=0 ;

  strcpy(buf, data[index].str) ;
  strtok(buf, " \n\t") ;
  data[index].freq = atoi(strtok(NULL, " \n\t")) ;
  for(p = strtok(NULL, " \n\t"); p; p = strtok(NULL, " \n\t")) 
    data[index].num[i++] = PhoneBg2Uint(p) ;

}

void SetNewline2Zero(int index)
{
  char *p ;

  p = strchr(data[index].str, '\n') ;
  if(p) *p = '\0' ;
}

void DataStripSpace(int index)
{
  int i, k=0 ;
  char old[MAXLEN], last = '\0' ;

  strcpy(old, data[index].str) ;
  for(i=0; old[i]; i++) {
    if(old[i] == ' ' && last == ' ') continue ;
    data[index].str[k++] = old[i] ;
    last = old[i] ;
  }
  data[index].str[k] = '\0' ;
}

int CompRecord(const RECORD *a, const RECORD *b)
{
  int i, cmp ;

  for(i=0; i<MAXZUIN; i++) {
    cmp = a->num[i] - b->num[i] ;
    if(cmp) return cmp ;
  }
  return b->freq - a->freq ;
}

int CompUint(int a, int b)
{
  int i;

  for(i=0; i<MAXZUIN; i++) {
    if(data[a].num[i] != data[b].num[i])
      return 1 ;
  }
  return 0 ;
}

int main(int argc,char *argv[])
{
  FILE *infile ;
  FILE *dictfile, *treedata, *ph_index ;
  char in_file[MAX_FILE_NAME] = "tsi.src";
  int i, k ;

  if( argc <= 1 ) 
    printf(user_msg);
  else 
    strcpy( in_file, argv[1]);

  infile = fopen(in_file, "r") ;

  if( !infile) {
    fprintf(stderr,"Error opening %s for reading!\n",in_file);
    exit(-1);
  }

  dictfile = fopen("dict.dat", "w") ;
  treedata = fopen("phoneid.dic", "w") ;
  ph_index = fopen("ph_index.dat", "w") ;

  if( !dictfile || !treedata || !ph_index) {
    fprintf(stderr,"Error opening output file!\n");
    exit(-1);
  }

  while(fgets(data[nData].str, MAXLEN, infile)) {
    DataStripSpace(nData) ;
    DataSetNum(nData) ;
    SetNewline2Zero(nData) ;
    nData++ ;
  }
  qsort(data, nData, sizeof(RECORD), (CompFuncType *)CompRecord) ;

  for(i=0; i<nData-1; i++) {
    if(i==0 || CompUint(i, i-1) != 0) 
      fprintf(ph_index, "%d\n", ftell(dictfile)) ;
    fprintf(dictfile, "%s\t", data[i].str) ;
  }
  fprintf(ph_index, "%d\n", ftell(dictfile)) ;
  fprintf(dictfile, "%s", data[nData-1].str) ;
  fprintf(ph_index, "%d\n", ftell(dictfile)) ;

  for(i=0; i<nData; i++) {
    if(i>0 && CompUint(i, i-1)==0) continue ;

    for(k=0; data[i].num[k]; k++)
      fprintf(treedata, "%hu ", data[i].num[k]) ;
    fprintf(treedata, "0\n") ;

  } 
  fclose(ph_index) ;
  fclose(dictfile) ;
  fclose(treedata) ;
  return 0 ;
}
