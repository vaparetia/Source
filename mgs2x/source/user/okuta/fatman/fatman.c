//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    fatman.c
    ファットマン
    2001/03/12 Masafumi Okuta
    $Id: fatman.c,v 1.1.1.3 2002/11/19 11:47:59 Yoshizawa1 Exp $
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
#include	"../../skoba/weapon_old/matsu.h"
#include	"fatman.h"

#ifdef DEBUG_MODE
#include	"debugmenu.h"	
#endif

// 外部参照用
static Work* _FatWork = NULL;

enum{ // モーション
// 固定
FAT_MOT_SYSFIX01	 = NPC_BASE_MOT_END,
// 移動系
FAT_MOT_STAND,			// 立ち 
FAT_MOT_RUN_START,		// 走り始め
FAT_MOT_RUN,			// 走り			5
FAT_MOT_BACK_RUN,		// 後ろ走り
FAT_MOT_TURN,			// 前からターン
FAT_MOT_TURN_BACK,		// 後ろからターン
FAT_MOT_REVERSE,		// 反転			
FAT_MOT_RUN_TURN,		// 走りターン 後ろへ	10
FAT_MOT_TURN_TO_FRONT,		// 走りターン 前へ
FAT_MOT_TURN_L,			// 左ターン			
FAT_MOT_TURN_R,			// 右ターン
FAT_MOT_STOP,			// 停止			
// 攻撃系
FAT_MOT_FIRE,			// 発射			15
FAT_MOT_FIRE_P,			// 銃構え
FAT_MOT_FIRE_END,		// 単発発射
FAT_MOT_RUN_FIRE,		// 走り銃構え
FAT_MOT_BACK_FIRE,		// 後ろ走り銃構え
FAT_MOT_BODYATTACK,		// 体当り		20
FAT_MOT_RELOAD,			// リロード
FAT_MOT_RUN_RELOAD,		// 走りながらリロード
// 爆弾設置
FAT_MOT_BOMB_SET,		// 爆弾設置
FAT_MOT_BOMB_STAND_SET,		// 爆弾設置:立ち
FAT_MOT_BOMB_DOWN_SET,		// 爆弾設置:座り	25
FAT_MOT_BOMB_STRECH_SET,	// 爆弾設置:伸び
FAT_MOT_BOMB_RUNSET,		// 爆弾設置:走り
FAT_MOT_BOMB_RUNSET_L,		// 爆弾設置:走り左回転
FAT_MOT_BOMB_RUNSET_R,		// 爆弾設置:走り右回転
// 特殊
FAT_MOT_GUN_PHASE_START,	// 銃撃フェーズ開始	30
FAT_MOT_BOMB_PHASE_START,	// 爆弾フェーズ開始	
FAT_MOT_BOMB_COUNT_START,	// 爆弾起動
FAT_MOT_BOMB_COUNT_START_S,	// 爆弾起動1
FAT_MOT_BOMB_COUNT_START_P,	// 爆弾起動2
FAT_MOT_BOMB_COUNT_START_E,	// 爆弾起動3
FAT_MOT_ROLLER_REPAIR,		// ローラーブレード修理
FAT_MOT_DANCE_1,		// 余興１
FAT_MOT_DANCE_2,		// 余興２
// ダメージ
FAT_MOT_BLAST_DAM_1,		// ブラストスーツダメージ１
FAT_MOT_BLAST_DAM_2,		// ブラストスーツダメージ２	40
FAT_MOT_BLAST_DAM_3,		// ブラストスーツダメージ３：うつぶせ転倒 
FAT_MOT_DAM_BOMB,		// うつぶせ転倒
FAT_MOT_DAM_GUN,		// ダメージ１
FAT_MOT_DAM_GUN2,		// ダメージ２
FAT_MOT_DAM_STUN,		// うつぶせ転倒
FAT_MOT_DAM_RUN_STUN,		// うつぶせ転倒		
FAT_MOT_ROLLER_CRASH,		// うつぶせ転倒
FAT_MOT_RUN_ROLLER_CRASH,	// あおむけ転倒
FAT_MOT_RISE_1,			// うつぶせ起き上がり 倒れ->首振り
FAT_MOT_RISE_2,			// うつぶせ起き上がり 首振り		50
FAT_MOT_RISE_3,			// うつぶせ起き上がり 起き上がり	
FAT_MOT_RISE_DEAD,		// うつぶせ起き上がり しにかけ
FAT_MOT_BACK_RISE,		// あおむけ起き上がり
FAT_MOT_DOWN,			// あおむけ
FAT_MOT_DOWN_FRONT,		// うつぶせ
FAT_MOT_HANG,			// 掴み回避		
FAT_MOT_CORPSE,			// 死体			
FAT_MOT_CORPSE_IDLE,		// いきたえだえ

FAT_MOT_FACEFIX,		// デバック用

FAT_MOT_MAX,			// 最大

// チェック用
FAT_MOT_CHK_RUN_START = FAT_MOT_RUN_START,
FAT_MOT_CHK_RUN_END   = FAT_MOT_TURN_R,

};


