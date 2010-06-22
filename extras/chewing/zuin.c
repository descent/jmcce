// $Id: zuin.c,v 1.1.1.1 2002/05/03 04:01:08 kids Exp $
// Zuinx.c : control keyboard mapping
// include the definition of ZuinData structure
#include<ctype.h>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include"global.h"
#include"zuin.h"
#include"char.h"

// process a key input
// return value:
//		ZUIN_ABSORB
//		ZUIN_COMMIT
//		ZUIN_KEY_ERROR
// 		ZUIN_ERROR


int IsHsuPhoEndKey(int pho_inx[], int key)
{
  switch(key) {
  case 's':
  case 'd':
  case 'f':
  case 'j':
  case ' ':
    return (pho_inx[0] || pho_inx[1] || pho_inx[2]) ;
  default:
    return 0 ;
  }
}

/* copy the idea from HSU keyboard */
int IsET26PhoEndKey(int pho_inx[], int key)
{
  switch(key) {
  case 'd':
  case 'f':
  case 'j':
  case 'k':
  case ' ':
    return (pho_inx[0] || pho_inx[1] || pho_inx[2]) ;
  default:
    return 0 ;
  }
}

int IsDefPhoEndKey(int key,int kbtype) // under construction
{
  if( Key2PhoneInx(key, 3, kbtype, 1)  )
    return 1;
  else {
    if( key == ' ' )
      return 1;
    else
      return 0;
  }
}

int EndKeyProcess(ZuinData *pZuin, int key, int searchTimes)
{
  uint16 u16Pho ;
  Word tempword ;

  if(pZuin->pho_inx[0]==0 && pZuin->pho_inx[1]==0 && pZuin->pho_inx[2]==0)
    return ZUIN_KEY_ERROR ;

  pZuin->pho_inx[3] = Key2PhoneInx(key, 3, pZuin->kbtype, searchTimes) ;
  u16Pho = PhoneInx2Uint(pZuin->pho_inx) ;
  if( GetCharFirst(&tempword, u16Pho)==0 ) {
    ZuinRemoveAll(pZuin);
    return ZUIN_NO_WORD;
  }
  else {
    pZuin->phone = u16Pho;
    memset(pZuin->pho_inx, 0, sizeof(pZuin->pho_inx)) ;
    return ZUIN_COMMIT;
  }
}

int DefPhoInput(ZuinData *pZuin,int key)
{
  int type=0,inx=0;

  if( IsDefPhoEndKey(key, pZuin->kbtype) ) {
    return EndKeyProcess(pZuin, key, 1) ;
  }
  else {
    /* decide if the key is a phone */
    for(type=0;type<3;type++) {
      inx = Key2PhoneInx(key, type, pZuin->kbtype, 1) ;
      if(inx) break ;
    }
    if(type == 3) { /* the key is NOT a phone */
      return ZUIN_KEY_ERROR;
    }
    /* fill the key into the phone buffer */
    pZuin->pho_inx[type] = inx ;
    return ZUIN_ABSORB;
  }
}

// 1999.12.6 kpchen
int HsuPhoInput(ZuinData *pZuin, int key)
{
  int type = 0, searchTimes = 0, inx = 0;

  if ( IsHsuPhoEndKey(pZuin->pho_inx, key) ) {
    if (pZuin->pho_inx[1]==0 && pZuin->pho_inx[2]==0) {
      //convert "������" to "������"
      if (12 <= pZuin->pho_inx[0] && pZuin->pho_inx[0] <= 14) {
      	pZuin->pho_inx[0] += 3 ;
      }
      //convert "�~" to "��"
      else if (pZuin->pho_inx[0] == 11 ) {
      	pZuin->pho_inx[0] = 0;
      	pZuin->pho_inx[2] = 2;
      }
      //convert "�|" to "��"
      else if (pZuin->pho_inx[0] == 9 ) {
      	pZuin->pho_inx[0] = 0;
      	pZuin->pho_inx[2] = 3;
      }
      //convert "�v" to "��"
      else if (pZuin->pho_inx[0] == 3 ) {
      	pZuin->pho_inx[0] = 0;
      	pZuin->pho_inx[2] = 9;
      }
      //convert "�z" to "��"
      else if (pZuin->pho_inx[0] == 7 ) {
      	pZuin->pho_inx[0] = 0;
      	pZuin->pho_inx[2] = 10;
      }
      //convert "�}" to "��"
      else if (pZuin->pho_inx[0] == 10 ) {
      	pZuin->pho_inx[0] = 0;
      	pZuin->pho_inx[2] = 11;
      }
      //convert "�{" to "��"
      else if (pZuin->pho_inx[0] == 8 ) {
      	pZuin->pho_inx[0] = 0;
      	pZuin->pho_inx[2] = 13;
      }
    } 
    searchTimes = ( key == 'j' ) ? 3 : 2 ;
    return EndKeyProcess(pZuin, key, searchTimes) ;
  }
  else {
    /* decide if the key is a phone */
    for(type=0,searchTimes=1; type<4; type++) {
      inx = Key2PhoneInx(key, type, pZuin->kbtype, searchTimes) ;
      if(!inx)
        continue ; /* if inx == 0, next type */
      else if (type == 0) {
        if (pZuin->pho_inx[0] || pZuin->pho_inx[1]) {/* if inx !=0 */
          searchTimes = 2 ; /* possible infinite loop here */
        }
	      else break;
      }
      else if (type == 1 && inx ==1) { /* handle i and e*/
        if (pZuin->pho_inx[1]) {
          searchTimes = 2;
        }
	      else break;
      }
      else break ;	
    }
    // processing very special cases "j v c"
    if(type==1 && inx==2
       && 12 <= pZuin->pho_inx[0] && pZuin->pho_inx[0] <= 14) {
      pZuin->pho_inx[0] += 3 ;
    }
   
    /* ������ ���� ���� */
    if(type==2 && pZuin->pho_inx[1] == 0
       && 12 <= pZuin->pho_inx[0] && pZuin->pho_inx[0] <= 14) {
      pZuin->pho_inx[0] += 3 ;
    }
   
    if(type == 3) { /* the key is NOT a phone */
      if(isalpha(key)) return ZUIN_NO_WORD ;
      return ZUIN_KEY_ERROR;
    }
    /* fill the key into the phone buffer */
    pZuin->pho_inx[type] = inx ;
    return ZUIN_ABSORB;

  }
}

