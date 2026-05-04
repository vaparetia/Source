//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	watcher.c
	見張り兵

	1999/07/07 Y.Korekado
	$Id: watcher.c,v 1.2 2002/12/26 11:04:36 Yoshizawa1 Exp $
	
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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"korekado/conv/korekado.x"
#include	"korekado/enemy/enemy.h"

#include	"watcher.h"
#include	"wcprepro.c"
#include	"wcaction.c"
#include	"wcthink.c"
#include	"wcgetres.c"

void	ENE_WatcherStageStartModeSneak( ENETHINK	*entk );

static void WatcherMain( Work *work )
{
	if ( GM_VRStatus & (GM_VR_IDLE) ) return ;
	if ( GM_VRStatus & GM_VR_CLEAR ) {
		ENE_ActStatusCheck ( &work->enethink ) ;
		work->action.old_status = work->action.status ;
		return ;
	}

	/* 前処理 */
	PreProcess( work ) ;
	/* スーパーアンリアルでも個別メッセージだけはうけとっておく */
	if ( work->enethink.status & ENE_STATUS_EVER_UNREAL ) return ;
	/* 思考処理 */
	Think( work ) ;
	/* 行動処理 */
	Action( work ) ;
}

static void Act( Work *work )
{
	ENE_GM_Act( &work->enethink ) ;

	ENE_PreProcess( &work->enethink ) ;
	WatcherMain( work ) ;
	ENE_AftProcess( &work->enethink ) ;
}

static void Die( Work *work )
{
	ENE_FreeResources( &(work->enethink) ) ;
}
/*--- ------------------------------------------------------------*/
/*	
	シナリオオプション使用状況
	s:ステータス r:ルート n:ノード d:ディフェンス位置
*/
static int GetResources( Work *work, int name )
{
	ENETHINK	*entk ;
	CONTROL		*ctrl ;
	ACTION		*act ;
	OBJECT		*body ;
	char		*d_pos_opt ;

	entk = &work->enethink ;
	ctrl = &work->control ;
	act = &work->action ;
	body = &work->body ;

	if ( WatcherGetResources( work, name ) < 0 ) return -1 ;

	/***** 見張り兵固有処理 *****/
	AT_SetType( act, ENE_TYPE_WATCHER ) ;

	/* ウォークマン兵用 */
	if ( (entk->act->bgm_track = GCL_GetOptionValue( 'b', -1 )) < 0  ) {
		entk->act->bgm_track = 0 ;
	} else {
#ifdef KP_WINDOWS		
		//-------------------------------------------------
		//	ＢＧＭ＿ＳＥの再生トラックを0x10以降に全て面した為
		
		if(entk->act->bgm_track < 0x10){
			entk->act->bgm_track += 0x10;
		}
#endif
		/* 個別ＢＧＭトラックを初期化する */
		GM_MixConvFader( entk->act->bgm_track, 0x20, 0 ) ;
	}

	/* 守備位置セット */
	d_pos_opt = GCL_GetOption( 'd' ) ;
	if ( d_pos_opt  != NULL ){
		ENE_GCL_GetFV( d_pos_opt, &entk->def_pos ) ;
		entk->def_mapbit = GM_GetHzxGroupID( GM_GetMapID( GCL_GetNextInt( ) ) );
printf("Scen Set def_map = %x \n",entk->def_mapbit );
	} else {
		entk->def_pos = ctrl->mov ;
		entk->def_mapbit = GM_CurrentMap ;
	}

	/* 気づいた時に呼ばれるプロック */
	if ( GCL_GetOption( 'w' ) != NULL ){
		work->watchwork.notice_proc.argc = ENE_GclGetProc( 
			&(work->watchwork.notice_proc.proc), &work->watchwork.notice_proc.argv[0] ) ;
		if ( work->watchwork.notice_proc.argc > (GCL_MAX_ARGS/2) ) return -1 ;
	} else {
		work->watchwork.notice_proc.proc = 0 ;
	}
	entk->character = (void *)&work->watchwork ;

if(0){
	extern void	HZX_Pos2Zone( FVECTOR *, int *, int * ) ;
	int g, z ;

	HZX_Pos2Zone( &entk->def_pos, &g, &z );
	entk->def_mapbit =  GM_GetBit( g ) ;
printf("Set def_map = %x \n",entk->def_mapbit );
}

	/*思考開始場所セット */

	if( GM_AlertMode == ALERT_MODE_ALERT ) {
		ENE_WatcherStartModeAlertWarp( &work->enethink ) ;
	} else if( GM_AlertMode == ALERT_MODE_AVOID ) {
		ENE_WatcherStartModeAvoidWarp( &work->enethink ) ;
	} else {
#if 1
		if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
			ENE_WatcherStageStartModeSneak( &work->enethink ) ;
		} else {
			ENE_WatcherStartModeSneak( &work->enethink ) ;
		}
#else
		if ( COM_StageKind() & ENE_STAGE_NO_NOTICE ) {	/* 反応無し */
			ENE_WatcherStageStartModeSneak( &work->enethink ) ;
		} else {
			ENE_WatcherStartModeSneak( &work->enethink ) ;
		}
#endif
	}

	work->action.CheckDamage = ENE_EnemyDamagePad ;

	/* ステージ情報当てはめ */
	ENE_LoadEneMemory( entk ) ;

	/* 武器位置最後に初期位置へリセット */
	ENE_ResetWeaponPosition( entk ) ;

//printf("2defnece[%f][%f][%f]\n",work->enethink.def_pos.vx,work->enethink.def_pos.vy,work->enethink.def_pos.vz ) ;

/*待機場所から1フレームだけ見つけちゃう問題*/
/*視界off状態で起動*/
	work->action.status |= ACT_STATUS_EYE_CLOSE ;

	return (0);
}

void *NewWatcher( name, where )
int	name ;
int	where ;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), ENEMY_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, name ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
