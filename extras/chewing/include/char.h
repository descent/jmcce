#ifndef CHAR_H

#define CHAR_H

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#include"global.h"

#define PHONE_NUM (1319)
typedef struct {
	char word[3] ;
} Word ;

int InitChar(const char *) ;
int GetCharFirst(Word *, uint16) ;
int GetCharNext (Word *) ;

#endif
