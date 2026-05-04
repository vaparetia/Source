//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	sample.c
	NPCサンプル

	2001/02/08 Y.Korekado
	$Id: sample.c,v 1.1.1.3 2002/11/19 11:44:23 Yoshizawa1 Exp $
*/
/*----------------------------------------------------------------
void	*NewNPCSample( name, where )
----------------------------------------------------------------*/
#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"npc.h"

/*----------------------------------------------------------------*/
//#define SMP_FATMAN
#define SMP_EMMA

//#define MODEL_NAME	(6371217)	/* GV_StrCode( "gbs_def" ) */
//#define BASE_MOTION	(108723)	/* GV_StrCode("gbs") */

#ifdef SMP_EMMA
//#define MODEL_NAME	(11752553)	/* GV_StrCode( "ema_def_sh_mt" ) */
#define MODEL_NAME	(828773)	/* GV_StrCode( "hos_femalea_def_mt" ) */

#define MW_MODEL_NAME	(5461097)	/* GV_StrCode( "ema_def_mh_mt" ) */
#define BASE_MOTION	(3351268)	/* GV_StrCode("npcema") */
#define DAMAGE_MOTION	(138590)	/* GV_StrCode("emadam") */
#define CAPTURE_MOTION	(137569)	/* GV_StrCode("emacap") */
#define DRAG_MOTION	(4452007)	/* GV_StrCode("emadrag") */
#endif

#define SMP_FACE_FAR	(107009)	/* GV_StrCode( "ema" ) */

#ifdef SMP_FATMAN
#define MODEL_NAME	(11755189)	/* GV_StrCode( "fat_def_sh_mt" ) */
#define MW_MODEL_NAME	(5463733)	/* GV_StrCode( "fat_def_mh_mt" ) */
#define BASE_MOTION	(3351927)	/* GV_StrCode("npcfat") */
#define DAMAGE_MOTION	(138590)	/* GV_StrCode("emadam") */
#define CAPTURE_MOTION	(137569)	/* GV_StrCode("emacap") */
#define DRAG_MOTION	(4452007)	/* GV_StrCode("emadrag") */
#endif


#define CHILD_TARGET_NUM	(11)
#define SMP_LIFE	(128)
#define SMP_FAINT	(10)
#define SMP_FAINT_COUNT	(60*30)
#define SMP_SLEEP_COUNT	(60*60*1 + 3)
#define SMP_DOWN_DAM	(10)

//#define	SMP_TARGET_SKIP	(1)

/*----------------------------------------------------------------*/
#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world

enum {
	SMP_MODE_NORMAL,
	SMP_MODE_BOMB
} ;
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	OBJECT		body ;
	CONTROL		control ;
	FMATRIX		lights[2] ;

	NPCWORK		npc ;
	NAVIGATE 	navigate ;
	NAVITARGET	navitrg ;
	RADAR_CTRL	rctrl ;

    TARGET			deftrg ;	/* 防御ターゲット */
	CAPTURE_TARGET	capture ;	/* 捕まりターゲット */
    TARGET			pushtrg ;	/* ぶつかりターゲット */
	GM_GageSet			gage ;

    TARGET			def_child[CHILD_TARGET_NUM] ;	/* 子ターゲット*/
    
    int			headmark ;

	/* sampleキャラクター専用 */
	int			think1 ;
	int			think2 ;
	int			think3 ;
	int			count3 ;

	int			mode ;	/* シナリオ実験用 */
	int			bomb_zone ;

	int			pl_dis ;	/* プレイヤーとの距離 */
	int			pl_dir ;	/* プレイヤーへの方向 */

	float		speed ;

	TARGET		attack ;
	POWER_TARGET	power ;
} Work ;

/*----------------------------------------------------------------*/
/* base_mar */
enum {
	SMP_MOT_TUKARE = NPC_BASE_MOT_END,
	SMP_MOT_DASH,

	SMP_MOT_SQUAT ,
	SMP_MOT_IYA_IDLE,
	SMP_MOT_DOWN,
	SMP_MOT_SUMMER,
	SMP_MOT_SLIDING,
	SMP_MOT_ELBOW,
	SMP_MOT_SHOUTEI,
	SMP_MOT_SPIRAL,
	SMP_MOT_SPINKICK_HIGH,
	SMP_MOT_SPINKICK_LOW,

} ;

/* damage_mar */
enum {
	SMP_MOT_DOWN_F,			/* 仰向け */
	SMP_MOT_WAKEUP_F,
	SMP_MOT_WAKEUP_SLOW_F,
	SMP_MOT_DAM_PUNCH_L,
	SMP_MOT_DAM_PUNCH_R,
	SMP_MOT_DAM_KICK,
} ;


static int	Child_Level_Num[] = { 
	0,	/* Level 0 のターゲット数 */
	2,	/* Level 1 のターゲット数 */
	8,	/* Level 2 のターゲット数 */
	1	/* Level 3 のターゲット数 */
} ;

enum {
	SMP_TARGET_CHILD_HEAD,
	SMP_TARGET_CHILD_HART,

