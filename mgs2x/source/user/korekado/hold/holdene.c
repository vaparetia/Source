//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
#include "BP_Renderer.h"
//BP
//----------------------------------------------------------------------------
/*
	NewHoldEnemy.c
	船倉兵

	2000/03/22 Y.Korekado
	$Id: holdene.c,v 1.1.1.3 2002/11/19 11:44:16 Yoshizawa1 Exp $

*/

/*----------------------------------------------------------------
void	*NewHoldEnemy( name, where )

シナリオオプション
		-m	オブジェクト名
		-p	場所
		-r	向き
		-s  ステータス

モーション

銃を肩にかけた歩き




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
#include	"camera.h"

#include	"korekado/conv/korekado.x"
#include	"korekado/enemy/enemy.h"

#include	"../../kano/us_soldier/us_soldier.h"

#include	"holdene.h"
#include	"chkmot.h"

#define PROJECTOR_SHADOW	(1)

#define	WEAPON_SHOLDER	(1)	/* 銃は肩に掛ける */

#define TARGET_ON (1)
//#define NO_PROC_CALL	(1)
#define ROOT_WALKER		(1)

#define NO_SCALE (1)	/* スケーリングは処理が多いので止める */
//#define NO_HELM (1)		/* ヘルメット無しで処理節約 */

//#define HOLD_SET_NEEDL	(1)	//GM_ActContorl 通らないから無理

#if BP_VITA
#  define HOLDENE_VITA_RENDER_OPT 1
#  define HOLDENE_VITA_UPDATE_OPT 1
#endif

#define _GUN_HAND	COUNT_VMODE(11)
/*----------------------------------------------------------------*/
#define HLD_NOISE_HEGHT (1300.0f)
#define HLD_DETECT_DELAY_TIME (COUNT_VMODE(60))
#define HOLD_FLOOR_LEVEL	(-17100.0f)

#define HLD_EYE_SIGTH	DEF_EYE_SIGHT
#define HLD_EYE_RANGE	DEF_EYE_RANGE / 2

//#define MAX_ENE	64
#define MAX_ENE	128

#define MODEL_NUM 18
#define MODEL_SCOT MODEL_NUM + 1
#define MODEL_PANTS MODEL_NUM + 2
#define MODEL_PANTS_NOPROC MODEL_NUM + 3

//#define ACTIVE_MID_NUM	(6)
#define ACTIVE_LOW_NUM	(6)	//この人数以上のアクティブはローポリ


#define MASTER_NUM	6
#define MOTION_NUM	USHOLD_gbs_ak_nom_idle_atk
#define MOTION_BUFF_SIZE	(300 * 10) /* 3000flam */

#define MODEL_SCOTTO 30

#define MOT_BASE_TIME	(BP_BASE_TICK())
#define	TIME_BASE	(BP_BASE_TICK())

#define	HOLD_ACT_INTERP_M			(900) /* 1/300単位 */
#define	HOLD_ACT_INTERP_S			(150) /* 1/300単位 */

#define RIGHT_TURN_MOT	USHOLD_look_r
#define LEFT_TURN_MOT	USHOLD_look_l
#define SALUTE_MOT		USHOLD_idle_2	/* 敬礼モーション */

#define NO_TURN_MOT	USHOLD_look_f

#define NECK_READY_MOT	USHOLD_listen_speech_refresh_idle	/* 首運動用意モーション */


#define	BODY_FLAG	(DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_IRREACTION)
#define	ITEMS_FLAG	(DG_FLAG_ONEPIECE)

#define DEF_NAME	GV_StrCode("us_def_1")
#define MOT_NAME	GV_StrCode("ushold")

#define CAP_NAME	GV_StrCode("obj_cap")
#define GLASS_NAME	GV_StrCode("obj_glass")
#define HELMET_NAME	GV_StrCode("obj_helmet_def_s")
#define WP_NAME_1	GV_StrCode("m4a_nm")
#define WP_NAME_2	GV_StrCode("m4b_gl")
#define WP_NAME_3	GV_StrCode("m4c_sc")
#define SCOT_CAP_NAME	GV_StrCode("sco_hat")
#define HEADPHONES_NAME	GV_StrCode("obj_headphone_def")

#define WP_SHLD_NAME_1	GV_StrCode("m4a_shld")

#define	ITEM_CAP	0x0001
#define	ITEM_GLASS	0x0002
#define	ITEM_HELMET	0x0004
#define	ITEM_WP_1	0x0010
#define	ITEM_WP_2	0x0020
#define	ITEM_WP_3	0x0040
#define	ITEM_SCOT_CAP	0x0080
#define	ITEM_HEADPHONES	0x0100
#define	ITEM_KAITAIC4	0x0200

#define ITEM_ATAMA	(ITEM_HEADPHONES|ITEM_SCOT_CAP|ITEM_HELMET|ITEM_CAP)
#define ITEM_WEAPON	(ITEM_WP_1|ITEM_WP_2|ITEM_WP_3)

/* project_flag */
#define FLAG_EYECHECK	0x80000000
#define FLAG_EYE_IN		0x00000001

/* end proc */
#define MAX_PROC	12
enum {
	PROC_DETECT,
	PROC_PROJE1,
	PROC_PROJE2,
	PROC_NOISE,
	PROC_NORMAL_DETECT,
	PROC_LIGHTER,
	PROC_CAMERA1,
	PROC_CAMERA2,
	PROC_SCOT,
	PROC_DEATH,
	PROC_START_LIGHTER_EVENT,
} ;

#define NOISE_DIS	10000

#define MODE_DEMO (1)
#define MODE_JIK_PARA (2)
/*----------------------------------------------------------------*/
typedef struct {
	FVECTOR			prev, c_prev ;	/* 補間元クォータニオン＆補正用クォータニオン */
	FVECTOR			next, c_next ;	/* 補間先クォータニオン＆補正用クォータニオン */
	FVECTOR			t ;
} SQUAD_WORK ;

typedef struct {
	FVECTOR	from[4] ;
	FVECTOR	to[4] ;
	float	t[4] ;
	float	tmp[4], param_x[4], param_y[4] ;
	float	scale_from[4], scale_to[4] ;
	float	omega[4] ;
	float	cosom[4] ;
	float	sinom[4] ;
} SLERP4_WORK;

typedef struct {
	FVECTOR	root ;
	FVECTOR	joints[64] ;
	FVECTOR	quat_buffer[4] ;
	FVECTOR	vec ;
	FMATRIX	mat ;
	FMATRIX	root_mat ;
	FMATRIX	mats[64] ;
	SLERP4_WORK	slerp4_work ;
	SQUAD_WORK	squad_work[64] ;
} ScrPadWork ;

struct _SBP_OBJ_Render;

typedef	struct holdene_Work	{
	GV_ACT_EX	actor ;
	HOLD_ENE	holdene[ MAX_ENE ] ;
	OBJECT		body_mtbuff[MOTION_NUM] ;	/* モーションバッファ用 */
	FVECTOR		step_buff[MOTION_NUM]  ;

	int			name ;
	int			lod_th[ MAX_LOD-1 ] ;
	int			ene_num ;
	int			eye_sight ;		/* 視力 */
	int			eye_range ;		/* 視界 */
	int			hzx_id ;		/* マップ */
	FVECTOR		screen_pos ;	/* スクリーンの場所 */
	FVECTOR		screen_pos2 ;	/* スクリーン２の場所 */
#ifdef PROJECTOR_SHADOW
	FVECTOR		projector1pos ;	/* プロジェクター１場所 */
	FVECTOR		projector2pos ;	/* プロジェクター２場所 */
	SVECTOR		projector1rot ;	/* プロジェクター１方向 */
	SVECTOR		projector2rot ;	/* プロジェクター２方向 */
#endif
	int			projector1flag ;	/* プロジェクター１フラグ */
	int			projector2flag ;	/* プロジェクター２フラグ */
	int			gameflag ;	/* ゲームフラグ */

	FVECTOR		camera_pos[2] ;	/* ＴＶカメラの場所 */
	int			camera_num ;	/* カメラ台数*/
	int			cameraman[2] ;	/* カメラマン*/

	int			proc_detect[ MAX_PROC ] ;

	u_short		m_ptr[MOTION_NUM] ;
	u_short		m_len[MOTION_NUM] ;
	FVECTOR		*m_buff ;
	FVECTOR		*m_step ;
	u_short		*m_height ;

	/* コマンダー関係 */
	E_GROUP		group ;
	E_UNIT		unit ;
	COMMANDER	*com ;

	int			scn_num ;
	int			pose_type ;		/* 立ち方 */
	int			aim_type ;	/* 体の向き方 */
	int			count ;		/* ゲームカウント */
	int			mode ;

	short		event_id ;
	short		event_ene ;
	FVECTOR		event_pos ;

	FVECTOR		*scot_pos ;		/* スコット座標 */
	int			active_num ;
	int			detect_delay ;

	FVECTOR		touch_pos[3] ;
} HoldEneWork ;

#define Work HoldEneWork

static	Work	*WORKP ;

/* event_id */
#define HLD_EVENT_LIGTER		0x00000001	/* ライター落とせ */


enum {
	TH_STAND,
	TH_ATTENTION,
	TH_SALUTE,
	TH_RETURN_SALUTE,
	TH_TURN,
	TH_NOISE,

	TH_NECK,
	TH_ALERT,
	TH_JOKE,
	TH_CAMERA,

	TH_ACTIVE,
	TH_DETECT,
	TH_PROJECTOR,
	TH_END,

	TH_DAMAGE,
} ;

enum {
	TH2_READY,
	TH2_IDLE,
	TH2_END,
	TH2_SLEEP_READY,
	TH2_SLEEP,
	TH2_MOVE,
	TH2_START,
	TH2_RETURN
} ;

enum {
	/* 立ち方 */
	GSCN_NONE,
	GSCN_SILENT,	/* 黙とうを捧げる */
	GSCN_ATTENTION,	/* 気を付けー */
	GSCN_SALUTE,	/* 敬礼する */
	GSCN_RETURN_SALUTE,	/* 直れー */
	GSCN_EXERCISE,	/* 首を回す */

	GSCN_GUN_RIGHT,		/* 銃を右に向ける */
	GSCN_GUN_LEFT,		/* 銃を左に向ける */
	GSCN_GUN_BACK,		/* 銃を後に向ける */
	GSCN_GUN_FORWARD,	/* 銃を前に向ける */

	GSCN_NECK_RIGHT,	/* 右に首を向ける */
	GSCN_NECK_LEFT,		/* 左に首を向ける */
	GSCN_NECK_DOWN,		/* 下に首を向ける */
	GSCN_NECK_FORWARD,	/* 前に首を向ける */
	GSCN_NECK_UP,		/* 上に首を向ける */

	GSCN_JOKE,		/* アメリカンジョーク */

	/* 向き方 */
	GSCN_SCREEN1,	/* スクリーン１を見る */
	GSCN_SCREEN2,	/* スクリーン２を見る */
	GSCN_RIGHT,		/* 右（1024）を見る */
	GSCN_LEFT,		/* 左（3072）を見る */
	GSCN_BACK,		/* 後ろ（0）を見る */
	GSCN_FORWARD,	/* 前（2048）を見る */
	GSCN_SCOT,		/* スコットを見る */
} ;


/*----------------------------------------------------------------*/
/* gbs.mlsから船倉兵専用へのモーション変換リスト */
int	use_standard_motion_num[][2] = {
/* 基本 */
	{ EM_stand, USHOLD_gbs_ak_nom_idle_atk },
	{ EM_walk, USHOLD_gbs_ak_nom_walk },
	{ EM_ijiiji, USHOLD_gbs_ak_nom_ijiiji },	/* 下覗き込み */
	{-1, -1},
} ;
int	CheckEneMotionArray[ EM_STANDARD_MAX ] ;
int	ScotMotionArray[ EM_STANDARD_MAX ] ;


//static	char gamescn[64] = { GSCN_NONE, GSCN_SALUTE, GSCN_LEFT, GSCN_NONE,GSCN_SALUTE, GSCN_NONE, GSCN_LEFT } ;
//static	char gamescn[64] = { GSCN_SCREEN2, GSCN_SCREEN1, -1 } ;
//static	char gamescn[64] = { GSCN_NONE, GSCN_ATTENTION, GSCN_SALUTE, GSCN_RETURN_SALUTE, -1 } ;
static	char gamescn[64] = { GSCN_NONE, -1 } ;

#define	JIKU_TIME1	COUNT_VMODE(300)

static	char	motion_table[][8] = {
#if 0
	{ USHOLD_idle_1, USHOLD_1_uneune, -1, -1,
		-1,-1,-1,-1},
	{ USHOLD_idle_2, USHOLD_2_to_4, USHOLD_idle_4, -1,
		-1, -1 ,-1,-1},
#else
	{ USHOLD_idle_1, -1, -1, -1,
		-1,-1,-1,-1},
	{ USHOLD_idle_2, -1, -1, -1,
		-1, -1 ,-1,-1},
#endif
	{ USHOLD_idle_3, -1, -1, -1,
		-1,-1,-1,-1},
	{ USHOLD_idle_4, -1, -1, -1,
		-1,-1,-1,-1},
	{ USHOLD_idle_5, -1, -1, -1,
		-1,-1,-1,-1},
	{ USHOLD_idle_6, -1, -1, -1,
		-1, -1, -1,-1},

	/* カメラマンモーション */
	{ USHOLD_usl_fold_cameraman_idle, -1, -1, -1,
		-1, -1, -1,-1},
} ;

#define MT_TYPE_LOOP	0
#define MT_TYPE_JOINT	1
#define MT_TYPE_ONE		2

#ifdef WEAPON_SHOLDER
static	char	motion_stat[] = {
	0,0,0,0, 0,0,1,0, 1,0,1,0,
	1,0, -1
} ;
#else
static	char	motion_stat[] = {
	0,0,0,0, 0,0,0,1, 0,1,0,1,0,
	1,1,1,1,1, 0,0,0,0,1, -1
} ;
#endif

static short	se_bikkuri[]={
//SD_E_BIKKRIS11,SD_E_BIKKRIS12,SD_E_BIKKRIS13,SD_E_BIKKRIS14
SD_E_BIKKRIS21,SD_E_BIKKRIS22,SD_E_BIKKRIS23,SD_E_BIKKRIS24
//SD_E_BIKKRIS11,0,SD_E_BIKKRIS12,0,SD_E_BIKKRIS13,0,SD_E_BIKKRIS14,0
} ;

static short	se_gun_noise[]={
//SD_E_USAEQU01,SD_E_USAEQU02,SD_E_USAEQU03,SD_E_USAEQU04
SD_E_USAEQU01,0,SD_E_USAEQU02,0,SD_E_USAEQU03,0,SD_E_USAEQU04,0
} ;

static short	se_kinuzure[]={
//SD_E_USATRN01,SD_E_USATRN02,SD_E_USATRN03,SD_E_USATRN04
SD_E_USATRN01,0,SD_E_USATRN02,0,SD_E_USATRN03,0,SD_E_USATRN04,0
} ;

static short	se_asioto[]={
SD_P_SENAKA02,SD_P_STAND02,SD_P_FOOTR01,SD_P_FOOTL01
} ;



/*----------------------------------------------------------------*/
FVECTOR HOLD_WP_Shift[] = {
{ -200.0f, -250.0f, -50.0f,},
{ 0.0f,0.0f,0.0f },
{ 0.0f,-200.0f,-400.0f },
} ;

SVECTOR HOLD_WP_Rot[] = {
	{ 2024,0,0 },
	{ 0,0,0 },
	{ -1024,0,0 },
} ;

/*----------------------------------------------------------------*/
extern void *NewSleep(FMATRIX *world, int *map ) ;
extern void SearchAndKillAttachment_called(OBJECT *obj) ;
/*----------------------------------------------------------------*/
static void SetActiveNum( )
{
	WORKP->active_num++ ;
printf("hold: set active[%d]\n",WORKP->active_num);
}
static void UnSetActiveNum( )
{
	WORKP->active_num-- ;
printf("hold: unset active[%d]\n",WORKP->active_num);
}
static int ActiveNum( )
{
	return WORKP->active_num ;
}

static void SetControlPos( HOLD_ENE *hld )
{
	CONTROL	*ctrl ;
	
	ctrl = &hld->control ;
	ctrl->mov = hld->mov ;
	ctrl->turn = ctrl->rot = hld->rot ;
}

static int NameToID( Work *work, int name )
{
	HOLD_ENE	*hld ;
	int i ;
	
	hld = &work->holdene[0] ;
	for( i=0; i<work->ene_num; i++ ) {
		if( hld->name == name ) return i ;
		hld++ ;
	}
	return -1 ;
}

int	HLD_GetHoldPosition( void )
{
	GCL_VAR_REF ref; //配列への参照データ
	int			name, id ;
	HOLD_ENE	*hld ;
	FVECTOR		*pos ;
	CONTROL		*ctrl ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	name = GCL_GetNextInt() ;

	if ( (ctrl = GM_SearchWhere( name )) != NULL ) {
		pos = &ctrl->mov ;
	} else {
		id = NameToID( WORKP, name ) ;
		hld = &WORKP->holdene[id] ;
		pos = &hld->mov ;
	}

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)pos->vx );

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)pos->vy );

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)pos->vz );

	return 1 ;
}

int	HLD_GetHoldStaus( void )
{
	GCL_VAR_REF ref; //配列への参照データ
	int			name, id ;
	HOLD_ENE	*hld ;
	FVECTOR		*pos ;
	CONTROL		*ctrl ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	name = GCL_GetNextInt() ;

	id = NameToID( WORKP, name ) ;
	hld = &WORKP->holdene[id] ;

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)hld->status2 );

	return 1 ;
}

static int GetGunDir( int scn_dir )
{
	if( scn_dir == GSCN_GUN_RIGHT ) return 1024 ;
	if( scn_dir == GSCN_GUN_LEFT ) return 3072 ;
	if( scn_dir == GSCN_GUN_BACK ) return 0 ;
	return 2048 ;
}

static void NoiseProc( HOLD_ENE *hld )
{
	GCL_ARGS arg ;
	int	argv[ 4 ] ;

	argv[0] = (int)hld->mov.vx ;
	argv[1] = (int)hld->mov.vy ;
	argv[2] = (int)hld->mov.vz ;

	arg.argc = 3 ;
	arg.argv = &argv[0] ;
	ENE_ExecProc( WORKP->proc_detect[ PROC_NOISE ], &arg ) ;
}

static void LighterProc( HOLD_ENE *hld )
{
	GCL_ARGS arg ;
	int	argv[ 4 ] ;
	CONTROL	*ctrl ;

	if ( (ctrl = GM_SearchWhere( hld->name )) != NULL ) {
		argv[0] = (int)ctrl->mov.vx ;
		argv[1] = (int)ctrl->mov.vy ;
		argv[2] = (int)ctrl->mov.vz ;
	} else {
printf(" holdene: lighter detect holdene \n") ; 
		argv[0] = (int)hld->mov.vx ;
		argv[1] = (int)hld->mov.vy ;
		argv[2] = (int)hld->mov.vz ;
	}

	arg.argc = 3 ;
	arg.argv = &argv[0] ;
	ENE_ExecProc( WORKP->proc_detect[ PROC_LIGHTER ], &arg ) ;
	WORKP->proc_detect[ PROC_LIGHTER ] = 0 ;/* 一回しか呼ばない */
}

static void StartLighterEventProc( HOLD_ENE *hld )
{
	ENE_ExecProc( WORKP->proc_detect[ PROC_START_LIGHTER_EVENT ], NULL ) ;
}

static void NormalDetectProc( HOLD_ENE *hld )
{
	GCL_ARGS arg ;
	int	argv[ 4 ] ;
	CONTROL	*ctrl ;

	if ( (ctrl = GM_SearchWhere( hld->name )) != NULL ) {
		argv[0] = (int)ctrl->mov.vx ;
		argv[1] = (int)ctrl->mov.vy ;
		argv[2] = (int)ctrl->mov.vz ;
	} else {
		argv[0] = (int)hld->mov.vx ;
		argv[1] = (int)hld->mov.vy ;
		argv[2] = (int)hld->mov.vz ;
	}

	arg.argc = 3 ;
	arg.argv = &argv[0] ;
//printf("Normal Detect Call Proc [%d] \n",WORKP->proc_detect[ PROC_NORMAL_DETECT ] ) ;
	ENE_ExecProc( WORKP->proc_detect[ PROC_NORMAL_DETECT ], &arg ) ;
	WORKP->proc_detect[ PROC_NORMAL_DETECT ] = 0 ;/* 一回しか呼ばない */
}

