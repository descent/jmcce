#ifndef _GOING_IO_H
#define _GOING_IO_H
#include"global.h"

#define KEYSTROKE_IGNORE 1
#define KEYSTROKE_COMMIT 2
#define KEYSTROKE_BELL 4
#define KEYSTROKE_ABSORB 8

int OnKeyEsc(void*, ChewingOutput*) ;
int OnKeyEnter(void*, ChewingOutput*);
int OnKeyDel(void*, ChewingOutput*);
int OnKeyBackspace(void*, ChewingOutput*);
int OnKeyTab(void*, ChewingOutput*) ;
int OnKeyLeft(void*, ChewingOutput*);
int OnKeyRight(void*, ChewingOutput*);
int OnKeyUp(void*, ChewingOutput*) ;
int OnKeyHome(void*, ChewingOutput*);
int OnKeyEnd(void*, ChewingOutput*);
int OnKeyDown(void*, ChewingOutput*);
int OnKeyCapslock(void*, ChewingOutput*);
int OnKeyDefault(void*,int, ChewingOutput*);
int OnKeyCtrlNum(void*, int, ChewingOutput*) ;
int OnKeyCtrlDefault(void*, ChewingOutput*) ;
int SetConfig(void *iccf, ConfigData *pcd) ;
int KBStr2Num(char str[]) ;
int InitChewing(void *iccf, ChewingConf *cf) ;
void SetChiEngMode(void *iccf,int mode);
int GetChiEngMode(void *iccf);
int OnKeyDblTab(void *iccf, ChewingOutput*);
#endif
