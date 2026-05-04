//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    parrot.c
    オウム
    2001/05/12 Masafumi Okuta
    $Id: parrot.c,v 1.1.1.3 2002/11/19 11:48:07 Yoshizawa1 Exp $
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
#include	"../../takabe/other/puppetik.h"
#include	"parrot.h"
#ifdef DEBUG_MODE
#include	"debugmenu.h"	
#endif

/*----------------------------------------------------------------*/

enum {
	SMP_MODE_NORMAL,
	SMP_MODE_BOMB
} ;
/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/
#include	"parrot.def"	// 宣言関連
#include	"pardata.c"	// データ関連
#include	"parparty.c"	// 共用関数関連
#include	"parsound.c"	// 音関連

#include	"parpre.c"
#include	"paract.c"
#include	"parthk.c"
#include	"parinit.c"
#ifdef DEBUG_MODE
#include	"pardbg.c"	// デバック
#endif

//#include	"parroute.c"
//#include	"paratra.c"
//#include	"parpanic.c"
//#include	"pardbg.c"
//#include	"pararea.c"
/*----------------------------------------------------------------*/

static void SampleMain( PARROT *work )
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

static	void	Act( work )
PARROT*	work;
{
    NPCWORK*	npc;
    npc = &work->npc;

    GM_CurrentMap = work->map;

    // ＮＰＣシステム系コントロール処理
    NPC_ActControl( npc );

    // ＮＰＣ前処理 
    NPC_PreProcess( npc );

    // キャラクターメイン処理
    SampleMain( work );

    // ＮＰＣ後処理 
    NPC_AfterProcess( npc );

    PAR_UpdateCageRot( work);

    work->vecCageRot.vy = -512;
    DG_SetPos2( (FVECTOR*)&work->matCage.m[3][0], &work->vecCageRot );	
    DG_PutObjs( work->objCage.objs );
}

static	void	Die( work )
PARROT*	work;
{
    GM_FreeObject( &work->objCage);	// かご    

    NPC_FreeResources( &work->npc );    // NPC

//  GM_RemoveGageSet( &work->gage );
}

/*----------------------------------------------------------------*/
static	int	GetResources( work, name, where )
PARROT*	work;
int	name;
int	where;
{
    NPCWORK*	npc;
    int		start_yrot;
    FVECTOR	start_pos;
	
    npc = &work->npc;
    // シナリオから値を取得 
    work->map = where; // 現在のマップ

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
    work->deftrg.class &= ~TARGET_CHILD;	// 子ターゲットオフ
    work->deftrg.class |= TARGET_LOCKON;	// ロックオン可能
    // ロックオン設定
    NPC_InitHomingTarget( npc, &work->homing, &work->body, &work->control, &work->deftrg);
    work->homing.status |= HOMING_SKIP;
    // コールバック設定 : フォーチュンバリア
    {
	extern void	GM_SetTargetCallBack( TARGET*, TARGET_CALLBACK, void* );
	GM_SetTargetCallBack( &work->deftrg, PAR_TargetCallback, work);
    }
    // 弾検知用ダミーターゲット 
    {
	FVECTOR vecSize, vecShift;
	vecSize.vx = 2000.f;
	vecSize.vy = 2000.f;
	vecSize.vz = 2000.f;
	vecSize.vw = 1.f;
	vecShift = DG_ZeroVector;
	
	GM_SetTarget( &work->dummytrg, TARGET_DEFENSE|TARGET_SEEK|TARGET_THROUGH, work->map, ENEMY_SIDE, &vecSize, &vecShift ) ;
	GM_SetTargetWeaponType( &work->dummytrg, 0 ) ;
	GM_PutTarget( &work->dummytrg ) ;
//NewTargetView( &work->dummytrg, 34, 184, 200 ) ;
    }
#if 0
    NPC_InitDefenceChildTarget( npc, work->def_child,	ENEMY_SIDE, Cheild_Target_Size, 
				Cheild_Target_Shift, Child_Level_Num, Cheild_Target_Connect );
#endif
    NPC_InitDefenceCapture( npc, &work->capture, &work->control, &work->body );

    // 首締めシフト座標セット 
    NPC_SetHangShift( npc, &HangShift );
    // ナビシステム初期化 
    NPC_InitNavi( npc, &work->control, &work->navigate, &work->navitrg );

    // 初期位置、方向、モーションのセット 
    {
	char*	opt;
	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){ // 初期位置
	    ENE_GCL_GetFV( opt, &start_pos );
	} else {
	    start_pos = GM_PlayerPosition;
	    start_pos.vz -= 1500;
	}
	if ( ( opt = GCL_GetOption( 'y' ) ) != NULL ){ // 初期Y回転角度
	    start_yrot = (GCL_GetNextInt() - 512) & 4095;	// かごがななめになった
	}else{
	    start_yrot = 0;
	}
    }

    { // 初期位置設定
	work->vecInitPos = start_pos;
	work->vecInitPos.vy += 166.f;
	work->vecInitPos.vz -= 20.f;
	NPC_InitPose( npc, &work->vecInitPos, start_yrot, BASE_MOTION, 0 );
    }

    // レーダー情報初期化 
    NPC_InitRader( npc, &work->rctrl, &npc->ctrl->mov, 
		   2000.0f, -2000.0f, 512, 4000, RADAR_COLOR_BLUE );
    // ヘッドマークコントロール初期化 
    NPC_InitHeadMark( work, npc, &work->headmark, &work->body, &work->control, &work->deftrg );

    work->nMaxLife = GCL_GetOptionValue( 'h', PAR_LIFE );
    // パラメータセット 
    NPC_SetActionParam( &npc->action, work->nMaxLife, PAR_FAINT, PAR_FAINT_COUNT, PAR_SLEEP_COUNT, PAR_DOWN_DAM  );
