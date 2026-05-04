//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  vamp_utl.c
  ヴァンプ ユーティリティー関数

  2001/03/23 T.Morita
  $Id: vamp_utl.c,v 1.1.1.3 2002/11/19 11:46:36 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"include/vamp.h"



/*

  ゲームエンド ライフ関係

*/
int VMPS_GameOverCheck( Work *work, int pad )
{
    if ( GM_IsGameOver() )
	return 0 ;

    if ( work->npc.action.status & NPC_ACT_STATUS_DEATH )
	work->npc.action.life = 0 ;
    if ( work->npc.action.life < 0 )
	work->npc.action.life = 0 ;
    if ( work->vital_m9 < 0 )
	work->vital_m9 = 0 ;

    if ( !work->npc.action.life || !work->vital_m9 )
    {
	/* ゲームオーバ処理は1回だけ */
	if ( !VMPS_Flag( VMPS_F_GAMEOVER ) )
	{
	    /* ヴァンプを殺した */
	    if ( work->npc.action.life == 0 )
 		if ( ++GM_KillCount > 30000 )
		    GM_KillCount = 30000 ;

	    /* エマを放す */
	    //VMPS_ReleaseEmma( work ) ;

	    /* ゲームエンド プロックがあれば呼ぶ  死んだ瞬間終っていいから 2001.06.08  */
	    if ( work->proc[VMPS_P_GAMEEND] )
		GCL_ExecProc( work->proc[VMPS_P_GAMEEND], NULL ) ;

	    /* ヘッドマークは消す */
	    NPC_CallHeadMark( &work->npc, HMK2_TYPE_KILL ) ;
	    if ( pad )
		NPC_SetModeFromPad( &work->npc,
				    VMPS_ActDamage, work->npc.base_mar,
				    pad, pad ) ;
	}
	VMPS_SetFlag( VMPS_F_GAMEOVER ) ;
	return 0 ;
    }
    return 1 ;
}


/*

  ダメージ

*/
int VMPS_DamageProccess( Work *work, int pad )
{
    /* エマに 解かれないようにするため */
    if ( work->capture.capture )
	work->capture.capture->flag |= CAPTURE_HANG ;

    /* ポーズ変更 */
    work->aim_pose = (work->aim_pose + 1) & 3 ;
    //work->aim_pose = irnd() & 3 ;

    return VMPS_GameOverCheck( work, pad ) ;
}

int VMPS_DamageVitality( Work *work, int damage, int pad )
{
#if DEBUG_MODE
    if ( damage )
	printf( "Vamp: Woops, I've got a damage. %d\n", damage ) ;
#endif

    /* ライフをダメージ分だけ減らす */
    work->npc.action.life -= damage ;
    return VMPS_DamageProccess( work, pad ) ;
}

int VMPS_DamageVitalityM9( Work *work, int damage, int pad )
{
#if DEBUG_MODE
    if ( damage )
	printf( "Vamp: Woops, I've got a damage by M9. %d\n", damage ) ;
#endif

    /* M9ライフをダメージ分だけ減らす */
    work->vital_m9 -= damage ;
    return VMPS_DamageProccess( work, pad ) ;
}


/*

  aimに向かって位置をframesのフレームで補間する

*/
void VMPS_SetAdjustPosition( Work *work, FVECTOR *aim, int flames )
{
    if ( work->pos_adjust.vw <= 0.0f )
    {
	_sceVu0SubVector( &work->pos_adjust, aim, &work->control.mov ) ;
	work->pos_adjust.vw = (float)flames ;
	_sceVu0ScaleVector( &work->pos_adjust, &work->pos_adjust, 1.0f/work->pos_adjust.vw ) ;
    }
}


/*

  首締めた状態にする

*/
void VMPS_CaptureEmma( Work *work )
{
    NPCWORK *npc = &work->npc ;
    CAPTURE_TARGET *emma ;

    emma = EMA_CommandGetCaptureTarget() ;
    npc->target.capture->capture = emma ;
    emma->capture = npc->target.capture ;
}

void VMPS_ReleaseEmma( Work *work )
{
    NPC_CaptureFlagClear( &work->npc ) ;
}