/* copy the idea from hsu */
int ET26PhoInput(ZuinData *pZuin, int key) {
  int type=0, searchTimes=0, inx=0 ;

  if ( IsET26PhoEndKey(pZuin->pho_inx, key) ) {
    if (pZuin->pho_inx[1]==0 && pZuin->pho_inx[2]==0) {
      //convert "����" to "����"
      if (pZuin->pho_inx[0] == 12 || pZuin->pho_inx[0] == 14) {
        pZuin->pho_inx[0] += 3 ;
      }
      //convert "�u" to "��"
      else if (pZuin->pho_inx[0] == 2) {
      pZuin->pho_inx[0] = 0;
      pZuin->pho_inx[2] = 8;
      }
      //convert "�v" to "��"
      else if (pZuin->pho_inx[0] == 3) {
        pZuin->pho_inx[0] = 0;
        pZuin->pho_inx[2] = 9;
      }
      //convert "�z" to "��"
      else if (pZuin->pho_inx[0] == 7) {
        pZuin->pho_inx[0] = 0;
        pZuin->pho_inx[2] = 10;
      }
      //convert "�y" to "��"
      else if (pZuin->pho_inx[0] == 6 ) {
      pZuin->pho_inx[0] = 0;
      pZuin->pho_inx[2] = 11;
      }
      //convert "�{" to "��"
      else if (pZuin->pho_inx[0] == 8 ) {
        pZuin->pho_inx[0] = 0;
        pZuin->pho_inx[2] = 12;
      }
      //convert "�~" to "��"
      else if (pZuin->pho_inx[0] == 11 ) {
        pZuin->pho_inx[0] = 0;
        pZuin->pho_inx[2] = 13;
      }
    }
    searchTimes = 2;
    return EndKeyProcess(pZuin, key, searchTimes) ;
  }
  else {
    /* decide if the key is a phone */
    for(type=0,searchTimes=1; type<3; type++) {
      inx = Key2PhoneInx(key, type, pZuin->kbtype, searchTimes) ;
      if(!inx) 
        continue ; /* if inx == 0, next type */
      else if (type == 0) {
        if (pZuin->pho_inx[0] || pZuin->pho_inx[1]) {/* if inx !=0 */
          searchTimes = 2 ; /* possible infinite loop here */
        }
        else
          break;
      }
      else
	break ;	
    }
    /* convert "����" to "����" */
    if (type == 1) {
      if (inx == 2) {
        if (pZuin->pho_inx[0] == 12 || pZuin->pho_inx[0] == 14) {
          pZuin->pho_inx[0] += 3;
	}
      }
      else {
        /* convert "�|" to "��" */
        if (pZuin->pho_inx[0] == 9) {
          pZuin->pho_inx [0] = 13;	
	}
      }
    }
   
    if(type==2 && pZuin->pho_inx[1]==0
       && (pZuin->pho_inx[0] == 12 || pZuin->pho_inx[0] == 14)) {
      pZuin->pho_inx[0] += 3 ;
    }
   
    if(type == 3) { /* the key is NOT a phone */
      if(isalpha(key)) return ZUIN_NO_WORD ;
      return ZUIN_KEY_ERROR;
    }
    /* fill the key into the phone buffer */
    pZuin->pho_inx[type] = inx ;
/*    printf("\n in 0 is %d\n",pZuin->pho_inx[0]);
    printf(" in 1 is %d\n",pZuin->pho_inx[1]);
    printf(" in 2 is %d\n",pZuin->pho_inx[2]);
    printf(" in 3 is %d\n",pZuin->pho_inx[3]); */
    return ZUIN_ABSORB;
  }
}

int ZuinPhoInput(ZuinData *pZuin,int key)
     // key: ascii code of input, including space
{
  switch( pZuin->kbtype) {
  case KB_HSU:
    return HsuPhoInput(pZuin,key) ;
    break ;
  case KB_ET26:
    return ET26PhoInput(pZuin,key);
    break;
  default:
    return DefPhoInput(pZuin,key);		
  }	
  return ZUIN_ERROR;
}

// remove the latest key
int ZuinRemoveLast(ZuinData *pZuin)
{
  int i;
	
  for(i=3;i>=0;i--) {
    if( pZuin->pho_inx[i] ) {
      pZuin->pho_inx[i] = 0;
      return 0;
    }
  }
  return 0;
}

// remove all the key entered
int ZuinRemoveAll(ZuinData *pZuin)
{
  memset( pZuin->pho_inx, 0, sizeof(pZuin->pho_inx) );
  return 0;
}

int ZuinIsEntering(ZuinData *pZuin)
{
  int i;

  for(i=0;i<4;i++)
    if( pZuin->pho_inx[i] )
      return 1;
  return 0;
}
 
