#ifndef USERPHRASE_H
#define USERPHRASE_H

#define FREQ_INIT_VALUE (1)
#define SHORT_INCREASE_FREQ (10)
#define MEDIUM_INCREASE_FREQ (5)
#define LONG_DECREASE_FREQ (10)
#define MAX_ALLOW_FREQ (99999999)

#define USER_UPDATE_FAIL (4)
#define USER_UPDATE_INSERT (1)
#define USER_UPDATE_MODIFY (2)
#define USER_UPDATE_IGNORE (8)

#include "global.h"

typedef struct {
  uint16 *phoneSeq ;
  char *wordSeq ;
  int userfreq ;
  int recentTime ;
  int origfreq ;		// the initial frequency of this phrase
  int maxfreq ;			// the maximum frequency of the phrase of the same pid
} UserPhraseData ;

int UserUpdatePhrase(const uint16 phoneSeq[], const char wordSeq[]) ;
UserPhraseData *UserGetPhraseFirst(const uint16 phoneSeq[]) ;
UserPhraseData *UserGetPhraseNext()  ;
void UpdateLifetime(void) ;
#endif