// 関数の宣言ファイル登録
#include	"fatman.def"
/*----------------------------------------------------------------*/

#ifdef DEBUG_MODE
#include	"fatdbgmenu.c"	// デバックモード用
#endif
#include	"fatdata.c"	// データ
#include	"fatutil.c"	// ユーティリティ
#include	"fatsound.c"	// サウンド
#include	"fatarea.c"	// エリア
#include	"fatzone.c"	// ゾーン選択
#include	"fatintrude.c"	// 思考:対イントルード
#include	"fatextern.c"	// 外部通信用
#include	"fatact.c"	// 動作
#include	"fatnpc.c"	// NPCシステムからローカライズした関数
#include 	"fatparty.c"	// 共用関数
#include	"fateye.c"	// 情報チェック
#include	"fatc4.c"	// C4爆弾
#include	"fatmove.c"	// 移動ルーチン
#include 	"fatattack.c"	// 攻撃
#include 	"fatdamage.c"	// ダメージ
#include 	"fatbomb.c"	// 爆弾フェーズ
#include 	"fatdemine.c"	// 解体フェーズ
#include 	"fatshoot.c"	// 銃撃フェーズ
#include	"fatpre.c"	// 前処理
#include	"fatthink.c"	// 思考
#include	"fatinit.c"	// 初期化
#include	"fatdebug.c"	// デバック
/*----------------------------------------------------------------*/
// メイン処理
static void FatmanMain( Work *work )
{
    /* 前処理 */
    PreProcess( work ) ;

    /* 思考処理 */
    Think( work ) ;

    /* 行動処理 */
    Action( work ) ;

    /* 後処理 */
    AfterProcess( work );
}
// NPC_ActControlのカスタマイズ版
static void FAT_ActControl( NPCWORK *npc )
{
    Work*	work;
    OBJECT*	body;
    CONTROL*	ctrl;

    work = (Work*)npc->character;
    body = npc->body ;
    ctrl = npc->ctrl ;
	
    npc->old_body_height = body->height ;
    GM_ActMotion( body ) ;

    // 前の座標と移動量取得: カスタマイズ部分はここ----------
    _sceVu0CopyVector( &work->vecPrePos, &npc->ctrl->mov);
    _sceVu0CopyVector( &work->vecStep, &work->npc.ctrl->step);
    if ( !(npc->action.status & NPC_ACT_STATUS_DAMAGE) ) _sceVu0CopyVector( &work->vecPreDmgStep, &work->vecStep);
    work->nMoveDist = _FVecLen3( &work->vecStep );
    // ------------------------------------------------------

    ctrl->height = body->height ;
    GM_ActControl( ctrl ) ;
    
    // IK
    if ( npc->action.status & NPC_ACT_STATUS_STAND ){ // 立っている時だけ
	Leg_IKcalc( ctrl, body);
    }
    GM_ActObject2( body );
    NPC_IK_Control( npc ) ;
    
    DG_GetLightMatrix( &ctrl->mov, npc->lights );

    /* ＳＥ変換用 */
    if ( npc->se_id >= 0 ) {
	MT_SetMotionSeTable( body->m_ctrl, GM_CurrentMap, npc->se_id, 
			     ( npc->ctrl->flr_atrs[ 0 ] & 0xf0000000 ) >> 28, 0 ) ;
    }
}
// 動作関数
static	void	Act( work )
Work		*work ;
{
    NPCWORK	*npc ;

    npc = &work->npc ;

    GM_CurrentMap = work->map;

    // NPCシステム系コントロール処理カスタマイズ版 
    FAT_ActControl( npc );

    // ＮＰＣ前処理 
    NPC_PreProcess( npc );

    // キャラクターメイン処理
    FatmanMain( work );

    // ＮＰＣ後処理 
    NPC_AfterProcess( npc );

    // 表示／非表示
    if ( work->body.flag & DG_FLAG_INVISIBLE ){
	if ( work->body.objs   != NULL && !(npc->body->objs->flag & DG_FLAG_INVISIBLE)){   
	    DG_InvisibleObjs( work->body.objs);
	}
	if ( work->body.evmobj != NULL && !(npc->body->evmobj->flag & DG_FLAG_INVISIBLE) ){
	    SET_FLAG( work->body.evmobj->flag, DG_EVMOBJ_INVISIBLE);
	}
	if ( work->gun.objs   != NULL )    DG_InvisibleObjs( work->gun.objs);
    }else{
	if ( work->gun.objs   != NULL )    DG_VisibleObjs( work->gun.objs);
    }
#if 1
    GM_MoveTarget( &work->pushtrg, &npc->ctrl->mov ) ;
#endif
}

