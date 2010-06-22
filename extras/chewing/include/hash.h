// $Id: hash.h,v 1.1.1.1 2002/05/03 04:01:08 kids Exp $

#ifndef HASH_H
#define HASH_H
#include "global.h"
#include "userphrase.h"

#define FIELD_SIZE (125)
#define N_HASH_BIT (14)
#define HASH_FILE  "hash.dat"
#define HASH_TABLE_SIZE (1<<N_HASH_BIT)

typedef struct tag_HASH_ITEM {
  int item_index ;
  UserPhraseData data ;
  struct tag_HASH_ITEM *next ;
} HASH_ITEM ;

HASH_ITEM *HashFindPhone(const uint16 phoneSeq[]) ;
HASH_ITEM *HashFindEntry(const uint16 phoneSeq[], const char wordSeq[]) ;
HASH_ITEM *HashInsert(UserPhraseData *pData) ;
HASH_ITEM *HashFindPhonePhrase(const uint16 phoneSeq[], HASH_ITEM *pHashLast) ;
void HashModify(HASH_ITEM *pItem) ;
int AlcUserPhraseSeq(UserPhraseData *pData, int len) ;
int ReadHash(void) ;
void FreeHashTable(void) ;
#endif
