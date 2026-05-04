//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	eft_act.c
	エフェクト寿命アクター

	2000/08/28 T.Morita
	$Id: eft_act.c,v 1.2 2002/12/04 10:29:00 takaki Exp $

*/
/*

  void *NewDemoEffect( DEMO_EFFECT *packet )
  エフェクト クッションアクタの起動

*/

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

#include <mgs_type.h>
#include <libgv.h>
#include <libdg.h>
#include <libfs.h>
#include <stream.h>
#include <gameheader.h>

#include "libdemo.h"


/* ---------------------------------------------------------------- */
typedef struct _work{
    GV_ACT_EX	actor ;
    int		life ;	/* ライフ */
    int         flag ;  /* 今後の拡張のため */
	int		effect_id ;
	void	*effect_work ;
} Work ;

static void Act( Work *work )
{
    work->life -= DM_FrameSkip + 1 ;
    if ( work->life < 0 )
	GV_DestroyActor( work ) ;
}

void *NewDemoEffectLaunch( DEMO_EFFECT *packet, void *parent )
{
    Work *work = NULL ;
    void *work_ptr = NULL ;
    DM_EFFECT_EXEC exec ;

    exec = (DM_EFFECT_EXEC)GM_GetCharaID( packet->effect_id ) ;
    if ( exec )
	switch( packet->exec_type )
	{
	case 0:/* 起動のみ 0フレーム終了のエフェクト（DM_SendMessageなど） */
	    (* exec)( DEMO_PACKET_ID( packet->header ), (int *)packet->data ) ;
		break ;
	    
	case 1:/* プロセスの終了管理 */
	    /* アクター生成 */
	    work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	    if ( work != NULL )
	    {
		GV_SetActor( &work->actor, Act, NULL ) ;
		GV_ActorEX( &work->actor ) ;

		work_ptr = (* exec)( DEMO_PACKET_ID( packet->header ), (int *)packet->data+4 ) ; 
		if ( !work_ptr )
		{
		    GV_DestroyActor( work ) ;
		    return NULL ;
		}
		work->life = *((int *)packet->data + 0) ;
		work->flag = DEMO_PACKET_ID( packet->header ) ;
		//work->flag = *((int *)packet->data + 1) ;/*あと3つまで増やせる*/
		work->effect_id = packet->effect_id ;
		work->effect_work = work_ptr ;

		/*親子関係を作る*/
		GV_SetActorChild( parent, work ) ;
		GV_SetActorChild( work, work_ptr ) ;

		printf( "(work%x child%x) life%d\n",work, work_ptr,work->life ) ;
#ifndef DEBUG_MODE
printf("effect id = %08x\n", work->effect_id );
#endif
	    }
	    return work ;
	}
    return NULL ;
}

/* ---------------------------------------------------------------- */
/* 指定したランチャーワークのエフェクトＩＤを取得する */
int DM_CheckEffectIDFromLauncher( void *work_ptr )
{
	Work	*work = work_ptr ;
	if ( work == NULL ) return ( 0 );

	return ( work->effect_id );
}
/* 指定したワークのエフェクトランチャーのライフをフレーム単位で補正する */
void DM_CorrectEffectLaunchLife( void *work_ptr, int frame )
{
	Work	*work = work_ptr ;
	if ( work == NULL ) return ;

	work->life += frame ;
}