// 破棄関数
static	void	Die( work )
Work		*work ;
{
    GM_FreeObject( &work->gun );
#if 0
    GV_Free( work->pbBezierPass); // 通過データ
#endif
    NPC_FreeResources( &work->npc );

    GM_FreeTarget( &work->pushtrg );    

    GM_RemoveGageSet( &work->gage );


    _FatWork = NULL;
}

// リソース設定:コンストラクタみたいなもの
static int GetResources(Work*	work,	// 設定するワークへのポインタ
			int	name,	// 名前
			int	where)	// 位置
{
    int 	i;
    NPCWORK*	npc;		// NPCワークへのポインタ
    FVECTOR	start_pos;	// 始点
    char*	opt;	

    _FatWork = work;

    npc = &work->npc;

    work->name = name;
    work->map  = where;

    if ( GCL_GetOption( 'c' ) ){ // パラメータ
	work->nBodyDurableMax 		= ( GCL_NextStr() != NULL ) ? GCL_GetNextInt() : 3; 	// 耐久値
	work->nBodyDurableRecover 	= ( GCL_NextStr() != NULL ) ? GCL_GetNextInt() : COUNT_VMODE(60); // 耐久値
	work->nVitality			= ( GCL_NextStr() != NULL ) ? GCL_GetNextInt() : 256;	// 体力
	work->fPowerRate		= (float)(( GCL_NextStr() != NULL ) ? GCL_GetNextInt() : 100) * 0.01f;	// 攻撃力割合
    }else{
	work->nBodyDurableMax 		= 3; 	// 耐久値
	work->nBodyDurableRecover	= COUNT_VMODE(60);	// 回復
	work->nVitality			= 256;	// 体力
	work->fPowerRate		= 1.f;	// 攻撃力割合
    }
#ifdef DEBUG_MODE
    printf("FATMAN-DURABLE-MAX       	= %d\n", work->nBodyDurableMax);
    printf("FATMAN-DURABLE-RECOVER   	= %d\n", work->nBodyDurableRecover);
    printf("FATMAN-VITALITY  		= %d\n", work->nVitality);
    printf("FATMAN-PowerRate 		= %f\n", work->fPowerRate);
#endif

    if ( GCL_GetOption( 'h' ) ){ // パラメータ
	int nAddRate, nCurr;
	for ( i = 0, nAddRate = 0; i < FAT_C4_LEVEL; i++ ){
	    nCurr = GCL_GetNextInt();
	    nAddRate += nCurr;
	    work->fBombDifficlutRate[i] = (float)(nAddRate) * 0.01f;
#ifdef DEBUG_MODE
	    printf("%f : %d %d ", work->fBombDifficlutRate[i], nAddRate, nCurr);
#endif
	}
    }else{
	for ( i = 0; i < FAT_C4_LEVEL; i++ ){
	    work->fBombDifficlutRate[i] = 0.25f;
#ifdef DEBUG_MODE
	    printf("%f : ", work->fBombDifficlutRate[i]);
#endif
	}
    }	
#ifdef DEBUG_MODE
    printf("\n");
#endif

    // NPC初期化
    NPC_InitNPC( npc, work, &work->body, &work->control, work->lights);

    // コントロールの初期化
    NPC_InitControl( npc, name);

    // モデル初期化
    NPC_InitObject( npc, MODEL_NAME ) ;

    // マルチウェイトモデルの初期化 
    NPC_InitMWObject( npc, MW_MODEL_NAME, 400000 ) ; // 4000

    // モーションの初期化 
    NPC_InitMotion( npc, BASE_MOTION, DAMAGE_MOTION, CAPTURE_MOTION, DRAG_MOTION ) ;

    // 武器モデル登録
    WeaponEfInitObject( &work->wctrl, &work->gun, &work->body, HUMAN21_MIGI_TE, WP_Glk, GLK_MODEL);

    // カートリッジ制御登録
    work->nCartCtrl = 0;
    work->nCartStep = CART_STEP_RELEASE;
    NewCartridgeControl( &work->nCartCtrl, &work->body, &work->gun, 5);
    work->nCartCtrl = 1;

    // ターゲットの初期化 
    NPC_InitDefenceTarget( npc, &work->deftrg, ENEMY_SIDE, SampleTrgSize, SampleDragShift, &DG_ZeroVector ) ;
    GM_SetPowerTarget( &work->deftrg, &work->powDefTrg, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( &work->deftrg, ChildTargCallBack, work );
    NPC_InitDefenceChildTarget( npc, work->def_child, ENEMY_SIDE, Cheild_Target_Size, 
				Cheild_Target_Shift, Child_Level_Num, Cheild_Target_Connect ) ;
    for ( i = 0; i < CHILD_TARGET_NUM; i++){
	GM_SetPowerTarget( &work->def_child[i], &work->powDefChild[i], POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
	GM_SetTargetCallBack( &work->def_child[i], ChildTargCallBack, work );
    }

    NPC_InitDefenceCapture( npc, &work->capture, &work->control, &work->body );

    // ホーミング : 低レベルのときは頭
    NPC_InitHomingTarget( npc, &work->homing, &work->body, &work->control, &work->deftrg);

    FAT_InitPushTarget( work);

    // ナビシステム初期化 
    NPC_InitNavi( npc, &work->control, &work->navigate, &work->navitrg ) ;

    // 初期位置、方向、モーションのセット
    if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
	ENE_GCL_GetFV( opt, &start_pos ) ;
    } else {
	start_pos = GM_PlayerPosition ;
	start_pos.vz -= 1500 ;
    }
    work->svecAdjust = DG_ZeroSVector;
    NPC_InitPose( npc, &start_pos, 1024, BASE_MOTION, 0 ) ;

    // レーダー情報初期化 
    NPC_InitRader( npc, &work->rctrl, &npc->ctrl->mov, 
		   2000.0f, -2000.0f, 512, 4000, RADAR_COLOR_YELOW ) ;

    // ヘッドマークコントロール初期化 
    NPC_InitHeadMark( work, npc, &work->nHeadmark, &work->body, &work->control, &work->deftrg ) ;

    // パラメータセット 
    NPC_SetActionParam( &npc->action, work->nVitality, SMP_FAINT, SMP_FAINT_COUNT,
				SMP_SLEEP_COUNT, SMP_DOWN_DAM  ) ;
    work->m9_faint = work->nVitality;	// M9気絶耐久

    // ライフゲージ（各キャラクターが管理する) 
    GM_InitGageSet( &work->gage, "FATMAN", 16, 180, 3, work->nVitality, 
		       work->nVitality, 0, 30, GM_GAGE_LEVEL_ENEMY ) ;
    GM_SetGageColorType( &work->gage, GM_GAGE_COLOR_TYPE_ENEMY_LIFE );
    GM_InitGageM9( &work->gage, work->nVitality, work->nVitality, 0, 30 ) ;
    GM_AppendGageSet( &work->gage ) ;
    GM_VisibleGage( &work->gage ) ;

    // 顔アニメ
    FAT_InitFaceAnime( work );

    // ファットマンステータスワーク初期化
    FAT_InitStatusWork( work );

    // ファットマン周辺情報ワーク初期化
    FAT_InitInfoWork( work );

    // ファットマン爆弾ワーク初期化
    FAT_InitBombWork( work );

    // ファットマン音関連ワーク初期化
    FAT_InitSoundWork( work );

    // ファットマンイベント関連ワーク初期化
    FAT_InitEventWork( work );

    // 位置データワークの初期化
    FAT_InitDataWork( work );

    // デバック
    FAT_InitDebugWork( work );

    {    // 影生成
	extern void* NewShadow( void *,void *,void *,FMATRIX *,int * );
	work->nShadowFlag = 1;
	GV_SetActorChild( work, NewShadow( &work->body.objs->objs[HUMAN21_MIGI_KAKATO], 
					   &work->body.objs->objs[HUMAN21_HIDARI_KAKATO], 
					   npc->ctrl, work->lights, &work->nShadowFlag));
    }
    // ローラーブレード跡 by Kunibe
    NewFatmanScratch( &work->control, &work->bSpecialEffect );

    // 思考処理初期化 
    StartThink( work );

    // ファットマンルートとゾーン通過情報テーブルの生成
    FATUTL_BezierPassCheck(work);

    // 初回メッセージチェック
    CheckMessage( work );

    work->control.seg_flag &= ~HZX_TYPE_ENEMY;
    work->control.hzx_check_type |= HZX_CHK_CYPHER;

#ifdef DEBUG_MODE
    GV_SetActorChild( work, NewFatC4PointView( work, name, where )); // デバック用
printf("size of Fatman work =  %d (byte)\n", sizeof(Work) );
#endif

    return 0;
}

// シナリオ呼出関数
void*	NewFatman( int 	name,	// 名前
		   int 	where )	// 場所
{
    Work		*work ;

    OPERATOR() ;
    work = (Work *)GV_NewActor(  GV_ACTOR_USER, sizeof( Work ) ) ;
    if ( work != NULL ) {
	GV_SetActor( &( work->actor ), Act, Die ) ;
	GV_ActorEX( &work->actor ) ;
	if ( GetResources( work, name, where ) < 0 ) {
	    GV_DestroyActor( work ) ;
	    return NULL ;
	}
    }

    return work;
}

// ファットマン非表示
int NewCmdFatInvisible( void )
{
    OBJECT* object;

    if ( _FatWork == NULL ) return -1;
#ifdef DEBUG_MODE
printf("Command Fatman Invisible\n");
#endif
    object = &_FatWork->body;
    SET_FLAG( object->flag, DG_FLAG_INVISIBLE);

    return 0;
}

// ファットマン表示
int NewCmdFatVisible( void )
{
    OBJECT* object;

    if ( _FatWork == NULL ) return -1;
#ifdef DEBUG_MODE
printf("Command Fatman Visible\n");
#endif
    object = &_FatWork->body;
    UNSET_FLAG( object->flag, DG_FLAG_INVISIBLE);

    return 0;
}