static void DeathProc( HOLD_ENE *hld )
{
	GCL_ARGS arg ;
	int	argv[ 4 ] ;
	CONTROL	*ctrl ;

	if ( (ctrl = GM_SearchWhere( hld->name )) != NULL ) {
		argv[0] = (int)ctrl->mov.vx ;
		argv[1] = (int)ctrl->mov.vy ;
		argv[2] = (int)ctrl->mov.vz ;
	} else {
		argv[0] = (int)hld->mov.vx ;
		argv[1] = (int)hld->mov.vy ;
		argv[2] = (int)hld->mov.vz ;
	}

	arg.argc = 3 ;
	arg.argv = &argv[0] ;
//printf("Normal Detect Call Proc [%d] \n",WORKP->proc_detect[ PROC_NORMAL_DETECT ] ) ;
	ENE_ExecProc( WORKP->proc_detect[ PROC_DEATH ], &arg ) ;
	WORKP->proc_detect[ PROC_DEATH ] = 0 ;/* 一回しか呼ばない */
}

static void ScotProc( HOLD_ENE *hld )
{
	GCL_ARGS arg ;
	int	argv[ 4 ] ;
	CONTROL	*ctrl ;


	if ( (ctrl = GM_SearchWhere( hld->name )) != NULL ) {
		argv[0] = (int)ctrl->mov.vx ;
		argv[1] = (int)ctrl->mov.vy ;
		argv[2] = (int)ctrl->mov.vz ;
	} else {
printf(" holdene: normal detect holdene \n") ; 
		argv[0] = (int)hld->mov.vx ;
		argv[1] = (int)hld->mov.vy ;
		argv[2] = (int)hld->mov.vz ;
	}

	arg.argc = 3 ;
	arg.argv = &argv[0] ;
	ENE_ExecProc( WORKP->proc_detect[ PROC_SCOT ], &arg ) ;
	WORKP->proc_detect[ PROC_SCOT ] = 0 ;/* 一回しか呼ばない */
}

static	void SetNextGameScn( work )
Work	*work ;
{
	int	scn ;

	work->scn_num++ ;
	scn = gamescn[ work->scn_num ] ;
	
	if ( scn < 0 ) {
		work->scn_num = 0 ;
		scn = gamescn[ work->scn_num ] ;
	}

	if ( scn < GSCN_SCREEN1 ) {
		work->pose_type = scn ;
	} else {
		work->aim_type = scn ;
	}

//printf( "pose [%d] aim[%d]\n",work->pose_type,work->aim_type ) ;

}

static	FVECTOR	*GetAbsRot( hld )
HOLD_ENE	*hld ;
{
	int mas, m_time ;
	FVECTOR	*abs ;

	mas = hld->mot_num ;

	m_time = hld->m_time%WORKP->m_len[mas] ;
//printf( "m_time[%d] hld->m_time[%d] mas[%d]\n",m_time, hld->m_time,mas);
	if ( hld->reverse_flag ) {
		m_time = WORKP->m_len[mas] - m_time ;
//if ( hld->id == 0 )printf("reverse m_time [%d]\n",m_time ) ;
	}
	abs = WORKP->m_buff + (MODEL_OBJS*(WORKP->m_ptr[mas]+m_time)) ;

	return abs ;
}


static	void	SetMotion( hld, num, intrp )
HOLD_ENE	*hld ;
int			num ;
int			intrp ; 
{
	FVECTOR	*abs ;

	if ( hld->mot_num == num ) return ;

	if ( hld->m_time>0) hld->m_time-- ;
	abs = GetAbsRot( hld ) ;

//printf(" set motion [%d] intrp[%d]\n",num,intrp ) ;
	hld->mot_num = num ; 
	hld->m_time = 0;
	hld->interp_count = intrp ;
	hld->interp_inc = intrp ;
	hld->interp_time = 0.0f ;
	hld->reverse_flag = 0 ;

	KR_MemCopy( &hld->abs_rots[0], abs, sizeof(FVECTOR), MODEL_OBJS ) ;
}

static	void	SetMotion2( hld, num, intrp )
HOLD_ENE	*hld ;
int			num ;
int			intrp ; 
{
	if ( hld->mot_num == num ) return ;
	SetMotion( hld, num, intrp ) ;
}

static	int	NextMotion( hld )
HOLD_ENE	*hld ;
{
	hld->c_mot ++ ;
	
	if ( motion_table[ hld->mot_type ][hld->c_mot] < 0 ) hld->c_mot = 0 ;
	
	return motion_table[ hld->mot_type ][hld->c_mot] ;
}

static	void	SetNextMotion( hld )
HOLD_ENE	*hld ;
{
	SetMotion2( hld, NextMotion( hld ), HOLD_ACT_INTERP_M ) ;
	switch( motion_stat[ hld->mot_num ] ) {
		case MT_TYPE_JOINT :
			hld->p_time = WORKP->m_len[hld->mot_num]-1 ;
			break ;
		case MT_TYPE_LOOP :
		default :
			hld->p_time = 60*MOT_BASE_TIME + KR_RandU( 60*20 ) ;
			break ;
	}
}

static	int	ChangeActive( hld )
HOLD_ENE	*hld ;
{
	extern void *NewCheckEnemy(OBJECT *,OBJECT *,FVECTOR *,int,FVECTOR *, int, int *, int,
			int, FVECTOR *, CNCTOBJ *, CONTROL *, HOLDBODYPARAM *, HOLD_ENE * ) ;
	FVECTOR	*mt_rots, *trg_pos ;
	int		i, m_time, mode, event, size, obj_n ;

	size = GV_GetMaxFreeMemory( GV_NORMAL_MEMORY ) ;
	if ( size < GV_MEM_EFFECT_LIMIT_SIZE ) {
printf("holdene:WARNING!!! No Memory!!!!! [%d]\n",size ) ;
		return -1 ;
	}

#ifdef HOLD_SET_NEEDL
	SearchAndKillAttachment_called(&hld->body[0]) ;
#endif

	/* アクティブ人数によってモデルクオリティ変更 */
	obj_n = ( ActiveNum( ) < ACTIVE_LOW_NUM ) ? 0 : 2 ;

	if ( hld->items & ITEM_KAITAIC4 ) {
		obj_n = 0 ;
	}

	event = 0 ;
	trg_pos = NULL ;
	for ( i=0; i<MAX_LOD; i++ ) {
		if ( i == obj_n ) {
			hld->body[i].objs->flag &= ~DG_FLAG_INVISIBLE ;
		} else {
			hld->body[i].objs->flag |= DG_FLAG_INVISIBLE ;
		}
	}
	if ( hld->items & ITEM_WEAPON ) {
		hld->cnct.body.objs->root = &( hld->body[obj_n].objs->objs[ HUMAN21_MIGI_TE ].world ) ;
	}
	if ( hld->items & ITEM_ATAMA ) {
		hld->atama.objs->root = &( hld->body[obj_n].objs->objs[ HUMAN21_ATAMA ].world ) ;
	}
	if ( hld->items & ITEM_GLASS ) {
		hld->glass.objs->root = &( hld->body[obj_n].objs->objs[ HUMAN21_ATAMA ].world ) ;
	}

printf("[%d] active status [%x] \n",hld->id, hld->status2 ) ;
	mode = ENE_NOTICE_INDISTINCT ;
	if( hld->status2 & HLD_STATUS2_NOISE_S ) {
		mode = ENE_NOTICE_NOISE ;
		NoiseProc( hld ) ;
printf("[%d] active noise\n",hld->id ) ;
	} else if( hld->status2 & HLD_STATUS2_HOLD_UP ) {
		mode = HLD_EVENT_HOLDUP ;
printf("[%d] active hold up\n",hld->id ) ;
	} else if( hld->status2 & HLD_STATUS2_LIGHTER ) {
		event = HLD_EVENT_LIGHTER ;
		trg_pos = &WORKP->event_pos ;
printf("[%d] active lighter\n",hld->id ) ;
	} else if( hld->status2 & HLD_STATUS2_TOUCH ) {
		event = HLD_EVENT_TOUCH ;
printf("[%d] active touch\n",hld->id ) ;
	} else if( hld->status2 & HLD_STATUS2_DAMAGE ) {
		event = HLD_EVENT_DAMAGE ;
	} else if( hld->status2 & HLD_STATUS2_CAPTURE ) {
		event = HLD_EVENT_CAPTURE ;
printf("[%d] active hang\n",hld->id ) ;
	} else {
printf("[%d] active other\n",hld->id ) ;
	}

	m_time = hld->m_time%WORKP->m_len[hld->mot_num] ;
	mt_rots = WORKP->m_buff + (MODEL_OBJS*(WORKP->m_ptr[hld->mot_num]+m_time)) ;
	hld->active = NewCheckEnemy( &hld->body[obj_n], &hld->cnct.body, 
		&hld->mov, hld->rot.vy, &hld->abs_rots[0], WORKP->name, &hld->status, mode,
		event, trg_pos, &hld->cnct, &hld->control, &hld->bodyparam, hld ) ;
	SetActiveNum( ) ;

	if ( hld->headmark != NULL ) {
printf(" destroy headmark !!\n") ;
		GV_DestroyOtherActor( hld->headmark ) ;
		hld->headmark = NULL ;
	}

	return 0 ;
}

static	void	ChangePassive( hld )
HOLD_ENE	*hld ;
{
	hld->status &= ~HLD_STATUS_ACTIVE_END ;
	hld->status &= ~HLD_STATUS_ACTIVE ;	/* 活動終了 */
	GV_DestroyActorQuick( hld->active ) ;
	UnSetActiveNum( ) ;

	hld->c_obj = -1 ;	/* LOD計算し直し */

#if 1

	hld->c_mot = 0 ;	/* モーションセット */

	if ( hld->status&HLD_STATUS_CAMERAMN ) {
		hld->mot_num = 0 ; 
	} else {
		hld->mot_num = motion_table[ hld->mot_type ][hld->c_mot]  ; 
	}
	hld->m_time = 0;
	hld->interp_count = HOLD_ACT_INTERP_M ;
	hld->interp_inc = HOLD_ACT_INTERP_M ;
	hld->interp_time = 0.0f ;
	hld->p_time = WORKP->m_len[hld->mot_num] ;

	hld->cnct.body.objs->root = NULL ;
#else
	SetNextMotion( hld ) ;
#endif
}


static int SleepNotice( hld )
HOLD_ENE	*hld ;
{
	FVECTOR	pos, vec ;

//return 0 ;

	if ( WORKP->mode == MODE_DEMO ) return 0 ;

	if( hld->status2 & HLD_STATUS2_DETECT || GM_GameStatus & GM_STATUS_DETECT ) {
#ifdef HOLD_SET_NEEDL
		SearchAndKillAttachment_called(&hld->body[0]) ;
#endif
		GV_DestroyOtherActor( hld->headmark ) ;
		KR_FMatToFvec( &BODYWORLD( &hld->body[hld->c_obj], HUMAN21_KOSHI), &pos ) ;
		_sceVu0SubVector(  &vec, &GM_PlayerPosition, &pos ) ;
		hld->p_dir = _FVecDir2( &vec ) ;
		hld->p_dis = _FVecLen2( &vec ) ;

//		hld->p_time = (hld->p_dis/500) + KR_RandU( 10*1 ) ;	/* 振り向き始める時間 */
		hld->p_time = COUNT_VMODE((hld->p_dis/500)) ;	/* 振り向き始める時間 */
		hld->think = TH_DETECT ;
		hld->count = 0 ;

		return 1 ;

	} else if( hld->status2 & HLD_STATUS2_PROJECTOR ) {
#ifdef HOLD_SET_NEEDL
		SearchAndKillAttachment_called(&hld->body[0]) ;
#endif
		GV_DestroyOtherActor( hld->headmark ) ;
		hld->think = TH_PROJECTOR ;
		hld->count = 0 ;

		return 1 ;
	}
	return 0 ;
}

static int Notice( hld )
HOLD_ENE	*hld ;
{
	FVECTOR	pos, vec ;

//return 0 ;

	if ( WORKP->mode == MODE_DEMO ) return 0 ;

	if( hld->status2 & HLD_STATUS2_DETECT || GM_GameStatus & GM_STATUS_DETECT ) {
		KR_FMatToFvec( &BODYWORLD( &hld->body[hld->c_obj], HUMAN21_KOSHI), &pos ) ;
		_sceVu0SubVector(  &vec, &GM_PlayerPosition, &pos ) ;
		hld->p_dir = _FVecDir2( &vec ) ;
		hld->p_dis = _FVecLen2( &vec ) ;

//		hld->p_time = (hld->p_dis/500) + KR_RandU( 10*1 ) ;	/* 振り向き始める時間 */
		hld->p_time = COUNT_VMODE((hld->p_dis/500)) ;	/* 振り向き始める時間 */
		hld->think = TH_DETECT ;
		hld->count = 0 ;

		return 1 ;

	} else if( hld->status2 & HLD_STATUS2_PROJECTOR ) {
		hld->think = TH_PROJECTOR ;
		hld->count = 0 ;

		return 1 ;
	} else if( hld->status2 & (HLD_STATUS2_HOLD_UP|HLD_STATUS2_NOISE_S|HLD_STATUS2_INDISTINCT|HLD_STATUS2_LIGHTER) ) {
//printf("kroe1[%x]\n",hld->status2);
		if ( ChangeActive( hld ) < 0 ) {
			UNSET_FLAG( hld->status2, (HLD_STATUS2_HOLD_UP|HLD_STATUS2_NOISE_S|HLD_STATUS2_INDISTINCT|HLD_STATUS2_LIGHTER) ) ;
			return 0 ;
		} else {
			hld->status |= HLD_STATUS_ACTIVE ;	/* 活動開始 */
			hld->think = TH_ACTIVE ;
			hld->count = 0 ;
		}
		if( hld->status2 & (HLD_STATUS2_LIGHTER) ) {
			StartLighterEventProc( hld ) ;
		}

		return 1 ;
	}

	return 0 ;
}

static int PoseType( hld )
HOLD_ENE	*hld ;
{
	switch ( WORKP->pose_type ) {
		case GSCN_NONE :
			if ( hld->think != TH_STAND ) {
				SetNextMotion( hld ) ;
				hld->think = TH_STAND ;
				hld->count = 0 ;
				return 1 ;
			}
			break ;
		case GSCN_ATTENTION :
			if ( hld->think != TH_ATTENTION ) {
				hld->think = TH_ATTENTION ;
				hld->think2 = TH2_READY ;
				hld->count = 0 ;
				return 1 ;
			}
			break ;
		case GSCN_SALUTE :
			if ( hld->think != TH_SALUTE ) {
				hld->think = TH_SALUTE ;
				hld->think2 = TH2_READY ;
				hld->count = 0 ;
				return 1 ;
			}
			break ;
		case GSCN_RETURN_SALUTE :
			if ( hld->think != TH_RETURN_SALUTE ) {
				hld->think = TH_RETURN_SALUTE ;
				hld->think2 = TH2_READY ;
				hld->count = 0 ;
				return 1 ;
			}
			break ;
		case GSCN_GUN_RIGHT :
		case GSCN_GUN_LEFT :
		case GSCN_GUN_BACK :
		case GSCN_GUN_FORWARD :
			if ( hld->think != TH_ALERT ) {
				hld->aim_dir = GetGunDir( WORKP->pose_type ) ;
				hld->think = TH_ALERT ;
				hld->think2 = TH2_START ;
				hld->count = 0 ;
				return 1 ;
			} else if ( hld->aim_dir != GetGunDir( WORKP->pose_type ) ) {
				hld->aim_dir = GetGunDir( WORKP->pose_type ) ;
				hld->think2 = TH2_READY ;
				hld->count = 0 ;
				return 1 ;
			}
			break ;

		case GSCN_NECK_RIGHT :
		case GSCN_NECK_LEFT :
		case GSCN_NECK_DOWN :
		case GSCN_NECK_FORWARD :
		case GSCN_NECK_UP :
			if ( hld->think != TH_NECK ) {
				hld->think = TH_NECK ;
				hld->think2 = TH2_READY ;
				hld->count = 0 ;
				return 1 ;
			}
			break ;

		case GSCN_JOKE :
			if ( hld->think != TH_JOKE ) {
				hld->think = TH_JOKE ;
				hld->think2 = TH2_IDLE ;
				hld->count = 0 ;
				return 1 ;
			}
			break ;

	}

	return 0 ;
}

static int AimType( hld )
HOLD_ENE	*hld ;
{
	switch ( WORKP->aim_type ) {
		case GSCN_SCREEN1 :
			if ( hld->aim_dir != hld->screen1_dir ) {
				hld->aim_dir = hld->screen1_dir ;
				hld->think = TH_TURN ;
				hld->count = 0 ;
				return 1 ;
			}
			break ;
		case GSCN_SCREEN2 :
			if ( hld->aim_dir != hld->screen2_dir ) {
				hld->aim_dir = hld->screen2_dir ;
				hld->think = TH_TURN ;
				hld->count = 0 ;
				return 1 ;
			}
			break ;
		case GSCN_LEFT :
			if ( hld->aim_dir != 3072 ) {
				hld->aim_dir = 3072 ;
				hld->think = TH_TURN ;
				hld->count = 0 ;
				return 1 ;
			}
			break ;
		case GSCN_RIGHT :
			if ( hld->aim_dir != 1024 ) {
				hld->aim_dir = 1024 ;
				hld->think = TH_TURN ;
				hld->count = 0 ;
				return 1 ;
			}
			break ;
		case GSCN_BACK :
			if ( hld->aim_dir != 0 ) {
				hld->aim_dir = 0 ;
				hld->think = TH_TURN ;
				hld->count = 0 ;
				return 1 ;
			}
			break ;
		case GSCN_FORWARD :
			if ( hld->aim_dir != 2048 ) {
				hld->aim_dir = 2048 ;
				hld->think = TH_TURN ;
				hld->count = 0 ;
				return 1 ;
			}
			break ;
		case GSCN_SCOT :
			hld->aim_dir = hld->turn_y = _FVecTrgDir2( &hld->pos, WORKP->scot_pos ) ;
			hld->adj_turn_x = ( hld->pos.vy < WORKP->scot_pos->vy ) ? -300 : 300 ;

			break ;
	}

	return 0 ;
}

/*----------------------------------------------------------------*/
static	void	ThinkStand( hld )
HOLD_ENE	*hld ;
{
	if( Notice( hld ) ) 	return ;
	if( PoseType( hld ) ) 	return ;
	if( AimType( hld ) ) 	return ;

if ( hld->id == 0 ) {
//printf("stand: count[%d] time[%d]\n",hld->count, hld->p_time ) ;
}
	if( hld->count == hld->p_time ) {
		SetNextMotion( hld ) ;
		hld->count = 0 ;
		hld->turn_y = hld->aim_dir ;
	}

	if ( hld->c_mot == 0 ) {
		/* 初期モーション時に初期位置とずれていたら修正 */
		if ( hld->mov.vx > hld->pos.vx+10.0F ) hld->mov.vx -= 1.0F ;
		if ( hld->mov.vx < hld->pos.vx-10.0F ) hld->mov.vx += 1.0F ;
		if ( hld->mov.vz > hld->pos.vz+10.0F ) hld->mov.vz -= 1.0F ;
		if ( hld->mov.vz < hld->pos.vz-10.0F ) hld->mov.vz += 1.0F ;
	}

	hld->count ++ ;
}

