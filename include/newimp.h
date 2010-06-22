#ifndef _JMCCE_NEWIMP_H_
#define _JMCCE_NEWIMP_H_


typedef unsigned short int WORD;
typedef unsigned char CHAR;


#define MAX_INPUT_LEN 20


typedef struct
{
  char szId[4];
  char szPrompt[20];
  char szEngName[20];
  char szVersion[5];
  char szWin[3];
  char szEnCode[4];
  char szLastModiDate[6];
  char szMinKeyLen;
  char szMaxKeyLen;
  char szMaxPhraseLen;

  long szPhraseCountExact;
  long szPhraseCountShow;
  long szCharCountShow;

  char szSymbolCountShow[2];
  char szMaxKeyLenFull;
  char szMaxKeyLenFullLimit;
  char szBeepLevel;
  char szAutoSelect;
  char szAutoSendLast;
  char szSelectListCount;
  char szSelectZeroLeading;
  char szSelectHorizantal;
  char szAssocAfterCharInp;
  char szAssocAfterPhraseInp;
  char szUserAssocAfterCharInp;
  char szUserAssocAfterCharFile[12];
  char szUserAssocAfterPhraseInp;
  char szUserAssocAfterPhraseFile[12];
  char szStaticRecordAfterUse;
  char szStaticRecordFile[12];
  char szAssocStaticRecordAfterUse;
  char szAssocStaticRecordFile[12];
  char szMergeToAssocLevel;
  char szIndexWithEndKey;
  char szCaseTransPolicy;
  char szSuperEnter;
  char szUseUserFonts;
  char szUserFontsType;
  char szUseAppend41OfETEN;
  char szUseKanaOfETEN;
  char szUsedUserFontArea;
  char szChoiceCharByPhrase;
  char szNotUsed[105];

}
__attribute__ ((packed)) TUI_Header;


#endif /* _JMCCE_NEWIMP_H_ */
