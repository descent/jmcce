#ifndef _GOINGUTIL_H
#define _GOINGUTIL_H

#define SYMBOL_KEY_OK 0
#define SYMBOL_KEY_ERROR 1
#define DECREASE_CURSOR 1
#define NONDECREASE_CURSOR 0

void AutoLearnPhrase(ChewingData *pgdata) ;
void SetUpdatePhraseMsg(ChewingData *pgdata, char *addWordSeq, int len, int state) ;
int NoSymbolBetween(ChewingData *pgdata, int begin, int end) ;
int ChewingIsEntering(ChewingData *pgdata);
void CleanAllBuf(ChewingData *) ;
int SpecialSymbolInput(int key, ChewingData *pgdata) ;
int SymbolInput(int key, ChewingData *pgdata) ;
int WriteChiSymbolToBuf(wch_t csBuf[], int csBufLen, ChewingData *pgdata) ;
int ReleaseChiSymbolBuf(ChewingData *pgdata, ChewingOutput*)  ;
int AddChi(uint16 phone, ChewingData *pgdata) ;
int CallPhrasing(ChewingData *pgdata) ;
int MakeOutput(ChewingOutput *pgo, ChewingData *pgdata) ;
int MakeOutputWithRtn(ChewingOutput *pgo, ChewingData *pgdata, int keystrokeRtn) ;
void MakeOutputAddMsgAndCleanInterval(ChewingOutput *pgo, ChewingData *pgdata) ;
int AddSelect(ChewingData *pgdata, int sel_i) ;
int CountSelKeyNum(int key, ChewingData *pgdata)  ;
int ChewingIsChiAt(int cursor, ChewingData *pgdata) ;
int ChewingKillSelectIntervalAcross(int cursor, ChewingData *pgdata)    ;
int KillCharInSelectIntervalAndBrkpt(ChewingData *pgdata, int cursorToKill) ;
int ChewingKillChar(ChewingData *pgdata, 
		  int cursorToKill, 
		  int chiSymbolCursorToKill, 
		  int minus) ;
void RemoveSelectElement(int i, ChewingData *pgdata) ;
int IsPreferIntervalConnted( int cursor, ChewingData *pgdata);



#endif