static	void	ThinkAttentionReady( hld )
HOLD_ENE	*hld ;
{
	if( Notice( hld ) ) 		return ;
	if( PoseType( hld ) ) 		return ;
	if( AimType( hld ) ) 		return ;

	if( hld->count == 0 ) {
		hld->p_time = KR_RandU( COUNT_VMODE(10) ) ;
	}

	if( hld->count == hld->p_time ) {
		SetMotion( hld, USHOLD_listen_attention_start, HOLD_ACT_INTERP_S ) ;
//printf(" time[%d]\n",WORKP->m_len[hld->mot_num] ) ;
	}

	if ( hld->p_time+COUNT_VMODE(5) == hld->count ) {
//		GM_SeSetMode( se_kinuzure[hld->id%8] , &hld->mov, GM_SEMODE_BOMB ) ;
		GM_SeSetMode( se_asioto[hld->id%4] , &hld->mov, GM_SEMODE_BOMB ) ;
	}
	if ( hld->p_time+COUNT_VMODE(15) == hld->count ) {
		GM_SeSetMode( se_gun_noise[hld->id%8] , &hld->mov, GM_SEMODE_BOMB ) ;
	}

	if ( hld->count > hld->p_time ) {
		if ( hld->p_time + WORKP->m_len[hld->mot_num] - 2 == hld->count ) {
			hld->think2 = TH2_IDLE ;
			hld->count = 0 ;
			return ;
		}
	}

	hld->count ++ ;
}

static	void	ThinkAttentionIdle( hld )
HOLD_ENE	*hld ;
{
	if( Notice( hld ) ) 		return ;
	if( PoseType( hld ) ) 		return ;
	if( AimType( hld ) ) 		return ;

	if( hld->count == 0 ) {
		SetMotion( hld, USHOLD_listen_attention_idle, HOLD_ACT_INTERP_S ) ;
	}

	hld->count ++ ;
}

static	void	ThinkSaluteReady( hld )
HOLD_ENE	*hld ;
{
	if( Notice( hld ) ) 		return ;
	if( PoseType( hld ) ) 		return ;
	if( AimType( hld ) ) 		return ;

	if( hld->count == 0 ) {
		hld->p_time = KR_RandU( COUNT_VMODE(5) ) ;
	}

	if( hld->count == hld->p_time ) {
		SetMotion( hld, USHOLD_listen_attention_salute_crisp, HOLD_ACT_INTERP_S ) ;
	}

	if ( hld->p_time+COUNT_VMODE(5) == hld->count ) {
		GM_SeSetMode( se_kinuzure[hld->id%8] , &hld->mov, GM_SEMODE_BOMB ) ;
//		GM_SeSetMode( se_asioto[hld->id%4] , &hld->mov, GM_SEMODE_BOMB ) ;
	}
	if ( hld->p_time+COUNT_VMODE(15) == hld->count ) {
//		GM_SeSetMode( se_gun_noise[hld->id%8] , &hld->mov, GM_SEMODE_BOMB ) ;
	}

	if ( hld->count > hld->p_time ) {
		if ( hld->p_time + WORKP->m_len[hld->mot_num] - 2 == hld->count ) {
			hld->think2 = TH2_IDLE ;
			hld->count = 0 ;
			return ;
		}
	}

	hld->count ++ ;
}

static	void	ThinkSaluteIdle( hld )
HOLD_ENE	*hld ;
{
	if( Notice( hld ) ) 		return ;
	if( PoseType( hld ) ) 		return ;
	if( AimType( hld ) ) 		return ;

	if( hld->count == 0 ) {
		SetMotion( hld, USHOLD_listen_attention_salute_crisp_idle, HOLD_ACT_INTERP_S ) ;
	}

	hld->count ++ ;
}

static	void	ThinkReturnSaluteReady( hld )
HOLD_ENE	*hld ;
{
	if( Notice( hld ) ) 		return ;
	if( PoseType( hld ) ) 		return ;
	if( AimType( hld ) ) 		return ;

	if( hld->count == 0 ) {
		hld->p_time = KR_RandU( COUNT_VMODE(10) ) ;
	}

	if( hld->count == hld->p_time ) {
		SetMotion( hld, USHOLD_listen_attention_salute_crisp_end, HOLD_ACT_INTERP_S ) ;
	}

	if ( hld->p_time+COUNT_VMODE(5) == hld->count ) {
		GM_SeSetMode( se_kinuzure[hld->id%8] , &hld->mov, GM_SEMODE_BOMB ) ;
//		GM_SeSetMode( se_asioto[hld->id%4] , &hld->mov, GM_SEMODE_BOMB ) ;
	}
	if ( hld->p_time+COUNT_VMODE(15) == hld->count ) {
//		GM_SeSetMode( se_gun_noise[hld->id%8] , &hld->mov, GM_SEMODE_BOMB ) ;
	}

	if ( hld->count > hld->p_time ) {
		if ( hld->p_time + WORKP->m_len[hld->mot_num] - 2 == hld->count ) {
			hld->think2 = TH2_IDLE ;
			hld->count = 0 ;
			return ;
		}
	}

	hld->count ++ ;
}

static	void	ThinkReturnSaluteIdle( hld )
HOLD_ENE	*hld ;
{
	if( Notice( hld ) ) 		return ;
	if( PoseType( hld ) ) 		return ;
	if( AimType( hld ) ) 		return ;

	if( hld->count == 0 ) {
		SetMotion( hld, USHOLD_listen_attention_idle, HOLD_ACT_INTERP_S ) ;
	}

	hld->count ++ ;
}

static	void	ThinkTurn( hld )
HOLD_ENE	*hld ;
{
	if( Notice( hld ) ) 		return ;
	if( AimType( hld ) ) 		return ;

	if( hld->count == 0 ) {
//		hld->p_time = KR_RandU( COUNT_VMODE(30) ) ;
		hld->p_time = KR_RandU( COUNT_VMODE(15) ) ;
	}

//printf("turn: count[%d] time[%d]\n",hld->count, hld->p_time ) ;
	if( hld->count == hld->p_time ) {
		int	dirs ;

		dirs = GV_DiffDirS( hld->rot.vy, hld->aim_dir ) ;

#if 1	/* 向き替え用の補完モーションがあれば使う */
		if ( dirs > 2 ) {
			SetMotion( hld, USHOLD_usa_m4a_listen_speech_l_turn, HOLD_ACT_INTERP_M ) ;
		} else if ( dirs < -2 ) {
			SetMotion( hld, USHOLD_usa_m4a_listen_speech_r_turn, HOLD_ACT_INTERP_M ) ;
		} else {
		}
#endif
	}

	if ( hld->p_time+COUNT_VMODE(10) == hld->count ) {
		GM_SeSetMode( se_kinuzure[hld->id%8] , &hld->mov, GM_SEMODE_BOMB ) ;
	}
	if ( hld->p_time+COUNT_VMODE(15) == hld->count ) {
		GM_SeSetMode( se_gun_noise[hld->id%8] , &hld->mov, GM_SEMODE_BOMB ) ;
		hld->turn_y = hld->aim_dir ;
	}
	if ( hld->p_time+COUNT_VMODE(30) == hld->count ) {
//		GM_SeSetMode( se_asioto[hld->id%4] , &hld->mov, GM_SEMODE_BOMB ) ;
	}


	if ( hld->count > hld->p_time ) {
		if ( hld->p_time + WORKP->m_len[hld->mot_num]-1 == hld->count ) {
//			SetMotion( hld, NextMotion( hld ), HOLD_ACT_INTERP_M ) ;
			SetNextMotion( hld ) ;
			hld->think = TH_STAND ;
			hld->count = 0 ;
			return ;
		}
	}

	hld->count ++ ;
}

static	void	ThinkNoise( hld )
HOLD_ENE	*hld ;
{
	if( hld->count & COUNT_VMODE(60*3) ) {
		hld->think = TH_ACTIVE ;
		hld->count = 0 ;
		return ;
	}
	
	hld->count ++ ;
}

static	void	ThinkProjector( hld )
HOLD_ENE	*hld ;
{
	if( hld->count == 0  ) {
		HeadMarkRun(&BODYWORLD( &hld->body[hld->c_obj], HUMAN21_ATAMA ), 5 ) ;
	}

	if( hld->count == COUNT_VMODE(60)  ) {
		FVECTOR	pos, vec ;

		KR_FMatToFvec( &BODYWORLD( &hld->body[hld->c_obj], HUMAN21_KOSHI), &pos ) ;
		_sceVu0SubVector(  &vec, &GM_PlayerPosition, &pos ) ;
		hld->p_dir = _FVecDir2( &vec ) ;

		hld->think = TH_DETECT ;
		hld->p_time = 0 ;	/* 振り向き始める時間 */

		hld->count = 0 ;
		return ;
	}
	
	hld->count ++ ;
}

static	void	ThinkNeckReady( hld )
HOLD_ENE	*hld ;
{
	if( hld->count == 0 ) {
		hld->p_time = KR_RandU( COUNT_VMODE(5) ) ;
	}

	if ( hld->p_time == hld->count ) {
		SetMotion( hld, NECK_READY_MOT, HOLD_ACT_INTERP_S ) ;
	}

	if ( hld->count > hld->p_time ) {
		if ( hld->p_time + WORKP->m_len[hld->mot_num]-1 == hld->count ) {
			hld->think2 = TH2_IDLE ;
			hld->count = 0 ;
			return ;
		}
	}
	hld->count ++ ;
}

static	void	ThinkNeckIdle( hld )
HOLD_ENE	*hld ;
{
	if ( hld->count == 0 ) {
	}

	switch ( WORKP->pose_type ) {
		case GSCN_NECK_RIGHT :
			hld->adj_turn_y = -900 ;
			break ;
		case GSCN_NECK_LEFT :
			hld->adj_turn_y = 900 ;
			break ;
		case GSCN_NECK_DOWN :
			hld->adj_turn_x = 800 ;
			break ;
		case GSCN_NECK_UP :
			hld->adj_turn_x = -800 ;
			break ;
	}

	if( Notice( hld ) ) 	return ;
	if( PoseType( hld ) ) 	return ;

	hld->count ++ ;
}

static	void	ThinkJokeIdle( hld )
HOLD_ENE	*hld ;
{
	if( Notice( hld ) ) 		return ;

	if ( hld->count == 0 ) {
		hld->p_time = 0 ;
	}
	
	if ( hld->p_time == hld->count ) {
		int r ;

		hld->p_time += COUNT_VMODE((60 + KR_RandU( 120 ))) ;
		r = 2 ;
		if ( hld->count < 400  ) r = KR_RandU( 5 ) ;

		switch ( r ) {
			case 0 :
				hld->turn_y = hld->aim_dir - 512 - KR_RandU( 512 ) ;
				break ;
			case 1 :
				hld->turn_y = hld->aim_dir - KR_RandU( 512 ) ;
				break ;
			case 2 :
//				hld->turn_y = hld->aim_dir ;
				switch ( WORKP->aim_type ) {
					case GSCN_SCREEN2 :
						hld->turn_y = hld->aim_dir = hld->screen2_dir ;
						break ;
					case GSCN_SCOT :
						hld->turn_y = hld->aim_dir = _FVecTrgDir2( &hld->pos, WORKP->scot_pos ) ;
						break ;
					default :
						hld->turn_y = hld->aim_dir = hld->screen1_dir ;
						break ;
				}
				break ;
			case 3 :
				hld->turn_y = hld->aim_dir + KR_RandU( 512 ) ;
				break ;
			case 4 :
				hld->turn_y = hld->aim_dir + 512 + KR_RandU( 512 ) ;
				break ;
		}
	}


if ( hld->count>600  ) {
	if( Notice( hld ) ) 	return ;
	if( PoseType( hld ) ) 	return ;
}

	hld->count ++ ;
}

static	void	ThinkAlertStart( hld )
HOLD_ENE	*hld ;
{
	if( Notice( hld ) ) 		return ;

	if( hld->count == 0 ) {
		hld->p_time = KR_RandU( COUNT_VMODE(5) ) ;
	}

	if ( hld->p_time == hld->count ) {
		SetMotion( hld, USHOLD_usa_m4a_listen_speech2cle, 3 ) ;
	}

	if ( hld->count > hld->p_time + _GUN_HAND ) {
		hld->think2 = TH2_READY ;
		hld->count = 0 ;
		hld->tmp_count = 0 ;
		return ;
	}
	hld->count ++ ;
}

static	void	ThinkAlertReady( hld )
HOLD_ENE	*hld ;
{
	int		dirs ;

	if( Notice( hld ) ) 		return ;

	if( hld->count == 0 ) {
		hld->p_time = KR_RandU( COUNT_VMODE(5) ) ;
		HeadMarkRun(&BODYWORLD( &hld->body[hld->c_obj], HUMAN21_ATAMA ), 0 ) ;
//		GM_SeSetMode( se_bikkuri[hld->id%4] , &hld->mov, GM_SEMODE_BOMB ) ;
		GM_SeSetMode( SD_E_BIKKRI01, &hld->mov, GM_SEMODE_BOMB ) ;
	}

	if ( hld->p_time == hld->count ) {
		dirs = GV_DiffDirS( hld->rot.vy, hld->aim_dir ) ;
	
		if ( dirs > 1024 ) {
			SetMotion( hld, LEFT_TURN_MOT, HOLD_ACT_INTERP_S ) ;
			hld->turn_y = hld->aim_dir  - 1024 ;
		} else if ( dirs < -1024 ) {
			SetMotion( hld, RIGHT_TURN_MOT, HOLD_ACT_INTERP_S ) ;
			hld->turn_y = hld->aim_dir  + 1024 ;
		} else {
			SetMotion( hld, NO_TURN_MOT, HOLD_ACT_INTERP_S ) ;
			hld->turn_y = hld->aim_dir ;
		}
	}

	if ( hld->p_time+COUNT_VMODE(5) == hld->count ) {
		GM_SeSetMode( se_kinuzure[hld->id%8] , &hld->mov, GM_SEMODE_BOMB ) ;
	}
	if ( hld->p_time+COUNT_VMODE(15) == hld->count ) {
		GM_SeSetMode( se_gun_noise[hld->id%8] , &hld->mov, GM_SEMODE_BOMB ) ;
	}
	if ( hld->p_time+COUNT_VMODE(20) == hld->count ) {
//		GM_SeSetMode( se_asioto[hld->id%4] , &hld->mov, GM_SEMODE_BOMB ) ;
	}

	if ( hld->count > hld->p_time ) {
		if ( (hld->p_time + WORKP->m_len[hld->mot_num]-1) == hld->count ) {
			hld->think2 = TH2_IDLE ;
			hld->count = 0 ;
			return ;
		}
	}
	hld->count ++ ;
}

static	void	ThinkAlertIdle( hld )
HOLD_ENE	*hld ;
{
	if ( hld->count == 0 ) {
		SetMotion( hld, hld->mot_num+1, 1 ) ;
	}

	if( Notice( hld ) ) 	return ;

	if ( (WORKP->pose_type == GSCN_GUN_RIGHT) || (WORKP->pose_type == GSCN_GUN_LEFT) ||
		(WORKP->pose_type == GSCN_GUN_BACK) || (WORKP->pose_type == GSCN_GUN_FORWARD) ) {
		if( PoseType( hld ) ) 	return ;
	} else {
		hld->think2 = TH2_RETURN ;
		hld->count = 0 ;
		return ;
	}

	hld->count ++ ;
}

static	void	ThinkAlertReturn ( hld )
HOLD_ENE	*hld ;
{
	if ( hld->count == 0 ) {
		SetMotion( hld, USHOLD_usa_m4a_listen_speech2cle, HOLD_ACT_INTERP_S ) ;
		hld->reverse_flag = 1 ;
		hld->m_time = WORKP->m_len[hld->mot_num] - _GUN_HAND ;
	}

	if( Notice( hld ) ) 	return ;

	if ( hld->count >= _GUN_HAND ) {
		if( PoseType( hld ) ) 	return ;
	}
	if ( hld->count == _GUN_HAND ) {
		SetMotion( hld, USHOLD_idle_1, 3 ) ;
printf("kore alert return ?");
	}

	hld->count ++ ;
}

static	void	ThinkActive( hld )
HOLD_ENE	*hld ;
{
	SET_FLAG( hld->status2, HLD_STATUS2_ACTIVE ) ;

	if ( hld->status & HLD_STATUS_ACTIVE_END ) {
		ChangePassive( hld ) ;
		if ( hld->status&HLD_STATUS_CAMERAMN ) {
			SetMotion( hld, USHOLD_usl_fold_cameraman_idle, HOLD_ACT_INTERP_S ) ;
			hld->think = TH_CAMERA ;
			hld->think2 = TH2_IDLE ;
		} else {
			hld->think = TH_STAND ;
		}
		hld->count = 0 ;
	}
	if ( hld->status & HLD_STATUS_DOWN_DETECT ) {
		LighterProc( hld ) ;
printf(" holdene: lighter detect holdene \n") ; 
		return ;
	}
	if ( hld->status & HLD_STATUS_NORMAL_DETECT ) {
		NormalDetectProc( hld ) ;
printf(" holdene: normal detect holdene \n") ; 
		return ;
	}
	if ( hld->status & HLD_STATUS_SCOT_DETECT ) {
		ScotProc( hld ) ;
printf(" holdene: scot detect holdene \n") ; 
		return ;
	}
	if ( hld->status & HLD_STATUS_DEATH ) {
		DeathProc( hld ) ;
printf(" holdene: death detect holdene \n") ; 
		return ;
	}

	hld->count ++ ;
}

static	void	ThinkDetect( hld )
HOLD_ENE	*hld ;
{
	int		dir, dirs ;

	if ( hld->count == 0 ) {
		hld->p_time += _GUN_HAND ;
	}

	if ( hld->count == (hld->p_time -_GUN_HAND)  ) {
		SetMotion( hld, USHOLD_usa_m4a_listen_speech2cle, 3 ) ;
	}

	if ( hld->p_time == hld->count ) {
		dir = _FVecTrgDir2( &hld->mov, &GM_PlayerPosition ) ;
		dirs = GV_DiffDirS( hld->rot.vy, dir ) ;
		
		if ( dirs > 1024 ) {
			SetMotion( hld, LEFT_TURN_MOT, HOLD_ACT_INTERP_S ) ;
			hld->turn_y = dir  - 1024 ;
		} else if ( dirs < -1024 ) {
			SetMotion( hld, RIGHT_TURN_MOT, HOLD_ACT_INTERP_S ) ;
			hld->turn_y = dir  + 1024 ;
		} else {
			SetMotion( hld, NO_TURN_MOT, HOLD_ACT_INTERP_S ) ;
			hld->turn_y = dir ;
		}

		HeadMarkRun(&BODYWORLD( &hld->body[hld->c_obj], HUMAN21_ATAMA ), 0 ) ;
		GM_SeSetMode( se_bikkuri[hld->id%4] , &hld->mov, GM_SEMODE_BOMB ) ;
//		GM_SeSetMode( SD_E_BIKKRIS11 , &hld->mov, GM_SEMODE_BOMB ) ;
	}

	if ( hld->p_time+COUNT_VMODE(5) == hld->count ) {
		GM_SeSetMode( se_kinuzure[hld->id%8] , &hld->mov, GM_SEMODE_BOMB ) ;
	}
	if ( hld->p_time+COUNT_VMODE(15) == hld->count ) {
		GM_SeSetMode( se_gun_noise[hld->id%8] , &hld->mov, GM_SEMODE_BOMB ) ;
	}
	if ( hld->p_time+COUNT_VMODE(20) == hld->count ) {
//		GM_SeSetMode( se_asioto[hld->id%4] , &hld->mov, GM_SEMODE_BOMB ) ;
	}

	if ( hld->count > hld->p_time ) {
		if ( (hld->p_time + WORKP->m_len[hld->mot_num]-1) == (hld->count+_GUN_HAND) ) {
			hld->think = TH_END ;
			hld->count = 0 ;
			return ;
		}
	}
	hld->count ++ ;
}

static	void	ThinkEnd( hld )
HOLD_ENE	*hld ;
{
	if ( hld->count == 0 ) {
		SetMotion( hld, hld->mot_num+1, 1 ) ;
	}

#if 0
	if ( hld->count > COUNT_VMODE(300) ) {
		if ( !(GM_GameStatus & GM_STATUS_DETECT) ) {
			hld->think = TH_STAND ;
			hld->turn_y = _FVecTrgDir2( &hld->mov, &WORKP->screen_pos ) ;
			SetMotion( hld, NextMotion( hld ), HOLD_ACT_INTERP_M ) ;
			hld->count = 0 ;
			return ;
		}
	}
#endif

	hld->count ++ ;
}