	SMP_TARGET_CHILD_ARMR1,
	SMP_TARGET_CHILD_ARMR2,
	SMP_TARGET_CHILD_ARML1,
	SMP_TARGET_CHILD_ARML2,

	SMP_TARGET_CHILD_LEGR1,
	SMP_TARGET_CHILD_LEGR2,
	SMP_TARGET_CHILD_LEGL1,
	SMP_TARGET_CHILD_LEGL2,

	SMP_TARGET_CHILD_BPDY,
} ;

static int	Cheild_Target_Connect[] = { 
	HUMAN21_ATAMA,
	HUMAN21_MUNE,

	HUMAN21_MIGI_UDE1,
	HUMAN21_MIGI_UDE2,
	HUMAN21_HIDARI_UDE1,
	HUMAN21_HIDARI_UDE2,
	HUMAN21_MIGI_ASHI1,
	HUMAN21_MIGI_ASHI2,
	HUMAN21_HIDARI_ASHI1,
	HUMAN21_HIDARI_ASHI2,

	HUMAN21_MUNE
} ;

static FVECTOR	Cheild_Target_Shift[] = { 
	/* level 0 は無し */
	/* level 1 */
	{ 0.0F, 30.0F, 0.0F },		/* 頭 */
	{ 100.0F, 0.0F, 100.0F },	/* 心臓 */
	/* level 2 */
	{ 0.0F, -130.0F, 0.0F },	/* 右腕１ */
	{ 0.0F, -130.0F, 0.0F },	/* 右腕２ */
	{ 0.0F, -130.0F, 0.0F },	/* 左腕１ */
	{ 0.0F, -130.0F, 0.0F },	/* 左腕２ */
	{ -10.0F, -200.0F, -10.0F },/* 右足１ */
	{ -20.0F, -250.0F, -20.0F },/* 右足２ */
	{ -10.0F, -200.0F, -10.0F },/* 左足１ */
	{ -20.0F, -250.0F, -20.0F },/* 左足２ */
	/* level 3 */
	{ 0.0F, 0.0F, 0.0F },		/* 胴体 */
} ;


static FVECTOR	Cheild_Target_Size[] = { 
	/* level 0 は無し */
	/* level 1 */
	{ 96.0F, 130.0F, 96.0F },		/* 頭 */
	{ 50.0F, 50.0F, 50.0F },	/* 心臓 */
	/* level 2 */
	{ 75.0F, 150.0F, 75.0F },	/* 右腕１ */
	{ 75.0F, 150.0F, 75.0F },	/* 右腕２ */
	{ 75.0F, 150.0F, 75.0F },	/* 左腕１ */
	{ 75.0F, 150.0F, 75.0F },	/* 左腕２ */
	{ 85.0F, 200.0F, 120.0F },	/* 右足１ */
	{ 85.0F, 200.0F, 250.0F },	/* 右足２ */
	{ 85.0F, 200.0F, 120.0F },	/* 左足１ */
	{ 85.0F, 200.0F, 250.0F },	/* 左足２ */
	/* level 3 */
	{ 200.0F, 300.0F, 200.0F },	/* 胴体 */
} ;

/* ターゲットサイズ */
static FVECTOR SampleTrgSize[] = {
	{ 400.0F, 1000.0F, 400.0F },	/* 立ち */
	{ 1000.0F, 200.0F, 1000.0F },	/* ダウン */
	{ 700.0F, 650.0F, 700.0F },		/* しゃがみ */
} ;

/* 引き摺りシフト */
static FVECTOR SampleDragShift[] = {
	{ 26.483F, -393.755F, 940.002F },	/* 仰向け頭16 */
	{ 0.0F, -113.487F, 291.057F },		/* 仰向け頭 */
	{ 0.0F, -421.119F, 1192.692F },		/* 仰向け足16 */
	{ 0.0F, -382.115F, 422.438F },		/* 仰向け足 */
	{ 0.0F, -346.867F, 1192.629F },		/* うつぶせ足16 */
	{ 0.0F, -294.030F, 463.501F },		/* うつぶせ足 */
	{ 45.761F, -317.266F, 946.003F },	/* うつ伏せ頭16 */
} ;

static FVECTOR 	HangShift = { 0.0F, 0.0F, 263.0F } ;
/*----------------------------------------------------------------*/
#include	"smppre.c"
#include	"smpact.c"
#include	"smpthink.c"
/*----------------------------------------------------------------*/
static void SampleMain( Work *work )
{
	/* 前処理 */
	PreProcess( work ) ;
	/* 思考処理 */
	Think( work ) ;
	/* 行動処理 */
	Action( work ) ;
	/* 後処理 */
	AfterProcess( work ) ;
}

static	void	Act( work )
Work		*work ;
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	/* ＮＰＣシステム系コントロール処理 */
	NPC_ActControl( npc ) ;

	/* ＮＰＣ前処理 */
	NPC_PreProcess( npc ) ;

	/* キャラクターメイン処理*/
	SampleMain( work ) ;

	/* ＮＰＣ後処理 */
	NPC_AfterProcess( npc ) ;
}