#if 0
    // ライフゲージ（各キャラクターが管理する) 
    GM_InitGageSet( &work->gage, "PARROT", 16, 196, 3, work->nMaxLife,
		    work->nMaxLife, 0, 30, GM_GAGE_LEVEL_NPC_LIFE );
    work->gage.text_len = 23;	// 見栄えで、えー感じに設定 
//  GM_SetGageColor( &work->gage, 0, 0, 0, 31, 63, 192, 31, 127, 255, 255, 0, 0 );
    GM_SetGageColorType( &work->gage, GM_GAGE_COLOR_TYPE_NPC_LIFE );
    GM_AppendGageSet( &work->gage );
    GM_VisibleGage( &work->gage );
#endif
    
    // 心音ならさない
    NPC_SetHeartBeat( npc, 0);

    /* フェイスアニメーションは それぞれのキャラが独自で制御して下さい。
       但し、NPCのシステムが npc->action.face_anime に 
       NPC_FCANIM_EYE_CLOSE や NPC_FCANIM_SUFFER に変更した場合
       対応するようにしてください。 */
#if 0
    if ( NPC_InitFaceAnime( npc, npc->body->evmobj, SMP_FACE_FAR, EMMA_INF_NAME ) < 0 ) {
	return -1;
    }
#endif
    // とりかご
    work->nInterpCage = 8;
    work->vecCageRot = DG_ZeroSVector;
    work->vecCageRot.vy = 512;
    work->vecCageRotAim = DG_ZeroSVector;
    work->vecCageRotAim.vy = 512;
    _sceVu0UnitMatrix( &work->matCage );
    _sceVu0CopyVector( (FVECTOR*)&work->matCage.m[3][0], &start_pos); // かごのマトリクス設定

#if 0
    GM_InitObject( &work->objCage, CAGE_MODEL_NAME, 
		   (DG_FLAG_TRANS|DG_FLAG_SEMITRANS|DG_FLAG_SHADE|DG_FLAG_ONEPIECE));
#else
    GM_InitObject( &work->objCage, CAGE_MODEL_NAME, 
		   (DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_ONEPIECE));
#endif
    DG_SetPos2( (FVECTOR*)&work->matCage.m[3][0], &work->vecCageRot );	
    DG_PutObjs( work->objCage.objs );
//    DG_SetPos( &work->matCage );	
//    DG_PutObjs( work->objCage.objs );

    // ワーク初期化
    PAR_WorkInit( work );

    // 思考処理初期化 */
    StartThink( work );

    work->mode = SMP_MODE_NORMAL;
#ifdef DEBUG_MODE
PAR_CallDbgSound();
PAR_CallDbgStrmMode();	
#endif
    return 0;
}

// シナリオ呼出
void*	NewParrot2( name, where )
int	name;
int	where;
{
    PARROT*	work;

    OPERATOR();
    work = (PARROT *)GV_NewActor(  GV_ACTOR_USER, sizeof( PARROT ) );
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

