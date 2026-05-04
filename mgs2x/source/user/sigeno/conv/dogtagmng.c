//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/***********************************************************
	dogtagmng.c
	取得ドッグタグ管理
	2001.05.15 K.Sigeno 
	$Id: dogtagmng.c,v 1.1.1.3 2002/11/19 11:49:06 Yoshizawa1 Exp $
************************************************************/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>

#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

#include "BP_TrophySystem.h"

#define INT_SIZE 32
#define DOGTAG_BUFSIZE 32
//32*32=1024bit で1024人記録可能
/*複数ドッグタグキャラ対応用 特別BIT*/
#define DOGTAG_SNAKE_ID	(1023)
#define DOGTAG_OLGA_ID	(1022)
#define MAX_GAME_LEVEL	(5)

void SIG_SetDogTagFlagSnake(void){
	int block ;
	block = DOGTAG_SNAKE_ID/INT_SIZE ;
//	GM_DogTagFlag[block] |= (1<<(DOGTAG_SNAKE_ID%INT_SIZE)) ;
}
void SIG_SetDogTagFlagOlga(void){
	int block ;
	block = DOGTAG_OLGA_ID/INT_SIZE ;
//	GM_DogTagFlag[block] |= (1<<(DOGTAG_OLGA_ID%INT_SIZE)) ;
}

void SIG_DogTagZeroClear(void){
	int i;
	for(i=0;i<DOGTAG_BUFSIZE;i++){
		GM_DogTagFlag[i] = 0 ;
	}
}
static //BP (verifying that nobody calls this except for the script function)
void SIG_SetDogTagFlag(int num){
	int block ;
	ASSERT(num<(INT_SIZE * DOGTAG_BUFSIZE)) ;
	block = num/INT_SIZE ;
	GM_DogTagFlag[block] |= (1<<(num%INT_SIZE)) ;
	GCL_SaveLinkVar( &GM_DogTagFlag[block], sizeof( int ) ) ;
/*アイテム数のほうもSave*/
	GCL_SaveLinkVar( &_GM_ItemsR[ IT_DogTag ], sizeof( short ) ) ;
	GCL_SaveLinkVar( &_GM_Items[ IT_DogTag ], sizeof( short ) ) ;

   BP_TrophySystem_UnlockTrophy(kTRP_GotDogTag);
}

void SIG_SetDogTagFlagName(void){
	char *ptr ;
	int num ;
	ptr = GCL_GetNextString();
	GCL_SetArgTop( ptr );
	num = GCL_GetNextInt();
	printf("GET DOGTAG ID == [%d]\n",num);
	SIG_SetDogTagFlag(num) ;
}

void SIG_ResetDogTagFlag(int num){
	int block ;
	ASSERT(num<(INT_SIZE * DOGTAG_BUFSIZE)) ;
	block = num/INT_SIZE ;
	GM_DogTagFlag[block] &= ~(1<<(num%INT_SIZE)) ;
}
int SIG_CheckDogTagFlag(int num){
	int block ;
	ASSERT(num<(INT_SIZE * DOGTAG_BUFSIZE)) ;
	block = num/INT_SIZE ;
	return ((GM_DogTagFlag[block] & (1<<(num%INT_SIZE)))!= 0) ;
}
int SIG_CheckDogTagFlag2(int num){
	int block ;
	int *flags;
	void *linkvar=GCL_GetLinkvarSaveAreaTop();
	ASSERT(num<(INT_SIZE * DOGTAG_BUFSIZE)) ;
	block = num/INT_SIZE ;
	flags=(int *)((int)(linkvar)+
				  (((int)GM_DogTagFlag)-((int)linkvarbuf)));
	return ((flags[block] & (1<<(num%INT_SIZE)))!= 0) ;
}
#if 1
/*OLGA & SNAKE CHECK Ver*/ 
int SIG_GetTotalDogTagNum(void){
	int num = 0,i;
	for(i=0;i<(INT_SIZE * DOGTAG_BUFSIZE);i++){
		if(SIG_CheckDogTagFlag(i)){
			num++;
		}
	}
#if 0
	/*1人で複数ドッグタグのキャラは減らしとく*/
	if(SIG_CheckDogTagFlag(DOGTAG_SNAKE_ID)){
		num -= MAX_GAME_LEVEL ;
	}
	if(SIG_CheckDogTagFlag(DOGTAG_OLGA_ID)){
		num -= MAX_GAME_LEVEL ;
	}
#else
	/*1人で複数ドッグタグのキャラは減らしとく*/
	if(SIG_CheckDogTagFlag(DOGTAG_SNAKE_ID)){
//		num -= MAX_GAME_LEVEL ;
		num-- ;
	}
	if(SIG_CheckDogTagFlag(DOGTAG_OLGA_ID)){
//		num -= MAX_GAME_LEVEL ;
		num-- ;
	}
#endif
	return num ;
}
#else
int SIG_GetTotalDogTagNum(void){
	int num = 0,i;
	for(i=0;i<(INT_SIZE * DOGTAG_BUFSIZE);i++){
		if(SIG_CheckDogTagFlag(i)){
			num++;
		}
	}
	return num ;
}
#endif
/*gcl command*/
void NewSIG_SetDogTagFlag( void )
{
	int id;
	id = GCL_GetNextInt();
#if 0
	if(SIG_CheckDogTagFlag(id)){
		/*すでに取ってます*/
	}else {
		/*初めてとります*/
		GM_IncrementItem( IT_DogTag, 1 ) ;
	}
#endif
//	GM_SetItemNum( IT_DogTag,SIG_GetTotalDogTagNum());
	SIG_SetDogTagFlag(id) ;
}