static	void	Die( work )
Work		*work ;
{
	NPC_FreeResources( &work->npc ) ;
	GM_RemoveGageSet( &work->gage ) ;
}
/*----------------------------------------------------------------*/
static	int	GetResources( work, name, where )
Work	*work ;
int		name ;
int		where ;
{
	NPCWORK *npc ;
	FVECTOR	start_pos ;
	char *opt ;
	int model ;
	
	npc = &work->npc ;


	/* シナリオから値を取得 */
	work->speed = (float)GCL_GetOptionValue( 's', 1 ) ;

	/* NPC初期化 */
	NPC_InitNPC( npc, work, &work->body, &work->control, work->lights ) ;

	/* コントロールの初期化 */
	NPC_InitControl( npc, name ) ;

	
	model = GCL_GetOptionValue( 'm', 0 ) ;
	if ( model == 0 ) model = MODEL_NAME ;
	/* オブジェクトの初期化 */
	NPC_InitObject( npc, model ) ;

	/* マルチウェイトモデルの初期化 */
//	NPC_InitMWObject( npc, MW_MODEL_NAME, 4 ) ;

	/* モーションの初期化 */
	NPC_InitMotion( npc, BASE_MOTION, DAMAGE_MOTION, CAPTURE_MOTION, DRAG_MOTION ) ;

	/* ターゲットの初期化 */
	NPC_InitDefenceTarget( npc, &work->deftrg, ENEMY_SIDE, SampleTrgSize, SampleDragShift, &DG_ZeroVector ) ;
	NPC_InitDefenceChildTarget( npc, work->def_child,
		ENEMY_SIDE, Cheild_Target_Size, Cheild_Target_Shift, Child_Level_Num, Cheild_Target_Connect ) ;
	NPC_InitDefenceCapture( npc, &work->capture, &work->control, &work->body ) ;
	/* 首締めシフト座標セット */
	NPC_SetHangShift( npc, &HangShift ) ;

#ifdef SMP_TARGET_SKIP
	npc->target.deftrg->class |= TARGET_SKIP ;
#endif

	/* ナビシステム初期化 */
	NPC_InitNavi( npc, &work->control, &work->navigate, &work->navitrg ) ;

	/* 初期位置、方向、モーションのセット */
	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
		ENE_GCL_GetFV( opt, &start_pos ) ;
	} else {
		start_pos = GM_PlayerPosition ;
		start_pos.vz -= 1500 ;
	}
	NPC_InitPose( npc, &start_pos, 1024, BASE_MOTION, 0 ) ;

	/* レーダー情報初期化 */
	NPC_InitRader( npc, &work->rctrl, &npc->ctrl->mov, 
			2000.0f, -2000.0f, 512, 4000, RADAR_COLOR_RED ) ;

	/* ヘッドマークコントロール初期化 */
	NPC_InitHeadMark( work, npc, &work->headmark, &work->body, &work->control, &work->deftrg ) ;

	/* パラメータセット */
	NPC_SetActionParam( &npc->action, SMP_LIFE, SMP_FAINT, SMP_FAINT_COUNT,
				SMP_SLEEP_COUNT, SMP_DOWN_DAM  ) ;

	/* ライフゲージ（各キャラクターが管理する) */
	GM_InitGageSet( &work->gage, "FATMAN", 16, 196, 3, COUNT_VMODE( SMP_LIFE ),
				    COUNT_VMODE( SMP_LIFE ), 0, 30, NPC_GAGE_LEVEL ) ;
	work->gage.text_len = 23 ;	/* 見栄えで、えー感じに設定 */
	GM_SetGageColor( &work->gage, 0, 0, 0, 31, 63, 192, 31, 127, 255, 255, 0, 0 ) ;
	GM_AppendGageSet( &work->gage ) ;
	GM_VisibleGage( &work->gage ) ;

	/* フェイスアニメーションは それぞれのキャラが独自で制御して下さい。
	但し、NPCのシステムが npc->action.face_anime に 
	NPC_FCANIM_EYE_CLOSE や NPC_FCANIM_SUFFER に変更した場合
	対応するようにしてください。 */
#if 0
	if ( NPC_InitFaceAnime( npc, npc->body->evmobj, SMP_FACE_FAR, EMMA_INF_NAME ) < 0 ) {
		return -1 ;
	}
#endif

	/* アイテムプロックセット */
	if ( GCL_GetOption( 'q' ) != NULL ){
		KRTH_GetItemProc( &npc->action.item, name ) ;
	}

	/* 思考処理初期化 */
	StartThink( work ) ;

	work->mode = SMP_MODE_NORMAL ;
	work->bomb_zone = 0 ;

#if 1
{
	extern	void	NewHostageSkirtA( OBJECT *body, FMATRIX *lights ) ;
	extern void *NewEvm_SkirtA( OBJECT * ); // ネーちゃん
	extern void *NewEvm_SkirtB( OBJECT * ); // おばはん

	//NewHostageSkirtA( npc->body, work->lights ) ;

  	NewEvm_SkirtA( npc->body );
}
#endif

	return 0 ;
}

	/*
		シナリオ呼び出し
	*/
void		*NewNPCSample( name, where )
int		name ;
int		where ;
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
	return work ;
}