static	void	ThinkSleepReady( HOLD_ENE *hld )
{
	if ( SleepNotice( hld ) ) return ;

	if ( hld->count == 0 ) {
		HeadMarkRun(&BODYWORLD( &hld->body[hld->c_obj], HUMAN21_ATAMA ), 4 ) ;

		/* モーションは１種類と決め付け */
//printf("hld->mot_type=%d motion_table[%d]\n",hld->mot_type, motion_table[ hld->mot_type ][0] ) ;
//hld->mot_num = 30 ;

		SetMotion2( hld, motion_table[ hld->mot_type ][0], HOLD_ACT_INTERP_M ) ;
	}
/*
	if ( hld->count < COUNT_VMODE(60) ) hld->adj_turn_x = 400 ;
	else if ( hld->count < COUNT_VMODE(90) ) hld->adj_turn_x = 200 ;
	else if ( hld->count < COUNT_VMODE(150) ) hld->adj_turn_x = 400 ;
	else if ( hld->count < COUNT_VMODE(180) ) hld->adj_turn_x = 200 ;
	else hld->adj_turn_x = 800 ;
*/
	if ( hld->count > COUNT_VMODE(60) ) {
		GV_SetActorChild( WORKP, hld->headmark = NewSleep(&BODYWORLD( &hld->body[hld->c_obj], HUMAN21_ATAMA ), &WORKP->hzx_id) ) ;
		hld->think2 = TH2_SLEEP ;
		hld->count = 0 ;
	}
	
	hld->count ++ ;
}

static	void	ThinkSleep( HOLD_ENE *hld )
{
	if ( SleepNotice( hld ) ) return ;

	SET_FLAG( hld->status2, HLD_STATUS2_EYE_CLOSE ) ;
	SET_FLAG( hld->status2, HLD_STATUS2_EAR_CLOSE ) ;
	SET_FLAG( hld->status2, HLD_STATUS2_SLEEPING ) ;
	hld->adj_turn_x = 400 ;

	hld->count ++ ;
}

static	FVECTOR	shift={0.0f, 0.0f, -750.0f} ;
static	void	SetCameramanPos( HOLD_ENE *hld, FVECTOR *pos )
{
	OBJECT *cam ;
	int num ;
	FMATRIX m ;
	
	num = (WORKP->cameraman[0] == hld->id) ? 0 : 1 ;

	cam = HLD_GetCameraObject( num ) ;
	DG_SetPos( &(cam->objs->objs[0].world) ) ;
	DG_MovePos( &shift ) ;
	DG_GetPos( &m ) ;
	KR_FMatToFvec( &m, pos ) ;
}

static	void	ThinkCamera( hld )
HOLD_ENE	*hld ;
{
	FVECTOR pos ;

	if( hld->count == 0 ) {
		SetMotion( hld, USHOLD_usl_fold_cameraman_idle, HOLD_ACT_INTERP_S ) ;
		hld->turn_y = _FVecTrgDir2( &hld->mov, WORKP->scot_pos ) ;
	}

//	if( Notice( hld ) ) 	return ;
//	if( PoseType( hld ) ) 	return ;
//	if( AimType( hld ) ) 	return ;

	SetCameramanPos( hld, &pos ) ;

//if ( WORKP->cameraman[0] == hld->id ) printf("a+ p[%f][%f] m[%f][%f]\n",pos.vx,pos.vz,hld->mov.vx,hld->mov.vz);
	if ( !(_PosInRangeXZ( &pos, &hld->mov, 60.0f )) ) {
		hld->think2 = TH2_MOVE ;
		hld->count = 0 ;
		return ;
	}
//if ( WORKP->cameraman[0] == hld->id ) printf("a+\n");

	hld->count ++ ;
}

static	void	ThinkCameraMove( hld )
HOLD_ENE	*hld ;
{
	FVECTOR pos ;

	if( hld->count == 0 ) {
		SetMotion( hld, USHOLD_usl_fold_cameraman_pan_r, HOLD_ACT_INTERP_S ) ;
		hld->turn_y = _FVecTrgDir2( &hld->mov, WORKP->scot_pos ) ;
	}

//	if( Notice( hld ) ) 	return ;
//	if( PoseType( hld ) ) 	return ;

//	if( AimType( hld ) ) 	return ;

	SetCameramanPos( hld, &pos ) ;

//if ( WORKP->cameraman[0] == hld->id ) printf("b+ p[%f][%f] m[%f][%f]\n",pos.vx,pos.vz,hld->mov.vx,hld->mov.vz);

	if ( _PosInRangeXZ( &pos, &hld->mov, 30.0f ) ) {
		hld->think2 = TH2_IDLE ;
		hld->count = 0 ;
		return ;
	}

	if ( hld->mov.vx > pos.vx ) hld->mov.vx -= 2.0F ;
	if ( hld->mov.vx < pos.vx ) hld->mov.vx += 2.0F ;
	if ( hld->mov.vz > pos.vz ) hld->mov.vz -= 2.0F ;
	if ( hld->mov.vz < pos.vz ) hld->mov.vz += 2.0F ;

	hld->turn_y = _FVecTrgDir2( &hld->mov, WORKP->scot_pos ) ;

	hld->count ++ ;
}

/*----------------------------------------------------------------*/
static	void	Think2Attention( hld )
HOLD_ENE	*hld ;
{
	switch( hld->think2 ) {
		case TH2_READY :
			ThinkAttentionReady( hld ) ;
			break ;
		case TH2_IDLE :
			ThinkAttentionIdle( hld ) ;
			break ;
	}
}

static	void	Think2Salute( hld )
HOLD_ENE	*hld ;
{
	switch( hld->think2 ) {
		case TH2_READY :
			ThinkSaluteReady( hld ) ;
			break ;
		case TH2_IDLE :
			ThinkSaluteIdle( hld ) ;
			break ;
	}
}

static	void	Think2ReturnSalute( hld )
HOLD_ENE	*hld ;
{
	switch( hld->think2 ) {
		case TH2_READY :
			ThinkReturnSaluteReady( hld ) ;
			break ;
		case TH2_IDLE :
			ThinkReturnSaluteIdle( hld ) ;
			break ;
	}
}

static	void	Think2Neck( hld )
HOLD_ENE	*hld ;
{
	switch( hld->think2 ) {
		case TH2_READY :
			ThinkNeckReady( hld ) ;
			break ;
		case TH2_IDLE :
			ThinkNeckIdle( hld ) ;
			break ;
	}
}

static	void	Think2Joke( hld )
HOLD_ENE	*hld ;
{
	switch( hld->think2 ) {
		case TH2_IDLE :
			ThinkJokeIdle( hld ) ;
			break ;
	}
}

static	void	Think2Camera( hld )
HOLD_ENE	*hld ;
{
	switch( hld->think2 ) {
		case TH2_IDLE :
			ThinkCamera( hld ) ;
			break ;
		case TH2_MOVE :
			ThinkCameraMove( hld ) ;
			break ;
	}
}

static	void	Think2Alert( hld )
HOLD_ENE	*hld ;
{
	switch( hld->think2 ) {
		case TH2_START :
			ThinkAlertStart( hld ) ;
		break ;
		case TH2_READY :
			ThinkAlertReady( hld ) ;
			break ;
		case TH2_IDLE :
			ThinkAlertIdle( hld ) ;
			break ;
		case TH2_RETURN :
			ThinkAlertReturn( hld ) ;
			break ;
	}
}

static	void	ThinkDamage( hld )
HOLD_ENE	*hld ;
{
	switch( hld->think2 ) {
		case TH2_SLEEP_READY :
			ThinkSleepReady( hld ) ;
			break ;
		case TH2_SLEEP :
			ThinkSleep( hld ) ;
			break ;
	}
}

static	void	Think( hld )
HOLD_ENE	*hld ;
{
	switch( hld->think ) {
		case TH_STAND :
			ThinkStand( hld ) ;
			break ;
		case TH_ATTENTION :
			Think2Attention( hld ) ;
			break ;
		case TH_SALUTE :
			Think2Salute( hld ) ;
			break ;
		case TH_RETURN_SALUTE :
			Think2ReturnSalute( hld ) ;
			break ;
		case TH_TURN :
			ThinkTurn( hld ) ;
			break ;
		case TH_NOISE :
			ThinkNoise( hld ) ;
		break ;
		case TH_PROJECTOR :
			ThinkProjector( hld ) ;
		break ;

		/* 首の運動 */
		case TH_NECK :
			Think2Neck( hld ) ;
		break ;

		/* アメリカンジョーク */
		case TH_JOKE :
			Think2Joke( hld ) ;
		break ;

		/* カメラマン */
		case TH_CAMERA :
			Think2Camera( hld ) ;
		break ;

		/* 銃を構えて警戒 */
		case TH_ALERT :
			Think2Alert( hld ) ;
		break ;

		case TH_ACTIVE :
			ThinkActive( hld ) ;
		break ;
		case TH_DETECT :
			ThinkDetect( hld ) ;
		break ;
		case TH_END :
			ThinkEnd( hld ) ;
		break ;

		case TH_DAMAGE :
			ThinkDamage( hld ) ;
		break ;
	}
}
/*----------------------------------------------------------------*/
static void ActSpeechToCle( HOLD_ENE *hld )
{
	int m_time ;

	m_time = hld->m_time%WORKP->m_len[hld->mot_num] ;
	
	if ( hld->reverse_flag ) {
//if ( hld->id == 0 )printf("To Cle m_time [%d]\n",WORKP->m_len[hld->mot_num] - m_time ) ;
		if ( (WORKP->m_len[hld->mot_num] - m_time) == COUNT_VMODE(9) ) {
			SET_FLAG( hld->status2, HLD_STATUS2_GUN_LEFT ) ;
			GM_SeSetMode( se_kinuzure[hld->id%8] , &hld->mov, GM_SEMODE_BOMB ) ;
		}
		if ( (WORKP->m_len[hld->mot_num] - m_time) == COUNT_VMODE(1) ) {
			SET_FLAG( hld->status2, HLD_STATUS2_GUN_SHOLDER ) ;
			GM_SeSetMode( se_kinuzure[hld->id%8] , &hld->mov, GM_SEMODE_BOMB ) ;
		}
	} else {
		if ( m_time == COUNT_VMODE(2) ) {
			SET_FLAG( hld->status2, HLD_STATUS2_GUN_LEFT ) ;
			GM_SeSetMode( se_kinuzure[hld->id%8] , &hld->mov, GM_SEMODE_BOMB ) ;
		}
		if ( m_time == COUNT_VMODE(9) ) {
			SET_FLAG( hld->status2, HLD_STATUS2_GUN_RIGHT ) ;
			GM_SeSetMode( se_kinuzure[hld->id%8] , &hld->mov, GM_SEMODE_BOMB ) ;
		}
	}

}


static void Action( HOLD_ENE *hld )
{
	switch( hld->mot_num ) {
		case USHOLD_usa_m4a_listen_speech2cle :
			ActSpeechToCle( hld ) ;
		break ;
		case USHOLD_look_r :
		case USHOLD_look_r_idle :
		case USHOLD_look_l :
		case USHOLD_look_l_idle :
		case USHOLD_look_f :
		case USHOLD_look_f_idle :
			SET_FLAG( hld->status2, HLD_STATUS2_GUN_RIGHT ) ;
		break ;
		default :
			SET_FLAG( hld->status2, HLD_STATUS2_GUN_SHOLDER ) ;
		break ;
	}
}


/*----------------------------------------------------------------*/
/* 全てのダメージフラグをクリア */
static void DamageFlagClear( HOLD_ENE *hld )
{
	TARGET	*def ;
	int i ;
	
	def = &(hld->bodyparam.def_child1[0]) ;
	for( i=0; i<PTARGET_LEVEL1_NUM; i++ ) {
		def->damaged = FLAG_CLEAR ;
		def->weapon_type = 0 ;
		def++ ;
	}
	def = &(hld->bodyparam.def_child2[0]) ;
	for( i=0; i<PTARGET_LEVEL2_NUM; i++ ) {
		def->damaged = FLAG_CLEAR ;
		def->weapon_type = 0 ;
		def++ ;
	}
	def = &(hld->bodyparam.def_child3[0]) ;
	for( i=0; i<PTARGET_LEVEL3_NUM; i++ ) {
		def->damaged = FLAG_CLEAR ;
		def->weapon_type = 0 ;
		def++ ;
	}
	def = &(hld->bodyparam.deftrg) ;
	def->damaged = FLAG_CLEAR ;
	def->weapon_type = 0 ;
}

static void CaptureFlagClear( HOLD_ENE *hld )
{
	hld->bodyparam.capture.capture = NULL ;

	if ( hld->items & ITEM_KAITAIC4 ) {
		hld->bodyparam.capture.flag &= CAPTURE_C4EXIST ;
	} else {
		hld->bodyparam.capture.flag = 0 ;
	}
}

static int ChildTargetCheck( child, n )
TARGET	*child ;
int		n ;
{
	int i ;

	for( i=0; i<n; i++ ) {
		if ( TARGET_POWER & child->damaged ) {
			return i ;
		} else {
			child->weapon_type = 0 ;
		}
		child++ ;
	}
	return -1 ;
}

static	void	TargetCheck( HOLD_ENE *hld )
{
	TARGET	*def ;
	long64	weapon ;
	int child_num ;
	PTARGET_INFO	*pinfo ;

	if ( WORKP->mode == MODE_DEMO ) return ;

	def = &(hld->bodyparam.def_child1[0]) ;
	pinfo = Ptarg.info[PTARGET_LEVEL1] ;
	if ( (child_num = ChildTargetCheck( def, PTARGET_LEVEL1_NUM )) >= 0 ) {
		def += child_num ;
		pinfo += child_num ;
		weapon = def->weapon_type ;
		if ( weapon & WP_M92 ) {
			SET_FLAG( hld->status2, HLD_STATUS2_SLEEP ) ;
			def->weapon_type = 0 ;
			def->damaged = 0 ;
#ifdef HOLD_SET_NEEDL
			ENE_SetNeedlV( &hld->body[0], pinfo->obj_num, &hld->bodyparam.off_center ) ;
#endif
		} else {
			SET_FLAG( hld->status2, HLD_STATUS2_DAMAGE ) ;
		}
		return ;
	}

	def = &(hld->bodyparam.def_child2[0]) ;
	pinfo = Ptarg.info[PTARGET_LEVEL2] ;
	if ( (child_num = ChildTargetCheck( def, PTARGET_LEVEL2_NUM )) >= 0 ) {
		def += child_num ;
		pinfo += child_num ;
		weapon = def->weapon_type ;
		if ( weapon & WP_M92 ) {
			SET_FLAG( hld->status2, HLD_STATUS2_SLEEP ) ;
			def->weapon_type = 0 ;
			def->damaged = 0 ;
#ifdef HOLD_SET_NEEDL
			ENE_SetNeedlV( &hld->body[0], pinfo->obj_num, &hld->bodyparam.off_center ) ;
#endif
		} else {
			SET_FLAG( hld->status2, HLD_STATUS2_DAMAGE ) ;
		}
		return ;
	}

	def = &(hld->bodyparam.def_child3[0]) ;
	pinfo = Ptarg.info[PTARGET_LEVEL3] ;
	if ( (child_num = ChildTargetCheck( def, PTARGET_LEVEL3_NUM )) >= 0 ) {
		def += child_num ;
		pinfo += child_num ;
		weapon = def->weapon_type ;
		if ( weapon & WP_M92 ) {
			SET_FLAG( hld->status2, HLD_STATUS2_SLEEP ) ;
			def->weapon_type = 0 ;
			def->damaged = 0 ;
#ifdef HOLD_SET_NEEDL
			ENE_SetNeedlV( &hld->body[0], pinfo->obj_num, &hld->bodyparam.off_center ) ;
#endif
		} else {
			SET_FLAG( hld->status2, HLD_STATUS2_DAMAGE ) ;
		}
		return ;
	}

	def = &(hld->bodyparam.deftrg) ;
	if ( TARGET_POWER & def->damaged ) {
		weapon = def->weapon_type ;
		if ( weapon & WP_M92 ) {
			SET_FLAG( hld->status2, HLD_STATUS2_SLEEP ) ;
			def->weapon_type = 0 ;
			def->damaged = 0 ;
#ifdef HOLD_SET_NEEDL
			ENE_SetNeedlV( &hld->body[0], HUMAN21_MUNE, &hld->bodyparam.off_center ) ;
#endif
		} else {
			SET_FLAG( hld->status2, HLD_STATUS2_DAMAGE ) ;
		}
		return ;
	}
	if ( TARGET_TOUCH & def->damaged ) {
		weapon = def->weapon_type ;
		def->weapon_type = 0 ;
		def->damaged = 0 ;
		SET_FLAG( hld->status2, HLD_STATUS2_TOUCH ) ;
	}

#if 1
	if ( GM_PlayerStatus & PLAYER_GROUND ) {
		FVECTOR floor_pos ;
		
		floor_pos = hld->mov ;
		
		if ( ENE_InRangeXYZ( &floor_pos, &WORKP->touch_pos[0], 250, 1100, 250 ) ) {
			SET_FLAG( hld->status2, HLD_STATUS2_TOUCH ) ;
		}
		if ( ENE_InRangeXYZ( &floor_pos, &WORKP->touch_pos[1], 250, 1100, 250 ) ) {
			SET_FLAG( hld->status2, HLD_STATUS2_TOUCH ) ;
		}
		if ( ENE_InRangeXYZ( &floor_pos, &WORKP->touch_pos[2], 250, 1100, 250 ) ) {
			SET_FLAG( hld->status2, HLD_STATUS2_TOUCH ) ;
		}
	}
#endif
}

static	void	CaptureCheck( HOLD_ENE *hld )
{
    TARGET			*def ;	/* 防御ターゲット */
	CAPTURE_TARGET	*cap ;

	def = &(hld->bodyparam.deftrg) ;
	if ( TARGET_CAPTURE & def->damaged ) {
		hld->status2 |= HLD_STATUS2_CAPTURE ;	/* 首締められた */
printf( "GV_Time[%d] capture %x !!1\n",GV_Time,def->class ) ;
		cap = &(hld->bodyparam.capture) ;
	    if ( cap->capture != NULL ) { /* 捕まった */
			if ( cap->flag & CAPTURE_HANG ) {	/* 首絞め */
			} else if ( cap->flag & CAPTURE_THROW ) {	/* 投げ */
			}
		}
		return ;
	}
}

static	void	DamageCheck( HOLD_ENE *hld )
{
	if ( WORKP->mode == MODE_DEMO ) return ;

	if ( hld->status & HLD_STATUS_ACTIVE ) {
		SET_FLAG( hld->status2, HLD_STATUS2_TARGET_SKIP ) ;
		return ;
	}

	TargetCheck( hld ) ;

	CaptureCheck( hld ) ;

	if ( hld->status2 & HLD_STATUS2_SLEEP ) {
		if ( hld->think == TH_DAMAGE ) {
			if ( (hld->think2 == TH2_SLEEP_READY) || (hld->think2 == TH2_SLEEP) ) return ;
		}
		hld->think = TH_DAMAGE ;
		hld->think2 = TH2_SLEEP_READY ;
		hld->count = 0 ;
		return ;
	}

	if ( hld->status2 & HLD_STATUS2_TOUCH ) {
		if ( ChangeActive( hld ) < 0 ) {
			UNSET_FLAG( hld->status2, HLD_STATUS2_TOUCH ) ;
		} else {
			hld->status |= HLD_STATUS_ACTIVE ;	/* 活動開始 */
			hld->think = TH_ACTIVE ;
			hld->count = 0 ;
		}
		DamageFlagClear( hld ) ;
		CaptureFlagClear( hld ) ;
		SET_FLAG( hld->status2, HLD_STATUS2_TARGET_SKIP ) ;
		return ;
	}

	if ( hld->status2 & HLD_STATUS2_DAMAGE ) {
		if ( ChangeActive( hld ) < 0 ) {
			UNSET_FLAG( hld->status2, HLD_STATUS2_DAMAGE ) ;
		} else {
			hld->status |= HLD_STATUS_ACTIVE ;	/* 活動開始 */
			hld->think = TH_ACTIVE ;
			hld->count = 0 ;
		}
		DamageFlagClear( hld ) ;
		CaptureFlagClear( hld ) ;
		SET_FLAG( hld->status2, HLD_STATUS2_TARGET_SKIP ) ;
		return ;
	}

	if ( hld->status2 & HLD_STATUS2_CAPTURE ) {
		if ( ChangeActive( hld ) < 0 ) {
			UNSET_FLAG( hld->status2, HLD_STATUS2_CAPTURE ) ;
		} else {
			hld->status |= HLD_STATUS_ACTIVE ;	/* 活動開始 */
			hld->think = TH_ACTIVE ;
			hld->count = 0 ;
		}
		DamageFlagClear( hld ) ;
		CaptureFlagClear( hld ) ;
		SET_FLAG( hld->status2, HLD_STATUS2_TARGET_SKIP ) ;
		return ;
	}
}

