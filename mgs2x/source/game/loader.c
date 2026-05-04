//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ロード処理プロセス
		libfsのステージ処理プロセスを使用して、ステージデータを読み込み、
		初期化処理を行う。

	1999/05/31 K.Uehara
	$Id: loader.c,v 1.1.1.3 2002/11/19 11:41:52 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<sys/types.h>

#include	"libgv.h"
#include	"libfs.h"

#include	"game.h"
#include "BP_Misc.h"

/* ------------------------------------------- */

typedef struct {
	GV_ACT actor;
	void *info;
	int type;
	int reading;
	int time;
   char loadDir[128];
} Work;

static void Act( Work *work )
{
	work->time ++;

	// ロードのタイプに応じて処理を行うことがある。

	if( work->reading ){
		if( ! FS_LoadStageSync( work->info ) ){
			/* 読み込み完了 */
			work->reading = 0;
		}
	} else {
		/* プロセス終了 */
		GV_DestroyActor( work );
	}
}

static void Die( Work *work )
{
   BP_SetSpinnerLoadFlag(kLoadFlag_Stage, 0);

	printf( "LOAD END\n" );
	FS_LoadStageComplete( work->info );
	GM_LoadComplete = -1;
}

void *GM_StartLoader( char *dir )
{
	Work *work;

	work = ( Work * )GV_NewActor( GV_ACTOR_ASSIST, sizeof( Work ) );
	ASSERT( work != NULL );

	printf( "loadreq %s\n", dir );

	if( ( work->info = FS_LoadStageRequest( dir ) ) == NULL ){
		printf( "STAGE %s not found\n", dir );
		HANGUP();
	}
	GV_SetActor( work, Act, Die );

	work->type = 0;
	work->reading = 1;

   strcpy(work->loadDir, dir);

   BP_SetSpinnerLoadFlag(kLoadFlag_Stage, 1);

	GM_LoadComplete = 0;

	return work;
}
