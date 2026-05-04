//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	hostage.c
	人質

	2001/03/21 Y.Korekado
	$Id: hostage.c,v 1.1.1.3 2002/11/19 11:44:17 Yoshizawa1 Exp $
*/
/*----------------------------------------------------------------
void	*NewHostage( name, where )
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
#include	"korekado/npc/npc.h"
#include	"../../kano/hostage/hostage.h"


/*----------------------------------------------------------------*/
#define SMP_FATMAN

#define MODEL_NAME	(6371217)	/* GV_StrCode( "gbs_def" ) */
#define BASE_MOTION	(7056426)	/* GV_StrCode("hostage") */

#define FEMALE_A	(828773)	/* GV_StrCode("hos_femalea_def_mt") */
#define FEMALE_B	(830821)	/* GV_StrCode("hos_femaleb_def_mt") */

#define RIC_DEF		(6371934)	/* GV_StrCode("ric_def") */
#define RIC_MID		(6381276)	/* GV_StrCode("ric_mid") */
#define RIC_LOW		(6380463)	/* GV_StrCode("ric_low") */
#define RIC_NO_BLIND		(3193701)	/* GV_StrCode("ric_def_sh") */

#define GREEN_NECKTIE	(572523)	/* GV_StrCode("hos_tie_green") */
#define BROWN_NECKTIE	(12117673)	/* GV_StrCode("hos_tie_browm") */

#define	ITEMS_FLAG	(DG_FLAG_ONEPIECE)

#define SMP_LIFE	(128)
#define SMP_FAINT	(5)
#define SMP_FAINT_COUNT	(60*30)
#define SMP_SLEEP_COUNT	(60*60*1 + 3)
#define SMP_DOWN_DAM	(10)

#include	"hostage.h"
/*----------------------------------------------------------------*/
/* base_mar */
enum {
	HSTG_MOT_SIT,			/* 座る */
	HSTG_MOT_DAM,			/* ダメージ */
	HSTG_MOT_SLEEP,			/* 首かっくん */
	HSTG_MOT_SLEEP_IDLE,	/* 眠り中 */
	HSTG_MOT_WAKE_UP,		/* 起きる */
	HSTG_MOT_STRAIN,		/* 緊張する */
	HSTG_MOT_OUT,			/* 死亡 */
	HSTG_MOT_STRUGGLE,		/* 暴れる */
	HSTG_MOT_2_STRAIGHT_FOOT,	/* 足伸ばす */
	HSTG_MOT_STRAIGHT_FOOT,		/* 足伸しっぱ */
	HSTG_MOT_BACK_FOOT,		/* 足伸引っ込める */

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
	{ 500.0F, 650.0F, 500.0F },		/* しゃがみ */
} ;

/* ターゲットシフト（しゃがみ状態） */
static FVECTOR HostageTrgShift = { 0.0F, 0.0F, 400.0F } ;	/* 立ち */

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

/*----------------------------------------------------------------*/
#include	"hstgpre.c"
#include	"hstgact.c"
#include	"hstgthink.c"
/*----------------------------------------------------------------*/
#define HSTG_LOD_MID_DIS	( 4000 )
#define HSTG_LOD_LOW_DIS	( 6000 )

extern void *NewEvm_SkirtA( OBJECT * ); // ネーちゃん
extern void *NewEvm_SkirtB( OBJECT * ); // おばはん

