//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	sysmouth.c
	口パクプログラムをコールする常駐型システム 
	*NewMouthSys( OBJECT * , int * );
	2000/07/07 K.Sigeno
	$Id: sysmouth.c,v 1.1.1.3 2002/11/19 11:49:49 Yoshizawa1 Exp $
*/

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <float.h>

#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"

#include "korekado/enemy/enemy.h"

#include "mouth.h"

/* ワーク */
typedef	struct _Work {
	GV_ACT_EX	actor ;
	OBJECT	*human ;		/* 接続先モデル*/
	int		*sw ;	/*リクエスト*/
//	int		now_mode;	/*現在の目標形状番号*/
	int		task;		/*アクターの動作状況*/
	int		req_count;		/*補完時間*/
} Work ;

/* ------------------------------------------------------- */

/* メイン処理 */


/***
#define	SW_FLAG_SWITCH1	0x00000002
#define	SW_FLAG_SWITCH2	0x00000004
#define	SW_FLAG_SWITCH3	0x00000008
#define	SW_FLAG_SWITCH4	0x00000010
０ デフォルト
１ い
２ お
***/
extern void *NewMouthAnim( OBJECT *,int *,int *,int *);

static void Act(Work *work)
{
	if(*work->sw >= FACE_ANIM_NUM){
		*work->sw = (FACE_ANIM_NUM-1) ;
	}
	if(*work->sw < 0){
		*work->sw = 0 ;
	}

	if(
	(work->task < GET_RES )
	&&(*work->sw != 0)
	){
		/*補完時間を設定*/
		work->req_count = FACE_INTERP ;
#if 0
		NewMouthAnim( work->human,&work->task,work->sw,
		&work->req_count) ;
#else
		GV_SetActorChild( &work->actor ,
			NewMouthAnim( work->human,&work->task,work->sw,
			&work->req_count) 
		);
#endif

	}else {
		work->task = OFF ;
	}
}

static void Die(Work *work)
{
}

/* ------------------------------------------------------------------------ */

/* 資源を獲得 */
static int GetResources(Work *work,OBJECT *human,int *sw)
{
	work->human = human;
	work->sw = sw ;
	work->task = OFF ;
//	work->	now_mode;	/*現在の目標形状番号*/
    return 1;
}


/* 初期化部メイン */

#define PRIO	0x40	/* 敵兵より後に処理 */

void *NewMouthSys( human ,sw )
OBJECT	*human ;	/* 人体オブジェクト */
int		*sw ; 		/* 制御スイッチ */
{
    Work *work ;
	work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if(work!=NULL) {
		GV_SetActor(&(work->actor),Act,Die) ;
		GV_ActorEX( &work->actor ) ;
		if(!GetResources(work,human,sw)){
			GV_DestroyActor(work) ;
			return NULL ;
		}
		
	}
	return (void *)work ;
}