static	void	StatusInit( work )
Work	*work ;
{
	HOLD_ENE	*hld ;
	int	i ;

	work->event_ene = -1 ;

	hld = &work->holdene[0] ;

	for( i=0; i<work->ene_num; i++ ) {
		hld->adj_turn_y = hld->adj_turn_x = 0 ;
		hld->status2 = 0 ;
		hld++ ;
	}
}

enum {
	HOLD_MESSAGE_GSCN_NONE,

	HOLD_MESSAGE_GSCN_ROOT_CHANGE,

	HOLD_MESSAGE_GSCN_SCREEN2,

	HOLD_MESSAGE_GSCN_RIGHT,	/* 右を向く */
	HOLD_MESSAGE_GSCN_LEFT,		/* 左を向く */
	HOLD_MESSAGE_GSCN_BACK,		/* 後ろを向く */
	HOLD_MESSAGE_GSCN_FORWARD,	/* 前を向く */

	HOLD_MESSAGE_GSCN_GUN_RIGHT,	/* 右に銃を向ける */
	HOLD_MESSAGE_GSCN_GUN_LEFT,		/* 左に銃を向ける */
	HOLD_MESSAGE_GSCN_GUN_BACK,		/* 後に銃を向ける */
	HOLD_MESSAGE_GSCN_GUN_FORWARD,	/* 前に銃を向ける */

	HOLD_MESSAGE_GSCN_NECK_RIGHT,	/* 右に首を向ける */
	HOLD_MESSAGE_GSCN_NECK_LEFT,	/* 左に首を向ける */
	HOLD_MESSAGE_GSCN_NECK_DOWN,	/* 下に首を向ける */
	HOLD_MESSAGE_GSCN_NECK_FORWARD,	/* 前に首を向ける */
	HOLD_MESSAGE_GSCN_NECK_UP,		/* 上に首を向ける */

	HOLD_MESSAGE_GSCN_SCOT,			/* スコットを見る */
	HOLD_MESSAGE_GSCN_SCREEN1,

	HOLD_MESSAGE_JOKE,				/* 18:アメリカンジョーク */

	HOLD_MESSAGE_LIGHTER_DROP=64,	/* ライター落とす */
	HOLD_MESSAGE_ROUTE_CHANGE,		/* ルートチェンジ */
	HOLD_MESSAGE_PROJECT1_ACTIVE,
	HOLD_MESSAGE_PROJECT2_ACTIVE,
	HOLD_MESSAGE_PROJECT1_STOP,
	HOLD_MESSAGE_PROJECT2_STOP,

} ;

static	void	CheckMessage( work )
Work	*work ;
{
	GV_MSG *msg;
	int mes_num, scn ;

	scn = 0 ;

#ifdef DEBUG_MODE
if(0){
	if( GV_PadData[ 1 ].press & PAD_X ){
		work->pose_type = GSCN_GUN_BACK ;
printf("kroekroekroe1\n");
	}
}
#endif

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
printf("[%d] holdene: message = [%d]\n",mes_num,msg->message[0] ) ;
		switch( msg->message[0] ){
			case HOLD_MESSAGE_GSCN_NONE:
				scn = GSCN_NONE ;
			break;
			case HOLD_MESSAGE_GSCN_SCREEN1:
				scn = GSCN_SCREEN1 ;
			break;
			case HOLD_MESSAGE_GSCN_SCREEN2:
				scn = GSCN_SCREEN2 ;
			break;
			case HOLD_MESSAGE_GSCN_SCOT:
				scn = GSCN_SCOT ;
			break;

			case HOLD_MESSAGE_GSCN_RIGHT:
				scn = GSCN_RIGHT ;
			break;
			case HOLD_MESSAGE_GSCN_LEFT:
				scn = GSCN_LEFT ;
			break;
			case HOLD_MESSAGE_GSCN_BACK:
				scn = GSCN_BACK ;
			break;
			case HOLD_MESSAGE_GSCN_FORWARD:
				scn = GSCN_FORWARD ;
			break;

			case HOLD_MESSAGE_GSCN_GUN_RIGHT:
				scn = GSCN_GUN_RIGHT ;
			break;
			case HOLD_MESSAGE_GSCN_GUN_LEFT:
				scn = GSCN_GUN_LEFT ;
			break;
			case HOLD_MESSAGE_GSCN_GUN_BACK:
				scn = GSCN_GUN_BACK ;
			break;
			case HOLD_MESSAGE_GSCN_GUN_FORWARD:
				scn = GSCN_GUN_FORWARD ;
			break;

			case HOLD_MESSAGE_GSCN_NECK_RIGHT :	/* 右に首を向ける */
				scn = GSCN_NECK_RIGHT ;
			break;
			case HOLD_MESSAGE_GSCN_NECK_LEFT :	/* 左に首を向ける */
				scn = GSCN_NECK_LEFT ;
			break;
			case HOLD_MESSAGE_GSCN_NECK_DOWN :	/* 下に首を向ける */
				scn = GSCN_NECK_DOWN ;
			break;
			case HOLD_MESSAGE_GSCN_NECK_FORWARD :	/* 前に首を向ける */
				scn = GSCN_NECK_FORWARD ;
			break;
			case HOLD_MESSAGE_GSCN_NECK_UP :		/* 上に首を向ける */
				scn = GSCN_NECK_UP ;
			break;

			case HOLD_MESSAGE_JOKE :		/* 上に首を向ける */
				scn = GSCN_JOKE ;
			break;

			case HOLD_MESSAGE_PROJECT1_ACTIVE :		/* プロジェクタ１がアクティブ */
				SET_FLAG( work->projector1flag, FLAG_EYECHECK ) ;
			break;
			case HOLD_MESSAGE_PROJECT2_ACTIVE :		/* プロジェクタ２がアクティブ */
				SET_FLAG( work->projector2flag, FLAG_EYECHECK ) ;
			break;
			case HOLD_MESSAGE_PROJECT1_STOP :		/* プロジェクタ１がストップ */
				UNSET_FLAG( work->projector1flag, FLAG_EYECHECK ) ;
			break;
			case HOLD_MESSAGE_PROJECT2_STOP :		/* プロジェクタ２がストップ */
				UNSET_FLAG( work->projector2flag, FLAG_EYECHECK ) ;
			break;

			/* 個別メッセージ */
			case HOLD_MESSAGE_LIGHTER_DROP :		/* ライター落とす */
				work->event_id = HLD_STATUS2_LIGHTER ;
				work->event_ene = NameToID( work, msg->message[1] ) ;
//				work->event_ene = msg->message[1] ;
				work->event_pos.vx = (float)msg->message[2] ;
				work->event_pos.vy = (float)msg->message[3] ;
				work->event_pos.vz = (float)msg->message[4] ;
//printf("LighterDrop Enemy[%d] x[%d] y[%d] z[%d]\n",work->event_ene,msg->message[2],msg->message[3],msg->message[4] ) ;
			break;
		}

		if ( msg->message[0] < HOLD_MESSAGE_LIGHTER_DROP ) {
			if ( scn < GSCN_SCREEN1 ) {
				work->pose_type = scn ;
			} else {
				work->aim_type = scn ;
			}
			work->count = 0 ;
		}

		msg-- ;
	}
}

static	void	GameScnCheck( work )
Work	*work ;
{
	if ( (work->count+1)%JIKU_TIME1 == 0 ) {
		SetNextGameScn( work ) ;
	}
}

static	int	CameraDis( pos )
FVECTOR		*pos ;
{
	GM_CameraSet	*cam ;
	FVECTOR		vec ;
	float dis ;

	cam = GM_GetCurrentCamera( 0 ) ;	/* 子画面は１*/
//    _sceVu0SubVector( &vec, pos, &cam->target ) ;
    _sceVu0SubVector( &vec, pos, &cam->position ) ;
    dis = GV_VecLen3F( &vec ) ;
	if ( cam->angle == 0 ) {
		dis = 100000000000.0f ;
	} else {
		dis /= cam->angle/2 ;
	}

	return (int)dis ;
}

static	void	SetLod( work, hld )
Work		*work ;
HOLD_ENE	*hld ;
{
	int cam_dis, i, lod ;

	cam_dis = CameraDis( &hld->mov ) ;
	lod = 0 ;
   for ( i=0; i< MAX_LOD-1; i++ ) {
		if ( cam_dis > work->lod_th[ i ] ) lod = i+1 ;
	}

	if ( hld->items & ITEM_KAITAIC4 ) {
		lod = 0 ;
	}

	if ( hld->c_obj != lod ) {
		hld->c_obj = lod ;
		for ( i=0; i<MAX_LOD; i++ ) {
			if ( lod == i ) {
				hld->body[i].objs->flag &= ~DG_FLAG_INVISIBLE ;
			} else {
				hld->body[i].objs->flag |= DG_FLAG_INVISIBLE ;
			}
		}
		if ( hld->items & ITEM_WEAPON ) {
			CNCT_CnctObjObject( &hld->cnct, &hld->body[lod] ) ;
		}
		if ( hld->items & ITEM_ATAMA ) {
			hld->atama.objs->root = &( hld->body[lod].objs->objs[ HUMAN21_ATAMA ].world ) ;
		}
		if ( hld->items & ITEM_GLASS ) {
			hld->glass.objs->root = &( hld->body[lod].objs->objs[ HUMAN21_ATAMA ].world ) ;
		}
	}
}
	/*
		クォータニオンの線形補間（正規化しないので注意）
	*/
static	void HLD_QuatSlerp( FVECTOR *res, FVECTOR *from, FVECTOR *to, float t )
{
	float	to1[4] ;
	float	cosom  ;	/* 本当はdouble型の方がいい */
	float	scale0, scale1 ;		/* 本当はdouble型の方がいい */

	/* 内積を求める */
	cosom = from->vx * to->vx + from->vy * to->vy + from->vz * to->vz + from->vw * to->vw ;

	/* 符号をそろえる */
	if ( cosom < 0.0F ){
		cosom = -cosom ;
		to1[0] = - to->vx ;
		to1[1] = - to->vy ;
		to1[2] = - to->vz ;
		to1[3] = - to->vw ;
	} else  {
		to1[0] = to->vx ;
		to1[1] = to->vy ;
		to1[2] = to->vz ;
		to1[3] = to->vw ;
	}

	/* 係数を求める */
	/* ２つの角度の差が小さすぎるときには線形補間で求める */
	scale0 = 1.0F - t ;
	scale1 = t ;

	/* 係数を使って計算 */
	to1[0] = scale0 * from->vx + scale1 * to1[0] ;
	to1[1] = scale0 * from->vy + scale1 * to1[1] ;
	to1[2] = scale0 * from->vz + scale1 * to1[2] ;
	to1[3] = scale0 * from->vw + scale1 * to1[3] ;
	res->vx = to1[0] ;
	res->vy = to1[1] ;
	res->vz = to1[2] ;
	res->vw = to1[3] ;
}

static FVECTOR	*HLD_ActMotion( hld, abs )
HOLD_ENE	*hld ;
FVECTOR	*abs ;
{
	FVECTOR	*old_abs ;
	int	i ;

	if ( hld->interp_count <= 0 ) return abs ;

	/* 補間処理 */
	old_abs = &hld->abs_rots[0] ;
	for ( i = MODEL_OBJS ; i > 0 ; i-- ){
		HLD_QuatSlerp( old_abs, old_abs, abs, hld->interp_time );
		abs++ ;
		old_abs++ ;
	}

	if ( hld->interp_count ){
//printf("hld->interp_count [%d] \n",hld->interp_count);
		if ( hld->interp_inc == 0 ) hld->interp_inc = 1 ;
		hld->interp_time += (float)TIME_BASE / (float)hld->interp_inc ;
		if ( ( hld->interp_count -= TIME_BASE ) <= 0 ){
			hld->interp_count = 0 ;
		}
	}

	return &hld->abs_rots[0] ;
}

static FVECTOR	*HLD_AdjustMotion( hld, abs )
HOLD_ENE	*hld ;
FVECTOR	*abs ;
{
	FVECTOR	quat ;
	SVECTOR rot ;
   FVECTOR Adj_Rots[ MODEL_OBJS ] ;

	if ( (hld->adj_y == 0) && (hld->adj_turn_y == 0) 
			&& (hld->adj_x == 0) && (hld->adj_turn_x == 0) ) {
		return abs ;
	}

	if ( hld->adj_turn_y != hld->adj_y ) {
		hld->adj_y = GV_NearExp8P ( hld->adj_y, hld->adj_turn_y ) ;
	} 
	if ( hld->adj_turn_x != hld->adj_x ) {
		hld->adj_x = GV_NearExp8P ( hld->adj_x, hld->adj_turn_x ) ;
	}
	/* アジャスト処理 */
	KR_MemCopy( &Adj_Rots[0], &hld->abs_rots[0], sizeof(FVECTOR), MODEL_OBJS ) ;

	rot.vx = hld->adj_x ;
	rot.vy = hld->adj_y ;
	rot.vz = 0 ;

	GM_RotToQuat( &rot, &quat ) ;
	MT_QuatMul( &Adj_Rots[HUMAN21_ATAMA], &quat, &hld->abs_rots[ HUMAN21_ATAMA ] );

	return &Adj_Rots[0] ;
}

static void HLD_ActMotion2( MOTION_CONTROL *m_ctrl, DG_OBJS *objs, FVECTOR *rots )
{
	ScrPadWork	*scrpad  = (ScrPadWork*)SCRPAD_ADDR ;

	if ( objs->root != NULL )	scrpad->root_mat = *( objs->root );
	else						scrpad->root_mat = objs->world ;

	MT_StartMemToSpr( (u_long128 *)scrpad->joints, (u_long128 *)rots, m_ctrl->n_joints );
	MT_WaitMemToSpr();

	{/* オブジェクトにマトリクスを設定する */
		FVECTOR			*joints = scrpad->joints ;
		FMATRIX			*mats = scrpad->mats ;
		DG_DEF	*def ;
		DG_MDL	*mdl ;
		DG_OBJ	*obj = objs->objs ;
		int		i ;

		def = objs->def ;
		scrpad->vec.vw = 1.0F ;
		{/* モデル情報から親子関係を取得して求める */
			for ( i = def->n_models ; i > 0 ; i-- ){	/* 拡張モデルは無視する */
				FMATRIX	*parent ;
				mdl = obj->model ;
				MT_QuatToMat( mats, joints );
				_sceVu0MulMatrix( mats, &scrpad->root_mat, mats ) ;
				scrpad->vec.vx = mdl->tx ;
				scrpad->vec.vy = mdl->ty ;
				scrpad->vec.vz = mdl->tz ;
				parent = &scrpad->mats[ mdl->parent ] ;
				_sceVu0ApplyMatrix( &mats->m[3][0], parent, &scrpad->vec );
				obj->world = *mats ;
				obj++ ;
				mats++ ;
				joints++ ;
			}
		}
	}

}


static	void	MakeMotion( work, hld )
Work		*work ;
HOLD_ENE	*hld ;
{
	FVECTOR	*abs ;
	abs = GetAbsRot( hld ) ;

	abs = HLD_ActMotion( hld, abs ) ;
	abs = HLD_AdjustMotion( hld, abs ) ;
	HLD_ActMotion2( work->body_mtbuff[hld->mot_num].m_ctrl, hld->body[hld->c_obj].objs, abs ) ;

	hld->m_time ++ ;
}

static	void	NoiseCheck( work )
Work		*work ;
{
	HOLD_ENE	*hld ;
	FVECTOR		pos, vec ;
	int	i, dis, min, min_ene, dir ;

	if ( !GM_NoisePower) return ;

printf("holdene:Noise!!! [%d]\n",GM_NoisePower);

	hld = &work->holdene[0] ;
	min = 100000000 ;
	min_ene = -1 ;

	switch( GM_NoisePower ) {
		case NOISE_L :
			SET_FLAG( GM_GameStatus, GM_STATUS_DETECT ) ;
			for( i=0; i<work->ene_num; i++ ) {
				hld->status2 |= HLD_STATUS2_DETECT ;	/* 発見 */
				hld++ ;
			}
			break ;
		case NOISE_M :
			SET_FLAG( GM_GameStatus, GM_STATUS_DETECT ) ;
			for( i=0; i<work->ene_num; i++ ) {
				hld->status2 |= HLD_STATUS2_DETECT ;	/* 発見 */
				hld++ ;
			}
			break ;
		case NOISE_SS : /*M9*/
printf("M9 noise\n");
			break ;
		case NOISE_HOLD :
			if ( GM_PlayerPosition.vy >= HOLD_FLOOR_LEVEL ) {
				for( i=0; i<work->ene_num; i++ ) {
					if ( !(hld->status & 
						(HLD_STATUS_EAR_CLOSE|HLD_STATUS_CAMERAMN|HLD_STATUS_SCOT) ) ) {

						_sceVu0SubVector( &vec, &GM_PlayerPosition, &hld->mov ) ;
						dis = _FVecLen3( &vec ) ;
						if ( dis <= ENE_HOLD_DIS ) {
							if ( GM_PlayerStatus & PLAYER_HOLD ) {
								dir = _FVecDir2( &vec ) ;
								dir = GV_DiffDirAbs( dir, GM_PlayerControl->rot.vy ) ;
								if ( dir >= 1536 ) {
									if ( dis < min ) {
										min = dis ;
										min_ene = i ;
									}
								}
							}
						}
					}
					hld++ ;
				}
	printf(" noise hold up hold[%d] dis[%d]\n",min_ene,min);
				if ( min_ene >= 0 && min < NOISE_DIS ) {
					hld = &work->holdene[min_ene] ;
					hld->status2 |= HLD_STATUS2_HOLD_UP ;	/* ホールドアップ */
				}
			}
			break ;
		case NOISE_S :
			if ( GM_PlayerPosition.vy >= HOLD_FLOOR_LEVEL ) {
				for( i=0; i<work->ene_num; i++ ) {
					if ( !(hld->status & 
						(HLD_STATUS_EAR_CLOSE|HLD_STATUS_CAMERAMN|HLD_STATUS_SCOT) ) ) {
						KR_FMatToFvec( &BODYWORLD( &hld->body[hld->c_obj], HUMAN21_KOSHI), &pos ) ;
						if ( DG_FABS(pos.vy - GM_NoisePosition.vy) < HLD_NOISE_HEGHT ) {
							dis = _FVecTrgDis( &pos, &GM_NoisePosition ) ;
							if ( dis < min ) {
								min = dis ;
								min_ene = i ;
							}
						}
					}
					hld++ ;
				}
	printf(" noise S hold[%d] dis[%d]\n",min_ene,min);
				if ( min_ene >= 0 && min < NOISE_DIS ) {
					hld = &work->holdene[min_ene] ;
					hld->status2 |= HLD_STATUS2_NOISE_S ;	/* 物音Ｓ聞こえた */
				}
			}
			break ;
	}
}

typedef	struct	{
	FVECTOR	player_pos ;
	FVECTOR	vec ;
	float	dx, dz, f ;
	int		r ;
	float	xx, zz ;
	int		num ;
	float	eye_sight ;
	int		eye_range ;
} ScrPadEyeinfo ;