static	void	LodHostage( Work *work )
{
	OBJECT	*body ;
	int cam_dis ;

	if ( !(work->status & HSTG_STATUS_LOD) ) return ;

	if ( work->model_num >= 11 ) return ;

	cam_dis = KR_CameraDis( &work->control.mov ) ;
	body = &work->body ;

//printf("cam_dis[%d] [%f][%f][%f]\n",cam_dis,work->control.mov.vx,work->control.mov.vy,work->control.mov.vz ) ;	
	if ( cam_dis > HSTG_LOD_LOW_DIS ) {
		UNSET_FLAG( body->objs->flag, DG_FLAG_INVISIBLE ) ;
		SET_FLAG( work->lod_body[0].objs->flag, DG_FLAG_INVISIBLE ) ;
		SET_FLAG( work->lod_body[1].objs->flag, DG_FLAG_INVISIBLE ) ;

		return ;
	}

	if ( cam_dis > HSTG_LOD_MID_DIS ) {
		SET_FLAG( body->objs->flag, DG_FLAG_INVISIBLE ) ;
		SET_FLAG( work->lod_body[0].objs->flag, DG_FLAG_INVISIBLE ) ;
		UNSET_FLAG( work->lod_body[1].objs->flag, DG_FLAG_INVISIBLE ) ;

		_CopyObjectWorld( body->objs, work->lod_body[1].objs ) ;
		DG_SetPos2( &work->control.mov, &work->control.rot );
		DG_PutObjs( work->lod_body[1].objs ) ;
		return ;
	}

	SET_FLAG( body->objs->flag, DG_FLAG_INVISIBLE ) ;
	UNSET_FLAG( work->lod_body[0].objs->flag, DG_FLAG_INVISIBLE ) ;
	SET_FLAG( work->lod_body[1].objs->flag, DG_FLAG_INVISIBLE ) ;

	_CopyObjectWorld( body->objs, work->lod_body[0].objs ) ;
	DG_SetPos2( &work->control.mov, &work->control.rot );
	DG_PutObjs( work->lod_body[0].objs ) ;
}

static void HostageMain( Work *work )
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
	HostageMain( work ) ;

	/* ＮＰＣ後処理 */
	NPC_AfterProcess( npc ) ;

	LodHostage( work ) ;

#if 0
	npc->lights[1].m[3][0] = 128 ;
	npc->lights[1].m[3][1] = 128 ;
	npc->lights[1].m[3][2] = 128 ;
#endif
}

static	void	Die( work )
Work		*work ;
{
	NPCWORK *npc ;
	
	npc = &work->npc ;

	TAKABE_FreePuppetIK( npc->action.ikwork ) ;
    GM_FreeTarget( npc->target.deftrg ) ;	/* 親だけで良い */
    GM_FreeControl( npc->ctrl ) ;
	if ( work->model_num == 0 ) {
		GM_FreeObject( &work->body ) ;
		if ( work->status & HSTG_STATUS_LOD ) {
			GM_FreeObject( &work->lod_body[0] ) ;
			GM_FreeObject( &work->lod_body[1] ) ;
		}
	} else if ( work->model_num < 11 ) {
		FreeObject_Hostage( npc->body, &work->object_chg[0] ) ;
		if ( work->status & HSTG_STATUS_LOD ) {
			FreeObject_Hostage( &work->lod_body[0], &work->object_chg[1] ) ;
			FreeObject_Hostage( &work->lod_body[1], &work->object_chg[2] ) ;
		}
	} else {
		GM_FreeObject( &work->body ) ;
	}

	if ( work->status & HSTG_STATUS_N_GREEN ) {
		GM_FreeObject( &work->necktie ) ;
	}
	if ( work->status & HSTG_STATUS_N_BROWN ) {
		GM_FreeObject( &work->necktie ) ;
	}

    if( npc->rctrl != NULL ) GM_FreeRadarControl( npc->rctrl ) ;
	if( npc->fcanim != NULL ) FC_ReleaseFaceControl( npc->fcanim ) ;
}
/*----------------------------------------------------------------*/
static	void	SetOnCorp( Work *work )
{
	static FVECTOR sft = { 0,0,500 } ;
	DG_SetPos2( &work->control.mov, &work->control.rot );
	DG_PutVector( &sft, &work->on_corp, 1 ) ;
}

