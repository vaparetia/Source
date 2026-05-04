//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	x_patch.c
	ＸＢＯＸ用パッチ

	2002/07/31 K.Takabe
	$Id: x_patch.c,v 1.3 2002/11/23 12:46:57 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include	"gameheader.h"
#include	"../../mode/demo/libdemo.h"

extern int DM_CheckEffectIDFromLauncher( void *work_ptr );
extern void DM_CorrectEffectLaunchLife( void *work_ptr, int frame );

/* ---------------------------------------------------------------- */
#define HAIR_MODEL_ID		(5518691)	/* "rai_hair_mh_mt" */

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT		actor ;
	EVM_DEF		*def ;
} Work_00 ;


/* ---------------------------------------------------------------- */
static void Act_00( Work_00 *work )
{
	DG_OBJ_BUFFER	*obj_buff ;
	DG_EVMOBJ		*evmobj, **que ;
	int				i ;

#ifndef PSX2
	/* ライデンの髪の毛オブジェクトを検索し、後回し描画フラグをクリアすることで描画をステージモデルより先に変更する */
	obj_buff = &( DG_Chanl(0)->obj_queue->evmobj_buffer ) ;
	que = (DG_EVMOBJ**)obj_buff->queue ;
	for ( i = obj_buff->n_queue ; i > 0 ; i--, que++ ){
		evmobj = *que ;
		if ( evmobj->def == work->def ){
			if ( DG_Chanl(0)->eye.m[3][1] < 4000.0f ){
				/* カメラが床より下の場合のみ描画順を変更する */
				evmobj->flag &= ~DG_EVMOBJ_LATTERDRAW ;
			} else {
				evmobj->flag |= DG_EVMOBJ_LATTERDRAW ;
			}
		}
	}
#endif
}
/* ---------------------------------------------------------------- */
static void Die_00( Work_00 *work )
{
}
/* ---------------------------------------------------------------- */
static int GetResources_00( Work_00 *work )
{
	work->def = GV_GetCache( GV_CacheID( HAIR_MODEL_ID, 'e' ) );
	return ( 0 );
}
/* ---------------------------------------------------------------- */
int NewXboxPatch_From_w46a_demo( void )
{
	Work_00		*work ;

	OPERATOR() ;

	work = (Work_00 *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work_00 ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act_00, Die_00 ) ;
		if ( GetResources_00( work ) < 0 ){
			GV_DestroyActor( work );
		}
	}
	return ( 0 );
}

/* **************************************************************** */

typedef	struct	{
	GV_ACT		actor ;
	int			flag ;
	int			count ;
} Work_01 ;

static void Act_01( Work_01 *work )
{
	work->count += ( DM_FrameSkip + 1 ) * TIME_BASE ;
	//if ( GV_PadData[1].press & PAD_A ) printf("count = %d\n", work->count );
	if ( work->count < 13000 ) return ;
	if ( work->flag == 0 ){
		DM_WORK		*dm_work ;
		DM_EFFECT	*effect ;
		int			i ;

		/* デモ管理構造体へのポインタを取得 */
		if ( ( dm_work = DM_GetDemoWork() ) == NULL ) return ;

		/* 検索 */
		effect = dm_work->eft_list ;
		for ( i = dm_work->n_effect ; i > 0 ; effect++, i-- ){
			/* effect "物理シム 新髪の毛" [NewHairModel_Demo] <id:0x3008><force_effect> */
			//printf("%d, %08x\n", DM_CheckEffectIDFromLauncher( effect->work_ptr ) );
			if ( DM_CheckEffectIDFromLauncher( effect->work_ptr ) == 0x01003007 ){
				DM_CorrectEffectLaunchLife( effect->work_ptr, 5 );
				work->flag = 1 ;
			}
		}
	}
}
static void Die_01( Work_01 *work )
{
}
static int GetResources_01( Work_01 *work )
{
	work->flag = 0 ;
	return ( 0 );
}
int NewXboxPatch_From_d005p01_demo( void )
{
	Work_00		*work ;

	OPERATOR() ;

	work = (Work_01 *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work_01 ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act_01, Die_01 ) ;
		if ( GetResources_01( work ) < 0 ){
			GV_DestroyActor( work );
		}
	}
	return ( 0 );
}