static	void EyeInfoCheck( work )
Work		*work ;
{
	ScrPadEyeinfo	*scrpad  = (ScrPadEyeinfo *)SCRPAD_ADDR ;
	HOLD_ENE	*hld ;
	float		dy ;

	hld = &work->holdene[0] ;
//return ;
	if ( WORKP->mode == MODE_DEMO ) return ;

	KR_FMatToFvec( &BODYWORLD( GM_PlayerBody, HUMAN21_KOSHI ), &work->touch_pos[0] ) ;
	KR_FMatToFvec( &BODYWORLD( GM_PlayerBody, HUMAN21_HIDARI_ASHI2 ), &work->touch_pos[1] ) ;
	KR_FMatToFvec( &BODYWORLD( GM_PlayerBody, HUMAN21_ATAMA ), &work->touch_pos[2] ) ;

	scrpad->player_pos = GM_PlayerPosition ;
	scrpad->num = work->ene_num ;
	scrpad->eye_sight = ( GM_PlayerStatus & PLAYER_DARK_AREA )? 
					(float) work->eye_sight/4:(float) work->eye_sight ;
	scrpad->eye_range = work->eye_range ;

	while( scrpad->num-- > 0  ) {
		scrpad->dx = scrpad->player_pos.vx - hld->mov.vx ;
		if ( DG_FABS(scrpad->dx) > scrpad->eye_sight ) goto end_chk ;
		scrpad->dz = scrpad->player_pos.vz - hld->mov.vz ;
		if ( DG_FABS(scrpad->dz) > scrpad->eye_sight ) goto end_chk ;

		if ( hld->status & HLD_STATUS_ACTIVE )  goto end_chk ;


		scrpad->f = atan2f( scrpad->dx, scrpad->dz ) ;
		scrpad->r = (int) (2048.0f * scrpad->f / ( float )M_PI) ;
		hld->p_dir = scrpad->r & 4095 ;


		if (hld->status&(HLD_STATUS_EYE_CLOSE|HLD_STATUS_CAMERAMN|HLD_STATUS_SCOT) ) goto end_chk ;

		if ( GM_PlayerStatus & (PLAYER_INTRUDE) ) goto end_chk ;

		if ( GM_PlayerStatus & (PLAYER_INTRUDE) ) goto end_chk ;

		if ( GM_PlayerStatus & (PLAYER_STEALTH) ) goto end_chk ;

//日本版から
		if ( GM_PlayerPosition.vy < HOLD_FLOOR_LEVEL ) goto end_chk ;

		/* 視野判定 */
#if 1
//printf("[%d] kore1 face[%d] pl[%d]\n",hld->id, hld->face_dir, hld->p_dir);
//		hld->face_dir = hld->rot.vy + hld->adj_y ;
		if ( _DiffDirAbs( hld->face_dir, hld->p_dir ) > scrpad->eye_range ) goto end_chk ;

//printf("[%d] kore2\n",hld->id);
		/* 高さは簡易計算 */
		dy = GM_PlayerFindPos.vy - hld->mov.vy ;
		if ( dy > 2000.0 || dy < -2000.0 ) goto end_chk ;
#else
		if ( _DiffDirAbs( hld->rot.vy, hld->p_dir ) > scrpad->eye_range ) goto end_chk ;
#endif

		scrpad->xx = scrpad->dx * scrpad->dx ;
		scrpad->zz = scrpad->dz * scrpad->dz ;
		hld->p_dis = (int)bp_sqrtf( scrpad->xx + scrpad->zz ) ;  //BP_MATH - emulate PS2 sqrtf

//printf("[%d] kore3\n",hld->id);
		if ( ENE_EyeOnlineCheck( work->hzx_id, &hld->mov, &GM_PlayerPosition ) ) goto end_chk ;

//printf("[%d] kore4\n",hld->id);
		hld->status2 |= HLD_STATUS2_INDISTINCT ;	/* プレイヤー見えた */

end_chk :
		hld++ ;
	}
}

#define HOLD_COS_10	(0.9848077530122f)
#define HOLD_COS_15	(0.9659258262891f)
#define HOLD_COS_20	(0.9396926207859f)
#define HOLD_COS_25	(0.9063077870366f)
#define HOLD_COS_30	(0.8660254037844f)
static	void	OtherCheck( work )
Work		*work ;
{
	HOLD_ENE	*hld ;
	int	i, detect_num ;

	if ( work->event_ene >= 0 ) {
		work->holdene[ work->event_ene ].status2 |= work->event_id ;
	}

#ifdef PROJECTOR_SHADOW
	detect_num = 0 ;
	if ( GM_N_PlayerShadowPos > 0 ) {
		int dis1, dis2 ;
		dis1 = _FVecTrgDis( &GM_PlayerFindPos, &work->projector1pos ) ;
		dis2 = _FVecTrgDis( &GM_PlayerFindPos, &work->projector2pos ) ;

		detect_num = ( dis1 < dis2 ) ? 1 : 2 ;
	}
	
	if ( !detect_num ) {
		if ( !(GM_PlayerStatus & (PLAYER_SQUAT|PLAYER_GROUND|PLAYER_CB_BOX)) ) {
			int dis1, dis2 ;
			float inp_th ;

			if ( work->projector1flag & FLAG_EYECHECK ) {
				dis1 = _FVecTrgDis( &GM_PlayerFindPos, &work->projector1pos ) ;
				if ( dis1 < 3000 ) {
//					extern void *NewLineView( FVECTOR *, int, u_char, u_char, u_char ) ;
					static FVECTOR Shift = { 0.0f, 0.0f, 4000.0f } ;
					FVECTOR	lin[2] ;
					float inp ;

					lin[0] = work->projector1pos ;
					DG_SetPos2( &work->projector1pos, &work->projector1rot ) ;
					DG_PutVector( &Shift, &lin[1], 1 ) ;

					lin[0].vy = GM_PlayerFindPos.vy ;
					lin[1].vy = GM_PlayerFindPos.vy ;
//					NewLineView(  &lin[0],1,0,0,255) ;
					inp = KR_InnerProduct( &lin[0], &lin[1], &GM_PlayerFindPos ) ;
					inp_th = ( dis1 < 1000 ) ? HOLD_COS_20 : HOLD_COS_15 ;
					if ( inp_th < inp ) {
//						lin[1] = GM_PlayerFindPos ;
//						NewLineView(  &lin[0],1,255,0,0) ;
						detect_num = 1 ;
					}
				}
			}
			if ( work->projector2flag & FLAG_EYECHECK ) {
				dis2 = _FVecTrgDis( &GM_PlayerFindPos, &work->projector2pos ) ;
				if ( dis2 < 3000 ) {
//					extern void *NewLineView( FVECTOR *, int, u_char, u_char, u_char ) ;
					static FVECTOR Shift = { 0.0f, 0.0f, 4000.0f } ;
					FVECTOR	lin[2] ;
					float inp ;

					lin[0] = work->projector2pos ;
					DG_SetPos2( &work->projector2pos, &work->projector2rot ) ;
					DG_PutVector( &Shift, &lin[1], 1 ) ;

					lin[0].vy = GM_PlayerFindPos.vy ;
					lin[1].vy = GM_PlayerFindPos.vy ;
//					NewLineView(  &lin[0],1,0,0,255) ;
					inp = KR_InnerProduct( &lin[0], &lin[1], &GM_PlayerFindPos ) ;

					inp_th = ( dis2 < 1000 ) ? HOLD_COS_25 : HOLD_COS_15 ;
					if ( inp_th < inp ) {
//						lin[1] = GM_PlayerFindPos ;
//						NewLineView(  &lin[0],1,0,255,0) ;
						detect_num = 2 ;
					}
				}
			}
		}
	}
	if ( detect_num ) {
		hld = &work->holdene[0] ;
		for( i=0; i<work->ene_num; i++ ) {
			hld->status2 |= HLD_STATUS2_PROJECTOR ;	/* 発見 */
			hld++ ;
		}
		if ( detect_num == 1 ) {
			if ( work->proc_detect[ PROC_PROJE1 ] != 0 ) {
				ENE_ExecProc( work->proc_detect[ PROC_PROJE1 ], NULL ) ;
				work->proc_detect[ PROC_PROJE1 ] = 0 ;
			}
		} else {
			if ( work->proc_detect[ PROC_PROJE2 ] != 0 ) {
				ENE_ExecProc( work->proc_detect[ PROC_PROJE2 ], NULL ) ;
				work->proc_detect[ PROC_PROJE2 ] = 0 ;
			}
		}
	}
#else
	if ( work->projector1flag & FLAG_EYE_IN ) {
		hld = &work->holdene[0] ;
		for( i=0; i<work->ene_num; i++ ) {
			hld->status2 |= HLD_STATUS2_PROJECTOR ;	/* 発見 */
			hld++ ;
		}
		if ( work->proc_detect[ PROC_PROJE1 ] != 0 ) {
			ENE_ExecProc( work->proc_detect[ PROC_PROJE1 ], NULL ) ;
			work->proc_detect[ PROC_PROJE1 ] = 0 ;
		}
	}
	if ( work->projector2flag & FLAG_EYE_IN ) {
		hld = &work->holdene[0] ;
		for( i=0; i<work->ene_num; i++ ) {
			hld->status2 |= HLD_STATUS2_PROJECTOR ;	/* 発見 */
			hld++ ;
		}
		if ( work->proc_detect[ PROC_PROJE2 ] != 0 ) {
			ENE_ExecProc( work->proc_detect[ PROC_PROJE2 ], NULL ) ;
			work->proc_detect[ PROC_PROJE2 ] = 0 ;
		}
	}
#endif
	if ( work->gameflag & HLD_STATUS_CAMERA1_DETECT ) {
		if ( work->proc_detect[ PROC_CAMERA1 ] != 0 ) {
			ENE_ExecProc( work->proc_detect[ PROC_CAMERA1 ], NULL ) ;
			work->proc_detect[ PROC_CAMERA1 ] = 0 ;
		}
	}
	if ( work->gameflag & HLD_STATUS_CAMERA2_DETECT ) {
		if ( work->proc_detect[ PROC_CAMERA2 ] != 0 ) {
			ENE_ExecProc( work->proc_detect[ PROC_CAMERA2 ], NULL ) ;
			work->proc_detect[ PROC_CAMERA2 ] = 0 ;
		}
	}


	if ( GM_GameStatus & GM_STATUS_DETECT ) {
		if ( work->proc_detect[ PROC_DETECT ] != 0 ) {
printf("hold detecttttttttttttttttttttttttttttttttt\n");
			ENE_ExecProc( work->proc_detect[ PROC_DETECT ], NULL ) ;
			work->proc_detect[ PROC_DETECT ] = 0 ;
		}
		work->detect_delay ++ ;
	}

}

static	void	MoveTarget( HOLD_ENE *hld )
{
	HOLDBODYPARAM	*bodyp ;
	OBJECT			*body ;

	bodyp = &hld->bodyparam ;
	body = &(hld->body[hld->c_obj]) ;

	/* ターゲットのマップはカレントマップがセットされる */
//    GM_CurrentMap = WORKP->hzx_id ;
    GM_CurrentMap = hld->control.hzx_id ;

    GM_MoveTarget( &( bodyp->deftrg ), &( hld->mov ) ) ;
	/* Level 1 */
    GM_MoveTarget2( &( bodyp->def_child1[ PTARGET_HEAD ] ), &(BODYWORLD( body, HUMAN21_ATAMA )) ) ;
	GM_MoveTarget2( &( bodyp->def_child1[ PTARGET_HART ] ), &(BODYWORLD( body, HUMAN21_MUNE )) ) ;
	GM_MoveTarget2( &( bodyp->def_child1[ PTARGET_GOLD ] ), &(BODYWORLD( body, HUMAN21_KOSHI )) ) ;

	/* Level 2 */
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_ARMR1 ] ), &(BODYWORLD( body, HUMAN21_MIGI_UDE1 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_ARMR2 ] ), &(BODYWORLD( body, HUMAN21_MIGI_UDE2 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_ARML1 ] ), &(BODYWORLD( body, HUMAN21_HIDARI_UDE1 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_ARML2 ] ), &(BODYWORLD( body, HUMAN21_HIDARI_UDE2 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_LEGR1 ] ), &(BODYWORLD( body, HUMAN21_MIGI_ASHI1 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_LEGR2 ] ), &(BODYWORLD( body, HUMAN21_MIGI_ASHI2 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_LEGL1 ] ), &(BODYWORLD( body, HUMAN21_HIDARI_ASHI1 )) ) ;
    GM_MoveTarget2( &( bodyp->def_child2[ PTARGET_LEGL2 ] ), &(BODYWORLD( body, HUMAN21_HIDARI_ASHI2 )) ) ;
	/* Level 3 */
    GM_MoveTarget2( &( bodyp->def_child3[ PTARGET_BODY ] ), &(BODYWORLD( body, HUMAN21_MUNE )) ) ;
}

static	void	StatusManage( HOLD_ENE *hld )
{
	if ( hld->status2 & HLD_STATUS2_TARGET_SKIP ) {
		SET_FLAG( hld->bodyparam.deftrg.class, TARGET_SKIP ) ;
	} else {
		UNSET_FLAG( hld->bodyparam.deftrg.class, TARGET_SKIP ) ;
	}

	if ( WORKP->detect_delay > HLD_DETECT_DELAY_TIME ) {
		SET_FLAG( hld->status, HLD_STATUS_DETECT_DELAY ) ;
	}

	/* アクティブな兵はこれ以降のチェックはしない */
	if ( hld->status & HLD_STATUS_ACTIVE) return ;



	if ( hld->status2 & HLD_STATUS2_EYE_CLOSE ) {
		SET_FLAG( hld->status, HLD_STATUS_EYE_CLOSE ) ;
	} else {
		UNSET_FLAG( hld->status, HLD_STATUS_EYE_CLOSE ) ;
	}
	if ( hld->status2 & HLD_STATUS2_EAR_CLOSE ) {
		SET_FLAG( hld->status, HLD_STATUS_EAR_CLOSE ) ;
	} else {
		UNSET_FLAG( hld->status, HLD_STATUS_EAR_CLOSE ) ;
	}

	if ( hld->status2 & HLD_STATUS2_GUN_LEFT ) {
		if ( hld->cnct.cnct_num != HUMAN21_HIDARI_TE ) {
			CNCT_CnctObjNum( &hld->cnct, HUMAN21_HIDARI_TE ) ;
			CNCT_CnctObjShift( &hld->cnct, &HOLD_WP_Shift[WP_POS_LEFT_HAND] ) ;
			CNCT_CnctObjRot( &hld->cnct, &HOLD_WP_Rot[WP_POS_LEFT_HAND] ) ;
		}
	}
	if ( hld->status2 & HLD_STATUS2_GUN_RIGHT ) {
		if ( hld->cnct.cnct_num != HUMAN21_MIGI_TE ) {
			CNCT_CnctObjNum( &hld->cnct, HUMAN21_MIGI_TE ) ;
			CNCT_CnctObjShift( &hld->cnct, &HOLD_WP_Shift[WP_POS_RIGHT_HAND] ) ;
			CNCT_CnctObjRot( &hld->cnct, &HOLD_WP_Rot[WP_POS_RIGHT_HAND] ) ;
		}
	}
	if ( hld->status2 & HLD_STATUS2_GUN_SHOLDER ) {
		if ( hld->cnct.cnct_num != HUMAN21_MUNE ) {
			CNCT_CnctObjNum( &hld->cnct, HUMAN21_MUNE ) ;
			CNCT_CnctObjShift( &hld->cnct, &HOLD_WP_Shift[WP_POS_SHOLDER] ) ;
			CNCT_CnctObjRot( &hld->cnct, &HOLD_WP_Rot[WP_POS_SHOLDER] ) ;
		}
	}

}

static void UpdateCertainEnemies( Work *work, int start, int end )
{
   int i;

   for ( i = start; i < end; ++i )
   {
      HOLD_ENE *hld = &work->holdene[i];

      if ( !(hld->status & HLD_STATUS_ACTIVE) ) {
         int mas;
         float	height ;
         FMATRIX mat ;
         FVECTOR	step ;

         SetLod( work, hld ) ;
         mas = hld->mot_num ;

         height = (float)*(WORKP->m_height + WORKP->m_ptr[mas]+ hld->m_time%WORKP->m_len[mas] ) ;
         /* 水平移動 */
         DG_SetPos2( &DG_ZeroVector, &hld->rot ) ;
         DG_PutVector( (work->m_step+work->m_ptr[mas]+(hld->m_time%work->m_len[mas])), &step, 1 ) ;
         _sceVu0AddVector( &hld->mov, &hld->mov, &step ) ;
         //printf("[%d] step [%f] [%f] [%f]\n",hld->m_time,step.vx,step.vy,step.vz) ;

         /* 体の向き */
         if ( hld->rot.vy != hld->turn_y ) {
            hld->rot.vy = GV_NearExp8P ( hld->rot.vy, hld->turn_y ) ;
         }

         /* 顔の向き */
         if ( hld->mot_num == USHOLD_look_r || hld->mot_num == USHOLD_look_r_idle ) {
            hld->face_dir = GV_NearExp8P ( hld->face_dir, hld->rot.vy-1024 ) ;
         } else if ( hld->mot_num == USHOLD_look_l || hld->mot_num == USHOLD_look_l_idle ) {
            hld->face_dir = GV_NearExp8P ( hld->face_dir, hld->rot.vy+1024 ) ;
         } else {
            //				hld->face_dir = hld->rot.vy ;
            hld->face_dir = hld->rot.vy + hld->adj_y ;
         }

         if ( hld->scale != 1.0f ) {
            if ( hld->scale == 0.0f ) {
               hld->scale = 1.0f ;
            }

            height = hld->pos.vy + (height*hld->scale);
            if ( hld->interp_count > 0 ) {
               hld->mov.vy = GV_NearExp8F( hld->mov.vy, height ) ;
            } else {
               hld->mov.vy = height;
            }

            _RotMatrixZYX( &mat, &hld->rot ) ;
            KR_FvecToMat( &hld->mov, &mat ) ;
            mat.m[3][0] /= hld->scale ;
            mat.m[3][1] /= hld->scale ;
            mat.m[3][2] /= hld->scale ;
            mat.m[3][3] /= hld->scale ;
            DG_SetPos( &mat ) ;
         } else {
            height = hld->pos.vy + height;
            if ( hld->interp_count > 0 ) {
               hld->mov.vy = GV_NearExp8F( hld->mov.vy, height ) ;
            } else {
               hld->mov.vy = height;
            }
            DG_SetPos2( &hld->mov, &hld->rot ) ;
         }

         DG_PutObjs( hld->body[hld->c_obj].objs ) ;
         MakeMotion( work, hld ) ;
#ifdef DEBUG_MODE
         if ( work->mode != MODE_DEMO ) {
            if ( work->com->status & CMST_ENEMY_BODY_LIGHT ) {
               hld->head_num |= (1 << 2) ;		/* 表示 */
               hld->head_num &= ~(3 << 3) ;	/* 番号クリア */
               hld->head_num |= hld->c_obj << 3 ;	/* 番号セット */
            } else {
               hld->head_num &= ~(1 << 2) ;	/* 非表示 */
            }
         }
#endif

         GM_RadarSetFlag( &hld->rctrl, RADAR_VISIBLE ) ;
         if ( hld->status & HLD_STATUS_EYE_CLOSE ) {
            GM_RadarSetSight( &hld->rctrl, hld->face_dir, 
               0, 0.0f, RADAR_COLOR_BLUE );
         } else {
            GM_RadarSetSight( &hld->rctrl, hld->face_dir, 
               //					HLD_EYE_RANGE*2, (float)HLD_EYE_SIGTH, RADAR_COLOR_BLUE );
               HLD_EYE_RANGE*2, (float)HLD_EYE_SIGTH+4000, RADAR_COLOR_BLUE );
         }
         SetControlPos( hld ) ;
      } else {
         GM_RadarResetFlag( &hld->rctrl, RADAR_VISIBLE ) ;
      }

      MoveTarget( hld ) ;
   }
}

struct _SLocalUltWork
{
   Work *mpWork;
   int mStart;
   int mEnd;
};

static void ActUlt( SULTParam const *pParam )
{
   struct _SLocalUltWork const *pLocalWork = (struct _SLocalUltWork const *) pParam;
   UpdateCertainEnemies( pLocalWork->mpWork, pLocalWork->mStart, pLocalWork->mEnd );
}

/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
	HOLD_ENE	*hld ;
   int i ;

	StatusInit( work ) ;

	CheckMessage( work ) ;

	GameScnCheck( work ) ;

	NoiseCheck( work ) ;

	EyeInfoCheck( work ) ;

	OtherCheck( work ) ;

	hld = &work->holdene[0] ;

	for( i=0; i<work->ene_num; i++ ) {
		DamageCheck ( hld ) ;

		Think ( hld ) ;
		Action ( hld ) ;
		
		StatusManage( hld ) ;
#if 0
		/* 30f/S 再生 */
		if ( (i%2) != (GV_Time%2) ) {
			hld++ ;
			continue ;
		}
#endif

		hld++ ;
	}

