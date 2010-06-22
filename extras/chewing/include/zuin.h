/*
	$Id: zuin.h,v 1.1.1.1 2002/05/03 04:01:08 kids Exp $
*/
#ifndef _ZUIN_H

#define _ZUIN_H

#define KB_TYPE_NUM 6
#define KB_DEFAULT 0
#define KB_HSU 1
#define KB_IBM 2
#define KB_GIN_YIEH 3
#define KB_ET 4
#define KB_ET26 5

#define ZUIN_IGNORE 0
#define ZUIN_ABSORB 1
#define ZUIN_COMMIT 2
#define ZUIN_KEY_ERROR 4
#define ZUIN_ERROR 8
#define ZUIN_NO_WORD 16

// Zuin module
int ZuinPhoInput(ZuinData *,int key);  // assume `key' is "ascii" code.
int ZuinRemoveLast(ZuinData *);
int ZuinRemoveAll(ZuinData *) ;
int ZuinIsEntering(ZuinData *);
         
#endif






