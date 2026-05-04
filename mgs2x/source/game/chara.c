//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	chara.c
		キャラクタエントリ管理ルーチン

	1999/05/31 K.Uehara
	$Id: chara.c,v 1.16 2002/07/15 11:01:44 usr01475 Exp $

	2002/01/23 M.Kobayashi
	for XBOX とりあえず静的リンクするシステムに書き換え
*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#include "libgv.h"
#include "libgv.cnf"
#include "libgcl.h"
#include "game.h"

#ifdef	PSX2
typedef char byte;
#endif

/* stage/binhead.s */
typedef struct {
	void *bintop;
	void *bsstop;
	void *bssend;
	void *charatop;
} BINHEAD;

CHARA *StageCharacterEntries;
static int StageCharacterNum = 0;

extern CHARA _StageCharacterEntries_mselect[];
static int StageCharacterNum_mselect = 0;

// this is the big list of stage character entries from _stage
extern CHARA _StageCharacterEntries__stage[];
static int StageCharacterNum_stage = 0;

#if 1 //BP_XBOX def KP_XBOX
static CHARA nullchara = {
	0,
	NULL,
};
#endif

void GM_InitChara( void )
{
#if 1 //BP_XBOX def KP_XBOX
	StageCharacterEntries = &nullchara;
#else
	extern int _mgs2_keep_end[];
	StageCharacterEntries = ( CHARA * )( _mgs2_keep_end );
#endif	
	StageCharacterNum = 0;
}

void GM_ResetChara( void )
{
#if 1 //BP_XBOX def KP_XBOX
	StageCharacterEntries = &nullchara;
#else
	CHARA *chara;
	/*
		ステージの最初に呼ばれる。
		キャラテーブルをクリア
	*/

	chara = StageCharacterEntries;
	chara->new = NULL;
#endif	
	StageCharacterNum = 0;
}

static int get_chara_num( CHARA *stageCharacterEntries )
{
	CHARA *chara;
	int n;

	n = 0;
	for( chara = stageCharacterEntries; chara->new != NULL; chara ++ ){
		n++;
	}
	return n;
}


void GM_SetupChara( void )
{
   extern void BP_InitStageBSS();
   extern CHARA * BP_GetCurrentStageFunctions();

   BP_InitStageBSS();

   StageCharacterEntries = BP_GetCurrentStageFunctions();
   StageCharacterNum = get_chara_num(StageCharacterEntries);

   // count mselect's characters
   if (StageCharacterNum_mselect == 0)
   {
      StageCharacterNum_mselect = get_chara_num(_StageCharacterEntries_mselect);
   }

   // count _stage.c's characters
   if (StageCharacterNum_stage == 0)
   {
      StageCharacterNum_stage = get_chara_num(_StageCharacterEntries__stage);
   }
}

void *GM_GetCharaID( int nID )
{
	// MGS2 は常駐キャラも、StageCharacterEntriesに記述
	CHARA *chara;
#if 0
	for( chara = StageCharacterEntries; chara->new != NULL; chara ++ ){
		if( chara->class_id == nID ){
			return chara->new;
		}
	}
#else
	// バイナリサーチ

	chara = GCL_SearchId( StageCharacterEntries, nID, StageCharacterNum );
	if( chara != NULL ){
		return chara->new;
	}
#endif

   // BP FIX
   // fall back to mselect's chara entries for VR missions
   chara = GCL_SearchId( _StageCharacterEntries_mselect, nID, StageCharacterNum_mselect );
   if (chara != NULL )
   {
      return chara->new;
   }
   // then try _stage.c
   chara = GCL_SearchId( _StageCharacterEntries__stage, nID, StageCharacterNum_stage );
   if (chara != NULL)
   {
      return chara->new;
   }

   printf( "ERROR! unknown chara-class %8.8X - Area %s\n", nID, GM_GetArea() );
   {
      // BP - JM
      // We have an unknown chara-class, which means that the stage chara file is
      // not correct and missing a lookup entry.  Crash the game in this case.

      HANGUP();
   }

   return NULL;
}

int CODEC_MemCallReset(void);

#if 0 //BP_TODO
#ifdef KP_XBOX
#include <xgraphics.h>
#endif
#endif

void *_CHARA_resident_chara_table[] = {
#ifdef DEBUG_MODE
	GM_StartDebugViewer,
#endif
	GM_StartMenuPrimManager,
	CODEC_MemCallReset,

#ifdef KP_XBOX
	// 呼び出しておかないとやばいライブラリ関数
#if 0 //BP_TODO
   XGWriteSurfaceOrTextureToXPR,
#endif
#endif
	
};