#if HOLDENE_VITA_UPDATE_OPT==0
   UpdateCertainEnemies( work, 0, work->ene_num );
#else

   for ( i = 0; i < work->ene_num; )
   {
      struct _SLocalUltWork p;
      int next = i + 10;
      if ( next > work->ene_num )
      {
         next = work->ene_num;
      }

      p.mpWork = work;
      p.mStart = i;
      p.mEnd = next;

      BP_Render_PostUltWork( ActUlt, &p, sizeof( p ) );

      i = next;
   }

   BP_Render_HelpDrainUltWork();
#endif
//printf("put model %f %f %f\n",work->pos.vx,work->pos.vy,work->pos.vz ) ;
	work->count++ ;
}

static	void	Die( work )
Work		*work ;
{
	HOLD_ENE	*hld ;
	int i, j ;


	for( i=0; i<MOTION_NUM; i++ ) {
		GM_FreeObject( &work->body_mtbuff[i] );
	}

	GV_DelayedFree( work->m_buff );
	GV_DelayedFree( work->m_step );
	GV_DelayedFree( work->m_height );

	hld = &work->holdene[0] ;
	for( j=0; j<work->ene_num; j++ ) {
#ifdef HOLD_SET_NEEDL
		SearchAndKillAttachment_called(&hld->body[0]) ;
#endif
		if ( hld->mdl_num == MODEL_PANTS ) { 
			extern int PL_DeleteDGCameraCheckChara( OBJECT *object ) ;
			PL_DeleteDGCameraCheckChara( &hld->body[0] ) ;
		}
		for( i=0; i<MAX_LOD; i++ ) {
#if HOLDENE_VITA_RENDER_OPT
         DG_AS_FreePrecomputedChainBuffersObjs( hld->body[i].objs );
#endif
			if ( hld->mdl_num < MODEL_NUM ) {
				FreeObject_USSoldier( &hld->body[i], &hld->object_chg[i] ) ;
			} else {
			    GM_FreeObject( &hld->body[i] ) ;
			}
		}
		if ( hld->items & ITEM_WEAPON ) {
#if HOLDENE_VITA_RENDER_OPT
         DG_AS_FreePrecomputedChainBuffersObjs( hld->cnct.body.objs );
#endif
         GM_FreeObject( &hld->cnct.body ) ;
		}
		if ( hld->items & ITEM_ATAMA ) {
#if HOLDENE_VITA_RENDER_OPT
         DG_AS_FreePrecomputedChainBuffersObjs( hld->atama.objs );
#endif
         GM_FreeObject( &hld->atama ) ;
		}
		if ( hld->items & ITEM_GLASS ) {
#if HOLDENE_VITA_RENDER_OPT
         DG_AS_FreePrecomputedChainBuffersObjs( hld->glass.objs );
#endif
         GM_FreeObject( &hld->glass ) ;
		}

		if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
			GM_FreeHomingTrg( &(hld->hom) ) ;
		}

	    GM_FreeRadarControl( &(hld->rctrl) ) ;
		GM_FreeControl( &(hld->control) ) ;
	    GM_FreeTarget( &hld->bodyparam.deftrg ) ;	/* 親だけで良い */

		hld++ ;
	}
}

/*----------------------------------------------------------------*/
static int MotionPreCalloc( Work *work )
{
	int	i, len, sum ;
	OBJECT	*body ;
	FVECTOR	*p, *s ;
	u_short *h ;

	for( i=0; i<MOTION_NUM; i++ ) {
		GM_InitObject( &work->body_mtbuff[i], DEF_NAME, BODY_FLAG );
		GM_ConfigObjectMotion( &work->body_mtbuff[i], 1, MOT_NAME, MT_FLAG_HUMAN2 );
		GM_ConfigObjectStep( &work->body_mtbuff[i], &work->step_buff[i] ) ;
		work->body_mtbuff[i].objs->flag |= DG_FLAG_INVISIBLE ;

		/* sar ファイルを使用しない プリ計算時にカメラ関係の初期化がまだなので落ちちゃう */
		if ( work->body_mtbuff[i].m_ctrl->sar_ctrl != NULL ) {
			GV_Free( work->body_mtbuff[i].m_ctrl->sar_ctrl ) ;
			work->body_mtbuff[i].m_ctrl->sar_ctrl = NULL ;
		}
	}

    if( (work->m_buff = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * 21 * MOTION_BUFF_SIZE ))==NULL) {
		printf(" m_buff NO MEMORY !! Hold Enemy Motion Buffer !!\n");
		return -1 ;
	}
    GV_ZeroMemory(work->m_buff,sizeof(FVECTOR)*21*MOTION_BUFF_SIZE);

    if( (work->m_step = (FVECTOR *)GV_Malloc(sizeof(FVECTOR) * MOTION_BUFF_SIZE))==NULL) {
		printf(" m_step NO MEMORY !! Hold Enemy Motion Buffer !!\n");
		return -1 ;
	}
    GV_ZeroMemory(work->m_step,sizeof(FVECTOR)*MOTION_BUFF_SIZE);

    if( (work->m_height = (u_short *)GV_Malloc(sizeof(u_short) * MOTION_BUFF_SIZE))==NULL) {
		printf(" m_height NO MEMORY !! Hold Enemy Motion Buffer !!\n");
		return -1 ;
	}
    GV_ZeroMemory(work->m_height,sizeof(u_short) * MOTION_BUFF_SIZE );

    MT_SetMotionSeTable( work->body_mtbuff[0].m_ctrl, GM_CurrentMap, 1, 0, 0 ) ;

	sum = 0 ;
	p = work->m_buff ;
	s = work->m_step ;
	h = work->m_height ;
	DG_SetPos( &DG_UnitMatrix ) ;
	for( i=0; i<MOTION_NUM; i++ ) {
		body = &work->body_mtbuff[i] ;
		work->m_ptr[i] = sum ;
printf(" MOTION [%d] [%d] \n",i,work->m_ptr[i]) ;
		GM_ConfigObjectAction( body, 0, i, 0, 0xfffff, 0 );
		len = COUNT_VMODE(body->m_ctrl->mt3_ctrl[0].file_header->motion_length) ;
		work->m_len[i] = len ;
		sum += len ;
		while( len-- > 0 ){
			DG_SetPos( &DG_UnitMatrix ) ;
			GM_ActObject( body ) ;

			KR_MemCopy( p, body->m_ctrl->abs_rots, sizeof(FVECTOR), MODEL_OBJS ) ;
if ( i==24 ) KR_MemCopy( s, &DG_ZeroVector, sizeof(FVECTOR), 1 ) ;
else			KR_MemCopy( s, body->step, sizeof(FVECTOR), 1 ) ;
			*h = body->height ;

if ( i<7){
//printf(" time [%d] step[%f][%f][%f] \n",work->m_len[i]-1-len,body->step->vx,body->step->vy,body->step->vz) ;
//printf(" time [%d] height[%f] \n",work->m_len[i]-1-len,body->height ) ;
}

			p += MODEL_OBJS ;
			s++ ;
			h++ ;
		}
	}
	
	printf(" MOTION DATA [%d] \n",sum) ;
	if( sum >= MOTION_BUFF_SIZE ) {
		printf("holdene: Err Motion Buff Over [%d] / [%d]!!\n",sum, MOTION_BUFF_SIZE );
	}
	return 0 ;
}

