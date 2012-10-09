#ifndef DICT_H

#define DICT_H

#ifndef SEEK_SET
#define SEEK_SET 0
#endif

#include"global.h"

#define PHONE_PHRASE_NUM (94000)
int InitDict(const char *) ;
int GetPhraseFirst(Phrase *phr_ptr, int phone_phr_id) ;
int GetPhraseNext (Phrase *phr_ptr) ;

#endif
