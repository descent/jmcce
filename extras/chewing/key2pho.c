
/*
	$Id: key2pho.c,v 1.1.1.1 2002/05/03 04:01:08 kids Exp $
	map zuins to uint16 type according to different kb_type
*/
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "global.h"

const static int shift[] = { 9,7,3,0 };
const char *ph_pho[]={					/* number of bits */
  "  £t£u£v£w£x£y£z£{£|£}£~£¡£¢£££¤£¥£¦£§£¨£©£ª",	/* 5 */
  "  £¸£¹£º",						/* 2 */
  "  £«£¬£­£®£¯£°£±£²£³£´£µ£¶£·",			/* 4 */
  "  £»£½£¾£¿"						/* 3 */
};
const char *ph_str=
"£t£u£v£w£x£y£z£{£|£}£~£¡£¢£££¤£¥£¦£§£¨£©£ª£¸£¹£º£«£¬£­£®£¯£°£±£²£³£´£µ£¶£·£»£½£¾£¿" ;

char *key_str[MAX_KBTYPE] = {
  "1qaz2wsxedcrfv5tgbyhnujm8ik,9ol.0p;/-7634",		/* standard kb */
  "bpmfdtnlgkhjvcjvcrzasexuyhgeiawomnkllsdfj",		/* hsu */
  "1234567890-qwertyuiopasdfghjkl;zxcvbn/m,.",		/* IBM */
  "2wsx3edcrfvtgb6yhnujm8ik,9ol.0p;/-['=1qaz",		/* Gin-yieh */
  "bpmfdtnlvkhg7c,./j;'sexuaorwiqzy890-=1234",		/* ET  */
  "bpmfdtnlvkhgvcgycjqwsexuaorwiqzpmntlhdfjk"		/* ET26 */
} ;

// 1999.12.6 changed by kpchen
int Key2PhoneInx(int key, int type, int kbtype, int searchTimes)
{
  char keyStr[5], bgStr[10], *p ;

  keyStr[0] = key ;
  keyStr[1] = '\0' ;
  Key2Pho(bgStr,keyStr,kbtype,searchTimes);
  p = strstr(ph_pho[type], bgStr) ;
  if(!p) return 0 ;
  return (p - ph_pho[type]) / 2 ;
}

uint16 PhoneInx2Uint(const int ph_inx[])
{
  int i;
  uint16 result = 0;

  for(i=0;i<4;i++)
    result |= ph_inx[i] << shift[i];
  return result;
}

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

  for(i=0,j=0;i<len && j<4;j++) {
    temp[0] = phone[i];
    temp[1] = phone[i+1];
    temp[2] = 0;
    pc = strstr(ph_pho[j],temp);
    if( pc) {
      result |= ((pc-ph_pho[j])>>1) << shift[j];
      i+=2;
    }
  }
  assert(i==len);
  return result;
}

int Key2Pho(char *pho,const char *inputkey, int kbtype, int searchTimes)
{
  int len = strlen(inputkey), i, s ;
  char *pTarget ;

  pho[0] = '\0' ; // 1999.12.6
  for(i=0; i<len; i++) {
    char *findptr = NULL;
    int index ;

    // 1999.12.6
    for(s=0,pTarget=key_str[kbtype]; s<searchTimes; s++,pTarget=findptr+1) {
      findptr=strchr(pTarget, inputkey[i]);
      if(!findptr) {
	/*puts("input error!") ;*/
	return 0 ;
      }
    }
    index = findptr - key_str[kbtype] ;
    pho[i*2] = ph_str[index*2] ;
    pho[i*2+1] = ph_str[index*2+1] ;
  }
  pho[len*2] = '\0' ;
  return 1 ;
}

/*
int main()
{
  char str[500], *ptr ;

  fgets(str, 500, stdin) ;
  for(ptr=strtok(str, " \t\n"); ptr; ptr=strtok(NULL, " \t\n")) {
    int len = strlen(ptr), i ;

    for(i=0; i<len; i++) {
      char *findptr ;
      int index ;
			
      if(!(findptr=strchr(key, ptr[i]))) {
	puts("input error!") ;
	return 1 ;
      }
      index = findptr - key ;
      printf("%c%c", ph_pho[index*2], ph_pho[index*2+1]) ;
    }
    printf(" ") ;
  }
  puts("") ;
  return 0 ;
}


*/