#if 1
static void	TargetCallBack( off, def, hld )
TARGET		*off, *def ;
HOLD_ENE	*hld ;
{
    if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			def->power->force = DG_ZeroVector ;
			GM_DamageTarget( off, def ) ;
			hld->bodyparam.off_center = off->center ;
//printf("oya call def->weapon_type[%x] off[%x]\n",def->weapon_type,off->weapon_type);
//printf("Oya [%d]force [%f][%f][%f] \n",GV_Time, def->power->force.vx,def->power->force.vy,def->power->force.vz );
		}
	}
}
//#define _TARGET_VIEW (1)
static int InitTarget( HOLD_ENE *hld )
{
	HOLDBODYPARAM		*bodyp ;
	TARGET			*trg, *child1, *child2, *child3 ;
	POWER_TARGET	*power, *power_child1, *power_child2, *power_child3 ;
    FVECTOR	size, offset ;
    int	i ;

	bodyp = &hld->bodyparam ;
	trg = &bodyp->deftrg ;
	power = &bodyp->power ;

	child1 = bodyp->def_child1 ;
	power_child1 = &bodyp->power_child1[0] ;
	child2 = bodyp->def_child2 ;
	power_child2 = &bodyp->power_child2[0] ;
	child3 = bodyp->def_child3 ;
	power_child3 = &bodyp->power_child3[0] ;

	size = EneBodyTrgSize[ ENE_BODYTRGSIZE_STAND ] ;
    GM_SetTarget( trg, TARGET_TOUCH|TARGET_DEFENSE|TARGET_SEEK, WORKP->hzx_id, ENEMY_SIDE,  &size, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( trg, 0 ) ;
    GM_SetPowerTarget( trg, power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
	GM_SetTargetCallBack( trg, TargetCallBack, hld ) ;
	GM_SetCaptureTarget( trg, &( bodyp->capture ), &hld->control, &hld->body[0] ) ;
    GM_PutTarget( trg ) ;


#ifdef _TARGET_VIEW
NewTargetView( &bodyp->deftrg, 34, 184, 200 ) ;
#endif

	for( i=0; i<PTARGET_LEVEL1_NUM; i++ ) {
		switch ( i ) {
			case PTARGET_HEAD:
				size.vx = size.vz = 96.0F ; size.vy = 130.0F ;
				offset.vx = offset.vz = 0.0F ; offset.vy = 30.0F ;
				break ;
			case PTARGET_HART:
				size.vx = 50.F ; size.vz = 165.0F ; size.vy = 65.0F ;
				offset.vx = 100.0F ; offset.vz = 0.0F ; offset.vy = 100.0F ;
				break ;
			case PTARGET_GOLD:
				size.vx = 100.0F ; size.vz = 150.0F ; size.vy = 65.0F ;
				offset.vx = offset.vz = 0.0F ; offset.vy = -150.0F ;
				break ;
		}
		GM_SetTarget( child1, CHILD_TRG_FLAG, WORKP->hzx_id, ENEMY_SIDE, &size, &offset ) ;
		GM_SetTargetWeaponType( child1, 0 ) ;
		GM_SetPowerTarget( child1, power_child1, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
		GM_SetTargetCallBack( child1, TargetCallBack, hld ) ;

#ifdef _TARGET_VIEW
NewTargetView( child1, 200, 34, 184 ) ;
#endif
		child1++ ;
		power_child1++ ;
	}
	GM_SetTargetParts( trg, bodyp->def_child1, PTARGET_LEVEL1_NUM, 1 ) ;

	for( i=0; i<PTARGET_LEVEL2_NUM; i++ ) {
		switch ( i ) {
			case PTARGET_ARMR1:
			case PTARGET_ARMR2:
			case PTARGET_ARML1:
			case PTARGET_ARML2:
			size.vx = size.vz = 75.0F ; size.vy = 150.0F ;
			offset.vx = offset.vz = 0.0F ; offset.vy = -130.0F ;
			break ;

			case PTARGET_LEGL1:
			case PTARGET_LEGR1:
			size.vx = 85.0F ; size.vz = 120.0F ; size.vy = 200.0F ;
			offset.vx = offset.vz = -10.0F ; offset.vy = -200.0F ;
			break ;

			case PTARGET_LEGL2:
			case PTARGET_LEGR2:
			size.vx = 85.0F ; size.vz = 120.0F ; size.vy = 250.0F ;
			offset.vx = offset.vz = -20.0F ; offset.vy = -250.0F ;
			break ;
		}
		GM_SetTarget( child2, CHILD_TRG_FLAG, WORKP->hzx_id, ENEMY_SIDE, &size, &offset ) ;
		GM_SetTargetWeaponType( child2, 0 ) ;
		GM_SetPowerTarget( child2, power_child2, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
		GM_SetTargetCallBack( child2, TargetCallBack, hld ) ;

#ifdef _TARGET_VIEW
NewTargetView( child2, 34, 184, 200 ) ;
#endif
		child2++ ;
		power_child2++ ;
	}
	GM_SetTargetParts( trg, bodyp->def_child2, PTARGET_LEVEL2_NUM, 2 ) ;

	for( i=0; i<PTARGET_LEVEL3_NUM; i++ ) {
		size.vx = 200.0F ; size.vz = 200.0F ; size.vy = 300.0F ;
		offset.vx = offset.vz = 0.0F ; offset.vy = 0.0F ;
		GM_SetTarget( child3, CHILD_TRG_FLAG, WORKP->hzx_id, ENEMY_SIDE, &size, &offset ) ;
		GM_SetTargetWeaponType( child3, 0 ) ;
		GM_SetPowerTarget( child3, power_child3, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
		GM_SetTargetCallBack( child3, TargetCallBack, hld ) ;

#ifdef _TARGET_VIEW
NewTargetView( child3, 184, 200, 34 ) ;
#endif
		child3++ ;
		power_child3++ ;
	}
	GM_SetTargetParts( trg, bodyp->def_child3, PTARGET_LEVEL3_NUM, 3 ) ;

	return 0 ;
}
#endif

static void InitControl ( HOLD_ENE *hld )
{
	CONTROL	*ctrl ;
	
	ctrl = &hld->control ;

	GM_InitControl( ctrl, hld->name, 0 ) ;
	SetControlPos( hld ) ;
	ctrl->height = 1049.0F ;

	GM_ConfigControlHazard( ctrl, 1200, 450, 500 ) ;
	GM_ConfigControlTrapCheck( ctrl ) ;
	GM_ConfigControlMessageCheck( ctrl ) ;

	GM_ConfigControlAddressCheck( ctrl ) ;
	GM_ConfigControlMapCheck( ctrl ) ;

    GM_ConfigControlHzxHeight( ctrl, 750.0F, ctrl->mov.vy + 100.0F ) ;
	GM_ConfigControlMapID( ctrl ) ;

    ctrl->seg_flag |= HZX_TYPE_ENEMY ;
	ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;
}


static void InitHoming( HOLD_ENE *hld )
{
	OBJECT *body ;
	CONTROL *ctrl;
	HOMING_TRG	*hom ;

	if ( (GM_GameStatus & STATE_VR_ANOTHER) ) {
		body = &hld->body[0] ;
		ctrl = &hld->control ;
		hom = &hld->hom ;

		GM_SetHomingTrg( hom, &(BODYWORLD( body, HUMAN21_MUNE )), 
			body, &(ctrl->hzx_id), ctrl, HOMING_ENEMY ) ;
		GM_SetHomingTrgTarget( hom, &hld->bodyparam.deftrg ) ;
		GM_PutHomingTrg( hom ) ;
	}
}

static void InitDogtag( Work *work )
{
	extern int SIG_CheckDogTagFlag(int num) ;
	extern void *NewCreateDogtag3(OBJECT *, OBJECT *, OBJECT *, int model_name, FVECTOR* shift, int* flag);
	HOLD_ENE	*hld ;
	int			i, hld_name ;

	hld = &work->holdene[0] ;
	for( i=0; i<work->ene_num; i++ ) {
		hld->dogtag_id = -1 ;
		hld->dogtag_item.n_proc = 0 ;
		hld->dogtag_item.c_proc = 0 ;
		hld ++ ;
	}

#if 1
	if ( (GCL_GetOption( 'j' ) != NULL) &&
		 (GM_GameLevel != GM_LEVEL_E_EXTREME) ){
#else
	if ( GCL_GetOption( 'j' ) != NULL ){
#endif
		char *ptr, *country, *ptr2 ;
		int	level, birth ;
			
		level = 0 ;
		switch ( GM_GameLevel ) {
			case GM_LEVEL_VERYEASY :
printf("hold: game level very easy\n");
			break ;
			case GM_LEVEL_EASY :
printf("hold: game level easy\n");
				level = 1 ;
			break ;
			case GM_LEVEL_NORMAL :
printf("hold: game level normal\n");
				level = 2 ;
			break ;
			case GM_LEVEL_HARD :
printf("hold: game level gard\n");
				level = 3 ;
			break ;
			case GM_LEVEL_EXTREME :
printf("enemy: game level extreme\n");
				level = 4 ;
			break ;
		}

		while ( GCL_NextStr() != NULL ) {
			/* ドッグタグを持つ兵 */
			hld_name = GCL_GetNextInt();
			hld = &work->holdene[NameToID( work, hld_name )] ;
			/* プロック */
			hld->dogtag_item.proc[ 0 ] = GCL_GetNextInt();
			/* リソース先頭 */
			ptr = ptr2 = NULL ;
			for ( i=0; i<5; i++ ) {
				ptr2 = GCL_GetNextString();
				if ( i == level ) ptr = ptr2;
			}
			ptr2 = GCL_NextStr() ;

			GCL_SetArgTop( ptr );
			/* ドッグタグ情報 */
			hld->dogtag_id = GCL_GetNextInt();			/*ＩＤ*/
			ASSERT( hld->dogtag_id >= 0 ) ;
#ifdef DOGTAG_DOUBLE
			if ( GM_Configuration2 & GM_CONFIG_DOGTAGS_2002 ) {
				for ( i=0; i<DOGTAG_RES_NUM; i++ ) {
					GCL_GetNextInt();
				}
			}
#endif
			hld->dogtag_item.str = GCL_GetNextString();	/* 名前 */
			country = GCL_GetNextString();				/* 国 */
			birth = GCL_GetNextInt();					/* 誕生日 */

			hld->dogtag_item.probability[ 0 ] = 100 ;
			hld->dogtag_item.n_proc = 1 ;
			hld->dogtag_item.c_proc = 0 ;
			if ( SIG_CheckDogTagFlag( hld->dogtag_id ) ) {
				hld->dogtag_item.c_proc = 1 ;	/* 取得済み */
			} else {
				GV_SetActorChild( work, 
					NewCreateDogtag3( &hld->body[0], &hld->body[1], &hld->body[2],
						E_DOGTAG_NAME, &DG_ZeroVector, &hld->dogtag_sw) ) ;
			}
			{	/* 名前出力 */
				extern void *NewNamePrint4Lod( OBJECT* body, OBJECT* bodyLod1, OBJECT* bodyLod2,
					char *str, int tagID, int nBCD ) ;
				NewNamePrint4Lod( &hld->body[0], &hld->body[1], &hld->body[2],
						 hld->dogtag_item.str, hld->dogtag_id, birth) ;
			}
			printf("dogtagname[%s] country[%s]\n",hld->dogtag_item.str, country);

			GCL_SetArgTop( ptr2 );
		}
	}
}

#if HOLDENE_VITA_RENDER_OPT
static struct _SBP_OBJ_Render *buffer_objs_for_opt_render( struct _SBP_OBJ_Render *pRender, DG_OBJS *objs )
{
#if 1
   objs->flag |= DG_FLAG_AS_NOBOUNDCHECK;
   return DG_AS_PrecomputeChainBuffersObjs( objs, pRender );
#else
   return NULL;
#endif
}
#endif

static	int	GetResources( work, pname, where )
Work	*work ;
int		pname ;
int		where ;
{
	HOLD_ENE	*hld ;
	char		*opt ;
	int			i, j, num, scale, name, height, watch ;

#if HOLDENE_VITA_RENDER_OPT
   struct _SBP_OBJ_Render *renderListHeadsLOD[MAX_LOD] = { NULL };
   struct _SBP_OBJ_Render *renderListWeapons = NULL;
   struct _SBP_OBJ_Render *renderListAtama = NULL;
   struct _SBP_OBJ_Render *renderListGlass = NULL;
#endif

	work->active_num = 0 ;

	/* モード */
	work->mode = GCL_GetOptionValue( 'm', 0 ) ;

	/* コマンダー処理 */
	if ( work->mode != MODE_DEMO ) {
		work->com = COM_GetCommander( ) ;
		if ( COM_SetGroup( &work->com->enemys, &work->group ) < 0 ) return (-1) ;
		COM_SetUnit( &work->group, &work->unit ) ;
	}

	hld = &work->holdene[0] ;


#ifdef HZX_DEBUG_MODE
	/* 複数のマップがセットされていないかチェック */
	HZX_GetGroupNo( where ) ;
#endif

	GM_CurrentMap = where ;	
	work->hzx_id = where ;
	work->name = pname ;
	work->gameflag = 0 ;
	WORKP = work ;

	work->ene_num = 0 ;
	work->eye_sight = HLD_EYE_SIGTH ;
	work->eye_range = HLD_EYE_RANGE ;
	/* シナリオ読み込み */
	if ( ( opt = GCL_GetOption( 'a' ) ) != NULL ){
		while ( GCL_NextStr() != NULL ){
			hld->id = work->ene_num++ ;				/* ID */

			hld->name = GCL_GetNextInt( ) ;			/* 名前 */
			ENE_GCL_GetFV( opt, &hld->pos ) ;		/* 場所 */
			GCL_GetNextSV( (short *)&hld->rot ) ;	/* 方向 */
     		hld->mdl_num = GCL_GetNextInt( )%(MODEL_NUM+4) ;		/* モデル番号 */
			scale = GCL_GetNextInt( ) ;				/* スケール値 */
#ifdef NO_SCALE
			hld->scale = 1.0F ;
#else
			hld->scale = (float)scale/100.0F ;
#endif
			hld->mot_type = GCL_GetNextInt( )%MASTER_NUM ;		/* モーションタイプ */
			hld->items = GCL_GetNextInt( ) ;		/* 帽子(1)、眼鏡(2)、ヘルメット(4) */

//#ifdef DEB_PRINT
#if 0
printf("id[%d]:",hld->id ) ;
printf("p[%.1f][%.1f] ",hld->pos.vx,hld->pos.vz ) ;
printf("dir[%d]",hld->rot.vy ) ;
printf("mdl[%d]",hld->mdl_num ) ;
printf("scl[%d] ",scale ) ;
printf("mot type[%d]\n",hld->mot_type ) ;
#endif

			hld ++ ;
		}
		printf(" hold enemy num [%d] \n", work->ene_num) ;
	} else return -1 ;

	if ( ( opt = GCL_GetOption( 't' ) ) != NULL ){
		num = ENE_GclGetInt( opt, &work->lod_th[0] ) ;
		if ( num != MAX_LOD-1 ) {
			printf(" Err LOD Threshold nedd %d!! \n",MAX_LOD-1 ) ;
			return -1 ;
		}
	}
	if ( ( opt = GCL_GetOption( 's' ) ) != NULL ){
		ENE_GCL_GetFV( opt, &work->screen_pos ) ;		/* 場所 */
		work->screen_pos2 = DG_ZeroVector ;
		ENE_GCL_GetFV( opt, &work->screen_pos2 ) ;		/* 場所 */
printf("screen[%f][%f][%f]\n",work->screen_pos.vx,work->screen_pos.vy,work->screen_pos.vz ) ;
printf("screen2[%f][%f][%f]\n",work->screen_pos2.vx,work->screen_pos2.vy,work->screen_pos2.vz ) ;
		hld = &work->holdene[0] ;
		for( j=0; j<work->ene_num; j++ ) {
if ( work->mode == MODE_JIK_PARA ) {
	FVECTOR ppp ;
	
	ppp.vx = 0.0f ;
	ppp.vy = hld->pos.vy ;
	ppp.vz = -10500.0f ;
			hld->rot.vy = _FVecTrgDir2( &ppp, &work->screen_pos ) ;
} else {
			hld->rot.vy = _FVecTrgDir2( &hld->pos, &work->screen_pos ) ;
}
			hld->screen1_dir = hld->turn_y = hld->rot.vy ;
			hld->aim_dir = hld->screen1_dir ;

if ( work->mode == MODE_JIK_PARA ) {
	FVECTOR ppp ;
	
	ppp.vx = 0.0f ;
	ppp.vy = hld->pos.vy ;
	ppp.vz = -10500.0f ;
			hld->screen2_dir = _FVecTrgDir2( &ppp, &work->screen_pos2 ) ;
} else {
			hld->screen2_dir = _FVecTrgDir2( &hld->pos, &work->screen_pos2 ) ;
}

			hld ++ ;
		}
	}

	/* 終了プロック */
	for( i=0; i<MAX_PROC; i++ ) {
		work->proc_detect[i] = 0 ;
	}
#ifndef NO_PROC_CALL
	if ( ( opt = GCL_GetOption( 'e' ) ) != NULL ){
		for( i=0; i<MAX_PROC; i++ ) {
			if( GCL_NextStr() != NULL ) {
				work->proc_detect[i] = GCL_GetNextInt( ) ;
			} else {
				break ;
			}
		}
	}
#endif

	/* プロジェクター */
	work->projector1flag = 0 ;
	work->projector2flag = 0 ;
	work->projector1pos = DG_ZeroVector ;
	work->projector2pos = DG_ZeroVector ;
	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
		extern void	*NewEye( FVECTOR *,SVECTOR *,int , int , OBJECT *, int , int * ) ;
		FVECTOR	pos ;
		SVECTOR	rot ;
		int		range ;

		ENE_GCL_GetFV( opt, &pos ) ;		/* 場所 */
		GCL_GetNextSV( (short *)&rot ) ;	/* 方向 */
		range = GCL_GetNextInt( ) ;			/* 視野 */

		work->projector1flag = FLAG_EYECHECK ;

#ifdef PROJECTOR_SHADOW
		work->projector1pos = pos ;
		work->projector1rot = rot ;
#else
		if ( WORKP->mode != MODE_DEMO ) {
			NewEye( &pos, &rot, 3000, range, GM_PlayerBody, 0x14c8, &work->projector1flag ) ;
		}
#endif
		if( GCL_NextStr() != NULL ) {
			ENE_GCL_GetFV( opt, &pos ) ;		/* 場所 */
			GCL_GetNextSV( (short *)&rot ) ;	/* 方向 */
			range = GCL_GetNextInt( ) ;			/* 視野 */
			work->projector2flag = FLAG_EYECHECK ;
#ifdef PROJECTOR_SHADOW
			work->projector2pos = pos ;
			work->projector2rot = rot ;
#else
			if ( WORKP->mode != MODE_DEMO ) {
				NewEye( &pos, &rot, 3000, 128, GM_PlayerBody, 0x14c8, &work->projector2flag ) ;
			}
#endif
		}
	}

if(0){
	extern void	*NewEye( FVECTOR *,SVECTOR *,int , int , OBJECT *, int , int * ) ;
	FVECTOR	pos ;
	SVECTOR	rot ;

	pos.vx = 0 ;
	pos.vy = -15750 ;
	pos.vz = -1500 ;
	rot.vx = -200 ;
	rot.vy = 1890 ;
	rot.vz = 0 ;

	work->projector1flag = FLAG_EYECHECK ;
	NewEye( &pos, &rot, 3000, 128, GM_PlayerBody, 0x14c8, &work->projector1flag ) ;
}


	/* 最初の見るべき場所 */
	watch = GCL_GetOptionValue( 'w', 0 ) ;

	/* モーションバッファ初期化 */
	if ( MotionPreCalloc( work ) < 0 ) return -1;

	InitDogtag( work ) ;

	/* 初期化 */
	hld = &work->holdene[0] ;
	for( j=0; j<work->ene_num; j++ ) {
		for( i=0; i<MAX_LOD; i++ ) {
			if ( hld->mdl_num < MODEL_NUM ) {
				InitObject_USSoldier(&hld->body[i], BODY_FLAG, &hld->object_chg[i], hld->mdl_num+(i*MODEL_NUM) ) ;
//				InitObject_USSoldier(&hld->body[i], BODY_FLAG, &hld->object_chg[i], hld->mdl_num+(0*MODEL_NUM) ) ;
			} else if ( hld->mdl_num == MODEL_SCOT ) {
				GM_InitObject( &hld->body[i], GV_StrCode("sco_def_light"), BODY_FLAG ) ;
				GM_ConfigObjectEvm( &hld->body[i], GV_StrCode("sco_def"), DG_EVMOBJ_IRREACTION ) ;
			} else if ( (hld->mdl_num == MODEL_PANTS) || (hld->mdl_num == MODEL_PANTS_NOPROC) ) {
				GM_InitObject( &hld->body[i], GV_StrCode("us_def_pants"), BODY_FLAG ) ;
			} else {
				if ( i==0) GM_InitObject( &hld->body[i], GV_StrCode("us_def_5_s"), BODY_FLAG ) ;
				if ( i==1) GM_InitObject( &hld->body[i], GV_StrCode("us_mid_5_s"), BODY_FLAG ) ;
				if ( i==2) GM_InitObject( &hld->body[i], GV_StrCode("us_low_5_s"), BODY_FLAG ) ;
			}

			GM_GroupObjs( hld->body[i].objs, GM_CurrentMap ) ;
			GM_ConfigObjectLight( &hld->body[i], hld->lights );
			if ( i!=0 )	hld->body[i].objs->flag |= DG_FLAG_INVISIBLE ;

#if HOLDENE_VITA_RENDER_OPT
         renderListHeadsLOD[i] = buffer_objs_for_opt_render( renderListHeadsLOD[i], hld->body[i].objs );
#endif
      }
		hld->c_obj = 0 ;	/* 初期表示オブジェ番号 */
		DG_SetPos2( &hld->pos, &hld->rot ) ;
		DG_PutObjs( hld->body[0].objs ) ;
		DG_GetLightMatrix( &hld->pos, hld->lights );
		hld->mov = hld->pos ;
		hld->m_time = KR_RandU( 600 ) ;

		/* 武器 */
		name = WP_NAME_1 ;
		if ( hld->items & ITEM_WEAPON ) {
			if ( hld->items & ITEM_WP_1 ) {
				name = WP_NAME_1 ;
			} else if ( hld->items & ITEM_WP_2 ) {
//				name = WP_NAME_2 ;
			} else if ( hld->items & ITEM_WP_3 ) {
//				name = WP_NAME_3 ;
			}
			NewConnectObject2( &hld->cnct, &hld->body[0], HUMAN21_MUNE,
					&HOLD_WP_Shift[WP_POS_SHOLDER], &HOLD_WP_Rot[WP_POS_SHOLDER], name ) ; 

#if HOLDENE_VITA_RENDER_OPT
         renderListWeapons = buffer_objs_for_opt_render( renderListWeapons, hld->cnct.body.objs );
#endif
      }

		/* その他アイテム */
#ifdef NO_HELM
		UNSET_FLAG( hld->items, ITEM_ATAMA ) ;
#endif
		if ( hld->items & ITEM_ATAMA ) {
			if ( hld->items & ITEM_CAP ) {
				name = CAP_NAME ;
			} else if ( hld->items & ITEM_HELMET ) {
				name = HELMET_NAME ;
			} else if ( hld->items & ITEM_SCOT_CAP ) {
				name = SCOT_CAP_NAME ;
			} else if ( hld->items & ITEM_HEADPHONES ) {
				name = HEADPHONES_NAME ;
			}
			GM_InitObject( &hld->atama, name, ITEMS_FLAG ) ;
			GM_ConfigObjectRoot( &hld->atama, &hld->body[0], HUMAN21_ATAMA ) ;
//hld->atama.objs->flag |= DG_FLAG_INVISIBLE ;

#if HOLDENE_VITA_RENDER_OPT
         renderListAtama = buffer_objs_for_opt_render( renderListAtama, hld->atama.objs );
#endif
      }
		if ( hld->items & ITEM_GLASS ) {
			GM_InitObject( &hld->glass, GLASS_NAME, ITEMS_FLAG ) ;
			GM_ConfigObjectRoot( &hld->glass, &hld->body[0], HUMAN21_ATAMA ) ;
#if HOLDENE_VITA_RENDER_OPT
         renderListGlass = buffer_objs_for_opt_render( renderListGlass, hld->glass.objs );
#endif
      }
		hld->think = TH_STAND ;
		hld->count = 0 ;
		hld->c_mot = 0 ;
		hld->mot_num = -1 ;	/* 初期化 */
		hld->headmark = NULL ;
		SetNextMotion( hld ) ;
		height = (float)*(WORKP->m_height + WORKP->m_ptr[hld->mot_num] ) ;
		hld->mov.vy = hld->pos.vy + (height*hld->scale);
		hld->adj_y = hld->adj_x = hld->adj_turn_y = hld->adj_turn_x = 0 ;
		hld->face_dir = hld->rot.vy ;

		MakeMotion( work, hld ) ;

		GM_InitRadarControl( &hld->rctrl, &hld->mov, RADAR_VISIBLE|RADAR_SIGHT, -1 );
//		GM_RadarSetVRange( &hld->rctrl, RADAR_VRANGE_UPPER , RADAR_VRANGE_LOWER );
//		GM_RadarSetVRange( &hld->rctrl, 2000.0f , -2000.0f );
		GM_RadarSetVRange( &hld->rctrl, 3000.0f , -2000.0f );

		InitTarget( hld ) ;

		/* 腰の位置からもう一度位置セット */
		DG_SetPos2( &hld->mov, &hld->rot ) ;
		for( i=0; i<MAX_LOD; i++ ) {
			DG_PutObjs( hld->body[i].objs ) ;
		}

		/* コントロール初期化 */
		InitControl( hld ) ;

		InitHoming( hld ) ;

#ifdef ROOT_WALKER
		{
			int x, y, z ;
			
			/* フロート比較になるがシナリオから受け取って値を弄ってないので大丈夫なはず */
			x = (int)hld->pos.vx ;
			y = (int)hld->pos.vy ;
			z = (int)hld->pos.vz ;
			if ( (x == 0) && (y == 0) ) {	/* 巡回兵 */

				hld->think = TH_ACTIVE ;
				hld->status |= HLD_STATUS_ACTIVE ;	/* 活動開始 */

				if( hld->mdl_num == MODEL_SCOT ) SET_FLAG( hld->status, HLD_STATUS_SCOT ) ;
printf("Set Root Ene Set Name[%d]\n",hld->name ) ;

				CNCT_CnctObjNum( &hld->cnct, HUMAN21_MIGI_TE ) ;
				CNCT_CnctObjShift( &hld->cnct, &HOLD_WP_Shift[WP_POS_RIGHT_HAND] ) ;
				CNCT_CnctObjRot( &hld->cnct, &HOLD_WP_Rot[WP_POS_RIGHT_HAND] ) ;

				hld->active = NewRootEnemy( &hld->body[0], &hld->cnct.body, 
					&hld->mov, hld->rot.vy, &hld->abs_rots[0], hld->name,
					&hld->status, z, &hld->control, hld ) ;
				SetActiveNum( ) ;
				if( hld->mdl_num == MODEL_SCOT ) {
					work->scot_pos = &(GM_SearchWhere( hld->name )->mov) ;
				}
			}
			if ( hld->body[0].evmobj != NULL ) {
				SET_FLAG( hld->body[0].objs->flag, DG_FLAG_INVISIBLE )  ;
				UNSET_FLAG( hld->body[0].evmobj->flag, DG_EVMOBJ_INVISIBLE ) ;
			} else {
				UNSET_FLAG( hld->body[0].objs->flag, DG_FLAG_INVISIBLE )  ;
			}
			SET_FLAG( hld->body[1].objs->flag, DG_FLAG_INVISIBLE ) ;
			if ( hld->body[1].evmobj != NULL ) {
				SET_FLAG( hld->body[1].evmobj->flag, DG_EVMOBJ_INVISIBLE ) ;
			}
			SET_FLAG( hld->body[2].objs->flag, DG_FLAG_INVISIBLE ) ;
			if ( hld->body[2].evmobj != NULL ) {
				SET_FLAG( hld->body[2].evmobj->flag, DG_EVMOBJ_INVISIBLE ) ;
			}

		}

		if ( hld->mdl_num == MODEL_PANTS ) { 
			extern int	PL_SetDGCameraCheckChara( OBJECT *object, 
				int joint, int proc, float far ) ;
				//object  : オブジェクト
				//joint   : 関節番号（パンツ兵なら HUMAN21_KOSHI）
				//proc    : 判定結果をシナリオに返すためのプロック
				//far     : 映っているが「遠すぎる」と判定される、
				//			画面に占める割合（％）。2くらいを設定しておけばＯＫ。

				/* 最初の見るべき場所 */
			int	pants_proc ;

			pants_proc = GCL_GetOptionValue( 'b', 0 ) ;
			PL_SetDGCameraCheckChara( &hld->body[0], HUMAN21_KOSHI, pants_proc, 2.0f ) ;
		}
#endif

#ifdef DEBUG_MODE
//		GV_SetActorChild( work, NewBig_Add_Objnum( &(hld->mov), &(hld->head_num) ) ) ;
		hld->head_num = 2 ;	/* 黄色 */
		hld->head_num |= hld->c_obj << 3 ;
#endif
		hld ++ ;
	}

//   DG_AS_DumpPrecomputedChainBufferObjInfo( renderListHeadsLOD[0] );
//   DG_AS_DumpPrecomputedChainBufferObjInfo( renderListWeapons );
//   DG_AS_DumpPrecomputedChainBufferObjInfo( renderListAtama );
//   DG_AS_DumpPrecomputedChainBufferObjInfo( renderListGlass );


	/* ＴＶカメラ スコットのコントロール取得後に設置すること */
	work->camera_num = 0 ;
	if ( ( opt = GCL_GetOption( 'c' ) ) != NULL ){
		int	dir, trg_name, cameraman ;

		ENE_GCL_GetFV( opt, &work->camera_pos[0] ) ;		/* 場所 */
		dir = GCL_GetNextInt( ) ;	/* 方向 */
		cameraman = GCL_GetNextInt( ) ;	/* カメラマン */
		trg_name = GCL_GetNextInt( ) ;	/* ターゲット名 */
		work->cameraman[0] = NameToID( work, cameraman ) ;
		if (work->cameraman[0] < 0 ) {
			printf( " cameraman[0] name Err[%d] \n",cameraman ) ;
			return -1 ;
		}
		NewHoldCamera( where, &work->camera_pos[0], dir, trg_name, &work->gameflag,0,
			&work->holdene[ work->cameraman[0] ].status2 ) ;
		SET_FLAG( work->holdene[ work->cameraman[0] ].status, HLD_STATUS_CAMERAMN ) ;
		work->holdene[ work->cameraman[0] ].mot_type  = MASTER_NUM ;
		work->holdene[ work->cameraman[0] ].think = TH_CAMERA ;
		work->holdene[ work->cameraman[0] ].think2 = TH2_IDLE ;

		work->camera_num ++ ;

		if( GCL_NextStr() != NULL ) {
			ENE_GCL_GetFV( opt, &work->camera_pos[1] ) ;		/* 場所 */
			dir = GCL_GetNextInt( ) ;	/* 方向 */
			cameraman = GCL_GetNextInt( ) ;	/* カメラマン */
			trg_name = GCL_GetNextInt( ) ;	/* ターゲット名 */

			work->cameraman[1] = NameToID( work, cameraman ) ;
			if (work->cameraman[1] < 0 ) {
				printf( " cameraman name Err[%d] \n",cameraman ) ;
				return -1 ;
			}
			NewHoldCamera( where, &work->camera_pos[1], dir, trg_name, &work->gameflag,1,
				&work->holdene[ work->cameraman[1] ].status2 ) ;
			SET_FLAG( work->holdene[ work->cameraman[1] ].status, HLD_STATUS_CAMERAMN ) ;
			work->holdene[ work->cameraman[1] ].mot_type  = MASTER_NUM ;
			work->holdene[ work->cameraman[1] ].think = TH_CAMERA ;
			work->holdene[ work->cameraman[1] ].think2 = TH2_IDLE ;
			work->camera_num ++ ;
		}
	}
	

	work->scn_num = 0 ;
	work->pose_type = GSCN_NONE ;
	switch ( watch ) {
		case 0:
			work->aim_type = GSCN_SCREEN1 ;
		break ;
		case 1:
			work->aim_type = GSCN_SCREEN2 ;
		break ;
		case 2:
			work->aim_type = GSCN_SCOT ;
		break ;
	}

	work->count = 0 ;
	work->detect_delay = 0 ;

printf("hold ene size=%d!!\n",sizeof( HOLD_ENE ) ) ;


printf("aaaaaaaaaaaaaaasaction size [%d] HOLDBODYPARAM[%d]\n",sizeof(ACTION),sizeof(HOLDBODYPARAM) ) ;

	return 0 ;
}

	/*
		シナリオ呼び出し
	*/
void		*NewHoldEnemy( name, where )
int		name ;
int		where ;
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &( work->actor ) ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

