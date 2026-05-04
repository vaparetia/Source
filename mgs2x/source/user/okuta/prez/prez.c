//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    prez.c
    大統領
    2001/04/23 Masafumi Okuta
    $Id: prez.c,v 1.1.1.3 2002/11/19 11:48:08 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <eekernel.h>
#include <stdlib.h>
#include <stdio.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"../conv/algfunc.h"
#include	"../conv/maoutil.h"
#include	"../../korekado/npc/npc.h"
#include	"../../korekado/action/action.h"
#include	"prez.h"
#ifdef DEBUG_MODE
#include	"debugmenu.h"	
#endif

static Work* _pworkPrez = NULL;

/*----------------------------------------------------------------*/
#include	"prez.def"
#include	"prezdata.c"
#include	"prezparty.c"
#include	"prezsound.c"
#include	"preznikita.c"
#include	"preznoise.c"
#include	"prezroute.c"
#include	"prezatra.c"
#include	"prezpanic.c"
#include	"prezpre.c"
#include	"prezact.c"
#include	"prezthk.c"
#include	"prezinit.c"
#include	"prezdbg.c"
#include	"prezarea.c"
/*----------------------------------------------------------------*/
// メイン処理
static void PrezMain( Work *work )
{
    // 前処理 
    PreProcess( work );
    // 思考処理 
    Think( work );
    // 行動処理 
    Action( work );
    // 後処理 
    AfterProcess( work );
}

// 動作関数
static	void	Act( work )
Work*	work;
{
    NPCWORK*	npc;

    npc = &work->npc;

    GM_CurrentMap = work->map;
    work->nDifficult = GM_GameLevel;

    // ＮＰＣシステム系コントロール処理
    NPC_ActControl( npc );

    // ＮＰＣ前処理 
    NPC_PreProcess( npc );

    // メイン処理
    PrezMain( work );

    // ＮＰＣ後処理 
    NPC_AfterProcess( npc );
}
// 破棄関数
static	void	Die( work )
Work*	work;
{
    NPC_FreeResources( &work->npc );
    GM_RemoveGageSet( &work->gage );

    _pworkPrez = NULL;
}
// リソース初期化
static	int	GetResources( work, name, where )
Work*	work;
int	name;
int	where;
{
    NPCWORK*	npc;
    int		nInitLife;
    int		start_yrot;
    FVECTOR	start_pos;
    
    npc = &work->npc;

    work->name = name;
    work->map  = where;

    // NPC初期化 
    NPC_InitNPC( npc, work, &work->body, &work->control, work->lights );

    // コントロールの初期化 
    NPC_InitControl( npc, name );

    // オブジェクトの初期化 
    NPC_InitObject( npc, MODEL_NAME );

    // マルチウェイトモデルの初期化 
    NPC_InitMWObject( npc, MW_MODEL_NAME, 20000 );

    // モーションの初期化 
    NPC_InitMotion( npc, BASE_MOTION, DAMAGE_MOTION, CAPTURE_MOTION, DRAG_MOTION );

    // ターゲットの初期化 
    NPC_InitDefenceTarget( npc, &work->deftrg, ENEMY_SIDE, SampleTrgSize, SampleDragShift, &DG_ZeroVector );
    NPC_InitDefenceChildTarget( npc, work->def_child,	ENEMY_SIDE, Cheild_Target_Size, 
				Cheild_Target_Shift, Child_Level_Num, Cheild_Target_Connect );
    NPC_InitDefenceCapture( npc, &work->capture, &work->control, &work->body );
    // 首締めシフト座標セット 
    NPC_SetHangShift( npc, &HangShift );

    // ナビシステム初期化 
    NPC_InitNavi( npc, &work->control, &work->navigate, &work->navitrg );
    // ルートナビ
    NPC_InitRouteNavi( npc, &work->rnavi, 0);

    // 初期位置、方向、モーションのセット 
#if 1
    {
	char*	opt;
	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){ // 初期位置
	    ENE_GCL_GetFV( opt, &start_pos );
	} else {
	    start_pos = GM_PlayerPosition;
	    start_pos.vz -= 1500;
	}
	if ( ( opt = GCL_GetOption( 'y' ) ) != NULL ){ // 初期Y回転角度
	    start_yrot = GCL_GetNextInt();	
	}else{
	    start_yrot = 0;
	}
    }
#else
    start_pos = work->rnavi.nodes[0];
#endif
    npc->ctrl->mov     = start_pos;
    npc->ctrl->rot.vy  = start_yrot;
    npc->ctrl->turn.vy = start_yrot;
    
    NPC_InitPose( npc, &start_pos, 1024, BASE_MOTION, 0 );

    // レーダー情報初期化 
    NPC_InitRader( npc, &work->rctrl, &npc->ctrl->mov, 
		   2000.0f, -2000.0f, 512, 4000, RADAR_COLOR_BLUE );

    // ヘッドマークコントロール初期化 
    NPC_InitHeadMark( work, npc, &work->headmark, &work->body, &work->control, &work->deftrg );

    // 体力取得
    work->nMaxLife = GCL_GetOptionValue( 'h', PREZ_LIFE );
    nInitLife 	   = GCL_GetOptionValue( 'm', work->nMaxLife );
    // パラメータセット 
    NPC_SetActionParam( &npc->action, work->nMaxLife, PREZ_FAINT, PREZ_FAINT_COUNT, PREZ_SLEEP_COUNT, PREZ_DOWN_DAM  );

    // ライフゲージ（各キャラクターが管理する) 
    GM_InitGageSet( &work->gage, "PREZ", 16, 56, 3, work->nMaxLife,
		    nInitLife, 0, 30, GM_GAGE_LEVEL_NPC_LIFE );
    GM_SetGageColorType( &work->gage, GM_GAGE_COLOR_TYPE_NPC_LIFE );
    GM_AppendGageSet( &work->gage );
    GM_VisibleGage( &work->gage );

    // ワーク初期化
    PREZ_WorkInit( work );

    // 思考処理初期化 */
    StartThink( work );

    _pworkPrez = work;

    return 0;
}

// シナリオ呼出
void*	NewPrezNikita( name, where )
int	name;
int	where;
{
    Work*	work;

    OPERATOR();
    work = (Work *)GV_NewActor(  GV_ACTOR_USER, sizeof( Work ) );
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die );
	GV_ActorEX( &work->actor );
	if ( GetResources( work, name, where ) < 0 ) {
	    GV_DestroyActor( work );
	    return NULL;
	}
    }
    return work;
}