static	int	GetResources( work, name, where )
Work	*work ;
int		name ;
int		where ;
{
	extern void	NPC_SetTargetCallBack( NPCWORK *npc ) ;
	NPCWORK *npc ;
	FVECTOR	start_pos, trg_shift ;
	SVECTOR start_rot ;
	int		start_dir, mot_name ;
	char *opt ;
	
	npc = &work->npc ;

	if ( (work->id = HSTG_SetHostage( work )) < 0 ) {
		printf("Hostage: Err Hostage Num Over\n" ) ;
		return -1 ;
	}

	/* シナリオから値を取得 */
	if ( (mot_name = GCL_GetOptionValue( 'm', 0 )) == 0 ) {
		return -1 ;
	}
	if ( (work->model_num = GCL_GetOptionValue( 'k', -1 )) < 0 ) {
printf("model_num=[%d]\n",work->model_num ) ;
		return -1 ;
	}
	work->status = GCL_GetOptionValue( 's', 0 ) ;
	work->vc_end_proc = GCL_GetOptionValue( 'e', 0 ) ;

	/* NPC初期化 */
	NPC_InitNPC( npc, work, &work->body, &work->control, work->lights ) ;
	npc->actmode_call = ActSit ;

	/* コントロールの初期化 */
	GM_InitControl( npc->ctrl, name, 0 ) ;
	npc->ctrl->hzx_height = 750 ;
	npc->ctrl->height = 1049.0F ;
	GM_ConfigControlHazard( npc->ctrl, 1200, 450, 500 ) ;
	GM_ConfigControlMessageCheck( npc->ctrl ) ;
	SET_FLAG( npc->ctrl->seg_flag, HZX_TYPE_ENEMY ) ;
//	SET_FLAG( npc->ctrl->skip_flag, (CTRL_SKIP_SEG_CHECK) ) ;
	SET_FLAG( npc->ctrl->skip_flag, (CTRL_SKIP_FLR_CHECK|CTRL_SKIP_SEG_CHECK) ) ;
//	ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;

	/* オブジェクトの初期化 */
	work->model_num = work->model_num%14 ;

	if ( work->model_num == 0 ) {	/* 大統領 */
		if ( work->status & HSTG_STATUS_LOD ) {
			GM_InitObject( npc->body, RIC_LOW, BODY_FLAG  );
			GM_InitObject( &work->lod_body[0], RIC_DEF, BODY_FLAG  );
			GM_InitObject( &work->lod_body[1], RIC_MID, BODY_FLAG  );
			GM_ConfigObjectLight( &work->lod_body[1], npc->lights );
			GM_ConfigObjectLight( &work->lod_body[0], npc->lights );
		} else {
			GM_InitObject( npc->body, RIC_DEF, BODY_FLAG  );
		}
		SET_FLAG( work->status, HSTG_STATUS_RIC ) ;
	} else if ( work->model_num == 11 ) {
		GM_InitObject( npc->body, FEMALE_A, BODY_FLAG  );
		SET_FLAG( work->status, HSTG_STATUS_OL ) ;
	} else if ( work->model_num == 12 ) {
		GM_InitObject( npc->body, FEMALE_B, BODY_FLAG  );
		SET_FLAG( work->status, HSTG_STATUS_WOMEN ) ;
	} else if ( work->model_num == 13 ) {
		GM_InitObject( npc->body, RIC_NO_BLIND, BODY_FLAG  );
		SET_FLAG( work->status, HSTG_STATUS_RIC ) ;
	} else {
		if ( work->status & HSTG_STATUS_LOD ) {
			InitObject_LowHostage( npc->body, BODY_FLAG, &work->object_chg[0], work->model_num ) ;
			InitObject_Hostage( &work->lod_body[0], BODY_FLAG, &work->object_chg[1], work->model_num ) ;
			InitObject_MidHostage( &work->lod_body[1], BODY_FLAG, &work->object_chg[2], work->model_num ) ;
			GM_ConfigObjectLight( &work->lod_body[0], npc->lights );
			GM_ConfigObjectLight( &work->lod_body[1], npc->lights );
		} else {
			InitObject_Hostage( npc->body, BODY_FLAG, &work->object_chg[0], work->model_num ) ;
		}
	}

	GM_ConfigObjectLight( npc->body, npc->lights );
	GM_ConfigControlObject( npc->ctrl, npc->body ) ;

	/* 付属品初期化 */
	if ( work->status & HSTG_STATUS_OL ) {
	  	GV_SetActorChild( work , NewEvm_SkirtA( npc->body ) );
	}
	if ( work->status & HSTG_STATUS_WOMEN ) {
	  	GV_SetActorChild( work , NewEvm_SkirtB( npc->body ) );
	}
	if ( work->status & HSTG_STATUS_N_GREEN ) {
		GM_InitObject( &work->necktie, GREEN_NECKTIE, ITEMS_FLAG ) ;
		GM_ConfigObjectRoot( &work->necktie, npc->body, HUMAN21_MUNE ) ;
	}
	if ( work->status & HSTG_STATUS_N_BROWN ) {
		GM_InitObject( &work->necktie, BROWN_NECKTIE, ITEMS_FLAG ) ;
		GM_ConfigObjectRoot( &work->necktie, npc->body, HUMAN21_MUNE ) ;
	}


	/* モーションの初期化 */
	NPC_InitMotion( npc, mot_name, 0, 0, 0 ) ;

	/* 方向のセット */
	start_dir = GCL_GetOptionValue( 'd', 0 ) ;

	start_rot.vx = 0 ;
	start_rot.vy = start_dir ;
	start_rot.vz = 0 ;
	DG_SetPos2( &DG_ZeroVector, &start_rot ) ;
	DG_PutVector( &HostageTrgShift, &trg_shift, 1 ) ;

	/* ターゲットの初期化 */
	NPC_InitDefenceTarget( npc, &work->deftrg, ENEMY_SIDE, SampleTrgSize, SampleDragShift, &trg_shift ) ;
	NPC_InitDefenceChildTarget( npc, work->def_child, ENEMY_SIDE, 
		Cheild_Target_Size, Cheild_Target_Shift, Child_Level_Num, Cheild_Target_Connect ) ;
	NPC_SetTargetCallBack( npc ) ;

//	NPC_InitDefenceCapture( npc, &work->capture, &work->control, &work->body ) ;
	NPC_ChangeTargetSize( npc, 2 ) ;	/* しゃがみ */

	/* 初期位置、モーションのセット */
	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
		ENE_GCL_GetFV( opt, &start_pos ) ;
	} else {
		start_pos = GM_PlayerPosition ;
		start_pos.vz -= 1500 ;
	}
	NPC_InitPose( npc, &start_pos, start_dir, BASE_MOTION, 0 ) ;

	/* レーダー情報初期化 */
	NPC_InitRader( npc, &work->rctrl, &npc->ctrl->mov, 
			2000.0f, -2000.0f, 0, 0, RADAR_COLOR_YELOW ) ;

	/* ヘッドマークコントロール初期化 */
	NPC_InitHeadMark( work, npc, &work->headmark, &work->body, &work->control, &work->deftrg ) ;

	/* パラメータセット */
	NPC_SetActionParam( &npc->action, SMP_LIFE, SMP_FAINT, SMP_FAINT_COUNT,
				SMP_SLEEP_COUNT, SMP_DOWN_DAM  ) ;

	/* モーションアジャスト */
	NPC_InitMotionAdjust( npc, &work->npcadjust ) ;
	SET_FLAG( npc->status, NPC_STATUS_HOSTAGE ) ;

	/* 思考処理初期化 */
	StartThink( work ) ;

	/* ゆっくり歩かせる場所 */
	SetOnCorp( work ) ;

	work->quest_time = 0 ;
	work->mail_time = 0 ;
	work->mail_seed = 0 ;

	work->heart_int = COUNT_VMODE(120) + KR_RandS( 60 ) ;
	work->heart_count = 0 ;
	if ( work->status & HSTG_STATUS_RIC ) {
		work->heart_int = COUNT_VMODE(70) ;
	}

	work->se_interval = 0 ;

	return 0 ;
}

	/*
		シナリオ呼び出し
	*/
void		*NewHostage( name, where )
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
