//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	gll.c
	ゴルルゴン

	2002/04/04 Y.Korekado
	$Id: gll.c,v 1.3 2002/11/23 12:42:27 Yoshizawa1 Exp $
*/
/*----------------------------------------------------------------
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
#include	"head.h"
//『既存使い回し』
//SD_V_RAISIB01 //ライデン影縛られ１「動けない」
//SD_V_RAISIB02 //ライデン影縛られ２「体が！？」
//SD_E_V_KAGE02 //影縛りダメージ

void GLL_AchooVibration( void );
void GLL_WalkVibration( void );
extern void VR_AddEnemy(void);
extern void		*NewFootWall( void );

/*----------------------------------------------------------------*/
#ifdef DEBUG_MODE
//#define GLL_TARGET_VIEW (1)

#include	"debugmenu.h"

#define GLL_DEBUG_EYE_VIEW		0x00000001
#define GLL_DEBUG_EYE_CLOSE		0x00000002
#define GLL_DEBUG_BODY_LIGHT	0x00000004
#define GLL_DEBUG_THINK			0x00000008

static	int	debug_flag = 0 ;

/* 視界コントロール */
static char *eye_ctrl_items[] =
	{ "OPEN", "CLOSE" };
static int  eye_ctrl_values[] =
	{ 0, GLL_DEBUG_EYE_CLOSE };
static GM_DEBUG_MENU eye_ctrl = {
	NULL, /* next */
 	"GLL", /* class */
	"EYE CONTROL",
	eye_ctrl_items,/*items*/
	eye_ctrl_values, /* values */
	NULL,	/*target*/
	GLL_DEBUG_EYE_CLOSE,/*mask*/
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	2, /*max*/
	0, /*current*/
	0  /*padding */
} ;

/* 体 光らす */
static char *body_light_items[] =
	{ "OFF", "ON" };
static int  body_light_values[] =
	{ 0,GLL_DEBUG_BODY_LIGHT };
static GM_DEBUG_MENU body_light = {
	NULL, /* next */
 	"GLL", /* class */
	"BODY LIGHT",
	body_light_items,/*items*/
	body_light_values, /* values */
	NULL,	/*target*/
	GLL_DEBUG_BODY_LIGHT,/*mask*/
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	2, /*max*/
	0, /*current*/
	0  /*padding */
} ;

/* 視界表示 */
static char *eye_view_items[] =
	{ "OFF", "ON" };
static int  eye_view_values[] =
	{ 0,GLL_DEBUG_EYE_VIEW };
static GM_DEBUG_MENU eye_view = {
	NULL, /* next */
 	"GLL", /* class */
	"EYE VIEW",
	eye_view_items,/*items*/
	eye_view_values, /* values */
	NULL,	/*target*/
	GLL_DEBUG_EYE_VIEW,/*mask*/
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	2, /*max*/
	0, /*current*/
	0  /*padding */
} ;

/* 思考 */
static char *think_deb_items[] =
	{ "OFF", "ON" };
static int  think_deb_values[] =
	{ 0, GLL_DEBUG_THINK };
static GM_DEBUG_MENU think_deb = {
	NULL, /* next */
 	"GLL", /* class */
	"THINK",
	think_deb_items,/*items*/
	think_deb_values, /* values */
	NULL,	/*target*/
	GLL_DEBUG_THINK,/*mask*/
	NULL,/*func*/
	0, /*strid*/
	0, /*type*/
	2, /*max*/
	0, /*current*/
	0  /*padding */
} ;
#endif
/*----------------------------------------------------------------*/
#include	"korekado/npc/npc.h"
#include	"gll.h"
#include	"gll_def.h"
/*----------------------------------------------------------------*/
int		GLL_GAME_STATUS ;
int		GLL_ACHOO_TIME ;
FMATRIX	GllEyebeemWorld ;
int		GllEyebeemFlag ;
/*----------------------------------------------------------------*/
extern void *AN_HeadMark( FMATRIX *world, int flag );
static int StatusFightGME ( int status ) ;
static int StatusFightGNO ( int status ) ;
static int StatusFightGLL ( int status ) ;
static void ChangeGun ( Work *work, int hand ) ;
static	FVECTOR	GllHeadPos ;	/* 頭の位置 */
static	int		BreakParts ;	/* 壊れた部位 */
/*----------------------------------------------------------------*/
#include	"gllpre.c"
#include	"gllthink.c"
/*-------------------------------------------------------------------*/

int GetGllHeadPos( void )
{
	GCL_VAR_REF ref; //配列への参照データ

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)GllHeadPos.vx );

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)GllHeadPos.vy );

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, (int)GllHeadPos.vz );

	return 1 ;
}

int GetGllBreakParts( void )
{
	GCL_VAR_REF ref; //配列への参照データ

	if ( GCL_NextStr() == NULL ) return -1 ;
	GCL_GetNextVarRef( &ref ); // 参照データの取得
	GCL_SetVarRef( &ref, 0, BreakParts );

	return 1 ;
}

/*----------------------------------------------------------------*/
static	u_char	DamageVib2H[] = { 1, 20, 0, 0 } ;
static	u_char	DamageVib2L[] = { 255, 20, 0, 0 } ;
void GLL_DetectVibration( void )
{
	extern void	*NewPadVibration( char *script, int type ) ;

	NewPadVibration( DamageVib2H, 1 ) ;
	NewPadVibration( DamageVib2L, 2 ) ;
}

static	u_char	AchooVib2H[] = { 0, 35, 127, 27, 127, 10, 0, 0 } ;
static	u_char	AchooVib2L[] = { 94, 35, 255, 27, 222, 21, 106, 24, 0, 0 } ;
void GLL_AchooVibration( void )
{
	extern void	*NewPadVibration( char *script, int type ) ;

	NewPadVibration( AchooVib2H, 1 ) ;
	NewPadVibration( AchooVib2L, 2 ) ;
}

static	u_char	WalkVib2H[] = { 1, 20, 0, 0 } ;
static	u_char	WalkVib2L[] = { 128, 20, 64, 20,0, 0 } ;
void GLL_WalkVibration( void )
{
	extern void	*NewPadVibration( char *script, int type ) ;

	NewPadVibration( WalkVib2H, 1 ) ;
	NewPadVibration( WalkVib2L, 2 ) ;
}

static int StatusFightGME ( int status ) {
	if ( !(status & GLL_STATUS_FIGHT) ) return 0 ;
	if ( !(status & GLL_STATUS_GME) ) return 0 ;
	return 1 ;
}
static int StatusFightGNO ( int status ) {
	if ( !(status & GLL_STATUS_FIGHT) ) return 0 ;
	if ( !(status & GLL_STATUS_GNO) ) return 0 ;
	return 1 ;
}
static int StatusFightGLL ( int status ) {
	if ( !(status & GLL_STATUS_FIGHT) ) return 0 ;
	if ( !(status & GLL_STATUS_GLL) ) return 0 ;
	return 1 ;
}

static void ChangeGun ( Work *work, int hand )
{
	static FVECTOR LeftGunShift = { 83.3, 402.9, -2912.9 } ;
	static SVECTOR LeftGunRot = {-857, -351, 751 } ;
	CNCTOBJ *cnct ;

	cnct = &work->cnct ;
	if ( hand ) {	/* 左 */
		CNCT_CnctObjNum( cnct, HUMAN21_HIDARI_TE ) ;
		CNCT_CnctObjShift( cnct, &LeftGunShift ) ;
		CNCT_CnctObjRot( cnct, &LeftGunRot ) ;
		work->gunhand = 1 ;
	} else {		/* 右 */
		CNCT_CnctObjNum( cnct, HUMAN21_MIGI_TE ) ;
		CNCT_CnctObjShift( cnct, &DG_ZeroVector ) ;
		CNCT_CnctObjRot( cnct, &DG_ZeroSVector ) ;
		work->gunhand = 0 ;
	}
}

static void SonicPad( Work *work )
{
	extern int	PL_KageshibariNow( void ) ;
	extern void	PL_KageshibariTimeDecrease( void ) ;
	GV_PAD *pad ;
	int i ;

	if ( PL_KageshibariNow() ) {
		pad = GV_PadDataDirect ;
		for( i = 0; i < 16; i++ ){
			if( pad->press & (1<<i) ) {
				PL_KageshibariTimeDecrease( ) ;
				PL_KageshibariTimeDecrease( ) ;
				PL_KageshibariTimeDecrease( ) ;
			}
		}
	}
}

static int gameover_delay_count ;
#define GAME_OVER_DELAY	COUNT_VMODE(120)
TARGET			achoo_trg ;
POWER_TARGET	achoo_powtrg ;

static int AchooRange( FVECTOR *pos )
{
//return 0 ;
	if ( pos->vx < -1000.0 ) return 1 ;
	if ( pos->vx > 1000.0 ) return 1 ;
	return 0 ;
}

static void AchooAtack( Work *work )
{
	TARGET			*t ;
	POWER_TARGET	*p ;
	FVECTOR		force = { 0.0F, 0.0F, 300.0F } ;
	
	t = &achoo_trg ;
	p = &achoo_powtrg ;

	GM_SetTarget( t, TARGET_OFFENSE, 0, PLAYER_SIDE, 
				 &DG_ZeroVector, &DG_ZeroVector ) ;
	GM_SetTargetWeaponType( t, WP_BLOW | WP_NOGUARD | WP_NORECOVER ) ;
	GM_SetPowerTarget( t, p, POWER_EXPLODE, 255, 0, GM_VitalityMax, &force ) ;
//	GM_SetPowerTarget( t, p, POWER_EXPLODE, 255, 0, 0, &force ) ;
	
	GM_PutTarget( t ) ;
	GM_TargetSetDirectAttack( t, GM_PlayerTarget ) ;
}

static void AchooCheck( Work *work )
{
	if ( GLL_GAME_STATUS & GLL_GS_ACHOO ) {
		if ( AchooRange( &GM_PlayerPosition ) ) {
			gameover_delay_count = GAME_OVER_DELAY ;
			AchooAtack( work ) ;
//			GM_GameOverProcStart( NULL ) ;
		}
	}

	if ( gameover_delay_count >= 0 ) {
		if ( gameover_delay_count == 0 ) {
//			GM_GameOverProcEnd( NULL ) ;
		}
//		if ( gameover_delay_count <= (GAME_OVER_DELAY-COUNT_VMODE(30)) ) {
			/* ゲームオーバーにするのでプレイヤーつつき放題やる */
			GM_PlayerControl->skip_flag |= CTRL_SKIP_SEG_CHECK ;
			if ( gameover_delay_count > (GAME_OVER_DELAY-COUNT_VMODE(30)) ) {
				GM_PlayerControl->mov.vz += STEP_VMODE(200.0) ;
			}
			GM_PlayerControl->step.vy = 0.0 ;
//		}
		if ( gameover_delay_count == (GAME_OVER_DELAY-COUNT_VMODE(30)) ) {
		}
		if ( gameover_delay_count == (GAME_OVER_DELAY-COUNT_VMODE(45)) ) {
//			GM_SeSetMode( SD_V_POUT0001, &GM_PlayerControl->mov, GM_SEMODE_BOMB ) ;
		}

		gameover_delay_count -- ;
	}
}
/*----------------------------------------------------------------*/
static void GllMain( Work *work )
{
	if ( GM_VRStatus & GM_VR_IDLE ) return ;

	/* 前処理 */
	PreProcess( work ) ;
  	if ( !(work->status & GLL_STATUS_STOP) ) {
		/* 思考処理 */
		Think( work ) ;
		/* 行動処理 */
		Action( work ) ;
		/* 後処理 */
		AfterProcess( work ) ;
	}
}

static	void	Act( work )
Work		*work ;
{
	NPCWORK	*npc ;

	npc = &work->npc ;

	/* ＮＰＣシステム系コントロール処理 */
  	if ( !(work->status & GLL_STATUS_STOP) ) {
		NPC_ActControlExe( npc, 10 ) ;
	} else {
		/* メッセージチェックは行う */
		npc->ctrl->n_msg = GV_ReceiveMessage( npc->ctrl->name, &( npc->ctrl->msg ) ) ;
	}
	
	if ( work->status & GLL_STATUS_FIGHT ) {
		if ( !(work->status2 & GLL_STATUS2_NO_ZLIMIT) ) {
			if ( work->control.mov.vz > work->z_limit ) {
				work->control.mov.vz = work->z_limit ;
			}
			if ( work->control.mov.vz < work->z_limit - 2000.0 ) {
				work->control.mov.vz = work->z_limit - 2000.0 ;
			}
		} else {
//printf("work->control.mov.vz [%f] \n",work->control.mov.vz);
		}
	}

    /* ＳＥ変換用 */
	if ( !(work->status & GLL_STATUS_STOP) ) {
	    MT_SetMotionSeTable( npc->body->m_ctrl, GM_CurrentMap, (work->id)+5, 
				 ( npc->ctrl->flr_atrs[ 0 ] & 0xf0000000 ) >> 28, 0 ) ;
	}

	/* ＮＰＣ前処理 */
	NPC_PreProcess( npc ) ;

	/* キャラクターメイン処理*/
	GllMain( work ) ;

	/* ＮＰＣ後処理 */
//	NPC_AfterProcess( npc ) ;
	GLL_AfterProcess( work ) ;

  	if ( !(work->status & GLL_STATUS_STOP) ) {
		SonicPad( work ) ;
		AchooCheck( work ) ;
	}

	if ( work->status & GLL_STATUS_STOP ) {
		SET_FLAG( work->body.objs->flag,  DG_FLAG_INVISIBLE ) ;
		SET_FLAG( work->cnct.body.objs->flag,  DG_FLAG_INVISIBLE ) ;
		work->shadow = 0 ;
	} else {
		UNSET_FLAG( work->body.objs->flag,  DG_FLAG_INVISIBLE ) ;
		UNSET_FLAG( work->cnct.body.objs->flag,  DG_FLAG_INVISIBLE ) ;
	}
	KR_FMatToFvec( &BODYWORLD(&work->body, HUMAN21_ATAMA), &GllHeadPos ) ;
#ifdef DEBUG_MODE
	if( debug_flag & GLL_DEBUG_BODY_LIGHT ){
		work->lights[1].m[3][0] = 128 ;
		work->lights[1].m[3][1] = 128 ;
		work->lights[1].m[3][2] = 128 ;
		SET_FLAG( work->body.objs->flag,  DG_FLAG_NOFOG ) ;
	} else {
		UNSET_FLAG( work->body.objs->flag,  DG_FLAG_NOFOG ) ;
	}
#endif
}

static	void	Die( work )
Work		*work ;
{
	NPC_FreeResources( &work->npc ) ;
	GM_RemoveGageSet( &work->gage ) ;
}
/*----------------------------------------------------------------*/
static	void	TargetCallBack( off, def, ptr )
TARGET		*off, *def ;
void		*ptr ;
{
	Work	*work ;
	NPCWORK *npc ;
	
	work = (Work *)ptr ;
	npc = &work->npc ;

	if ( def->damaged & TARGET_POWER ) {
		if ( off->class & TARGET_POWER ) {	/* こいつに攻撃された */
			def->power->force = DG_ZeroVector ;
			GM_DamageTarget( off, def ) ;
//			entk->act->bodyp.off_center = off->center ;
//			def->power->force = off->power->force ;
//printf("child def->weapon_type[%lx] off[%lx]\n",def->weapon_type,off->weapon_type);
//printf("Ko [%d]force [%f][%f][%f] \n",GV_Time, def->power->force.vx,def->power->force.vy,def->power->force.vz );
		}
	}
}

void	GLL_InitDefenceTarget( Work *work, NPCWORK *npc, TARGET *child_trg, POWER_TARGET *power,
			int side, FVECTOR *size, FVECTOR *shift, int *connect )
{
	extern void *NewGolBreakPart(DG_OBJ *,float,int *sw,int,float) ;
	extern void *NewGolBreakPart2(DG_OBJ *obj,int *sw,float len1 ,float len2 ,
	int time_a1,int time_a2,float scale1,float scale2,int re_time) ;
	int		cnt = 18;	/*変形にかかる時間*/
	float	gme_len = 10.0f ;		/*パーツの離れる距離*/
	float	gme_scale = 0.9f ;		/*パーツのサイズ 0.0で完全消滅*/
	float	gll_len = 10.0f ;		/*パーツの離れる距離*/
	float	gll_scale = 0.4f ;		/*パーツのサイズ 0.0で完全消滅*/
	float	len, scale ;
	NPCTARGET *npctrg ;
	TARGET	*child  ;
	int i ;

	npctrg = &npc->target ;
	npctrg->def_child = child = child_trg ;

	len = gll_len ;
	scale = gll_scale ;
	if ( StatusFightGME (work->status ) ) {
		len = gme_len ;
		scale = gme_scale ;
	}


	for( i=0; i<GLL_TARGET_NUM; i++ ) {
		GM_SetTarget( child, GLL_TARGET_CLASS, 1, side,  size, shift ) ;
		GM_SetTargetWeaponType( child, 0 ) ;
	    GM_SetPowerTarget( child, power, POWER_DECREASE, 5, 3, 3, &DG_ZeroVector ) ;
		GM_PutTarget( child ) ;
		GM_SetTargetCallBack( child, TargetCallBack, work ) ;
		npctrg->connect_obj[i] = *connect ;
		child->name = 800 ;

		work->vanim_flag[i] = 0 ;		/*制御フラグ 0で復元 1で変形 -1で終了 */
		if ( work->status & GLL_STATUS_FIGHT ) {
			if ( StatusFightGLL( work->status ) ) {
				GV_SetActorChild( work, 
					NewGolBreakPart2(&(work->body.objs->objs[npctrg->connect_obj[i]]),
					&work->vanim_flag[i], 
					10.0,	/* ダメージ中のパーツ距離 */
					13.0,	/* 破壊時ののパーツ距離 */
					30,		/* ダメージおよび破壊への変形にかかる時間 */
					GLL_REGENE_TIME - GLL_REGENE_START_TIME,	/* 再生への変形にかかる時間 */
					0.4,	/* ダメージ中のパーツサイズ */
					0.8,	/* 破壊時のパーツサイズ */
					GLL_REGENE_START_TIME ) ) ;/* ダメージ中から再生開始までの時間 */
			} else {
				GV_SetActorChild( work, 
					NewGolBreakPart(&(work->body.objs->objs[npctrg->connect_obj[i]]),len, 
						&work->vanim_flag[i] ,cnt,scale) ) ;
			}
		}

#ifdef GLL_TARGET_VIEW
NewTargetView( child, 200, 34, 184 ) ;
#endif
		child ++ ;
		power ++ ;
		size ++ ;
		shift ++ ;
		connect ++ ;
	}
	npctrg->child_trg_num = GLL_TARGET_NUM ;
	npc->target.capture = NULL ;

	for( i=0; i<GLL_TARGET_NUM; i++ ) {
		work->trg_time[ i ] = 0 ;
	}
}

#define OBJECT_FLAG (DG_FLAG_IRREACTION|DG_FLAG_TRANS|DG_FLAG_SHADE|DG_FLAG_FINISHCALC)
void	GLL_InitObject( NPCWORK *npc, int modelname )
{
	OBJECT		*body ;
	CONTROL		*ctrl ;

	body = npc->body ;
	ctrl = npc->ctrl ;

	/* 関節型モデルの初期化 */
	GM_InitObject( body, modelname, OBJECT_FLAG  );
	GM_ConfigObjectLight( body, npc->lights );

	/* object control の関連付け */
	GM_ConfigControlObject( ctrl, body ) ;
}

static	int	GetResources( work, name, where )
Work	*work ;
int		name ;
int		where ;
{
	extern void	*NewGllHead( FMATRIX *gl_head, int *flag, int *spot, FMATRIX *lights ) ;
	extern void *NewConnectObject( OBJECT *cnct_obj, int cnct_num, FVECTOR *shift, SVECTOR *rot, int model ) ;
	NPCWORK *npc ;
	FVECTOR	start_pos ;
	char *opt ;
	int model, value ;
	
	npc = &work->npc ;

	GLL_ACHOO_TIME = 0 ;
	GLL_GAME_STATUS = 0 ;
	SET_FLAG( GLL_GAME_STATUS, GLL_GS_SEARCH_LIGHT_IN ) ;
	gameover_delay_count =  -1 ;
	BreakParts = 0 ;
	GllEyebeemFlag = 0 ;

	/* ステータス */
	work->status = GCL_GetOptionValue( 's', 0 ) ;

	/* 死亡ぷろっく */
	work->end_proc = GCL_GetOptionValue( 'e', 0 ) ;

	/* 特殊ゲームオーバーぷろっく */
	work->gover_proc = GCL_GetOptionValue( 'g', 0 ) ;

	/* 特殊クリアぷろっく */
	work->clear_proc = GCL_GetOptionValue( 'c', 0 ) ;

	/* モデル */
	value = GCL_GetOptionValue( 'm', 0 ) ;
	switch( value ) {
		case 1 :
			model = GLL_MODEL ;
			SET_FLAG( work->status, GLL_STATUS_GLL ) ;
			work->id = 0 ;
		break ;
		case 2 :
			model = GME_MODEL ;
			SET_FLAG( work->status, GLL_STATUS_GME ) ;
			work->id = 1 ;
		break ;
		default :
			model = GNO_MODEL ;
			SET_FLAG( work->status, GLL_STATUS_GNO ) ;
			work->id = 2 ;
//model = GME_MODEL ;
		break ;
	}

//	work->id = GCL_GetOptionValue( 'i', 0 ) ;

	/* 視力 */
	work->eye_length = GCL_GetOptionValue( 'l', GLL_DEF_EYE_LENGTH ) ;
	work->eye_range = GLL_DEF_EYE_RANGE ;

	/* NPC初期化 */
	NPC_InitNPC( npc, work, &work->body, &work->control, work->lights ) ;

	/* コントロールの初期化 */
	NPC_InitControl( npc, name ) ;

//SET_FLAG( npc->ctrl->skip_flag, CTRL_SKIP_FLR_CHECK ) ;

	/* オブジェクトの初期化 */
	GLL_InitObject( npc, model ) ;

	/* モーションの初期化 */
	NPC_InitMotion( npc, BASE_MOTION, 0, 0, 0 ) ;

	/* ターゲットの初期化 */
#if 1
	NPC_InitDefenceTarget( npc, &work->deftrg, ENEMY_SIDE, SampleTrgSize, &DG_ZeroVector, &DG_ZeroVector ) ;
	npc->target.deftrg->class |= TARGET_SKIP ;

	if ( StatusFightGNO( work->status ) ) {
		GLL_InitDefenceTarget( work, npc, work->def_child, work->def_power,
			ENEMY_SIDE, GNO_Target_Size, Cheild_Target_Shift, Cheild_Target_Connect ) ;
	} else if ( StatusFightGLL( work->status ) ) {
		GLL_InitDefenceTarget( work, npc, work->def_child, work->def_power,
//			ENEMY_SIDE, GLL_Target_Size, Cheild_Target_Shift, Cheild_Target_Connect ) ;
			ENEMY_SIDE, GLL_Target_Size, GLL_Target_Shift, Cheild_Target_Connect ) ;
	} else {
		GLL_InitDefenceTarget( work, npc, work->def_child, work->def_power,
			ENEMY_SIDE, Cheild_Target_Size, Cheild_Target_Shift, Cheild_Target_Connect ) ;
	}

#else
	NPC_InitDefenceTarget( work, npc, &work->deftrg, ENEMY_SIDE, SampleTrgSize, &DG_ZeroVector, &DG_ZeroVector ) ;
	NPC_InitDefenceChildTarget( npc, work->def_child,
		ENEMY_SIDE, Cheild_Target_Size, Cheild_Target_Shift, Child_Level_Num, Cheild_Target_Connect ) ;
#endif

#ifdef SMP_TARGET_SKIP
	npc->target.deftrg->class |= TARGET_SKIP ;
#endif

	/* ナビシステム初期化 */
	NPC_InitNavi( npc, &work->control, &work->navigate, &work->navitrg ) ;

	value = GCL_GetOptionValue( 'r', 0 ) ;


	if ( work->status & GLL_STATUS_FIGHT ) {
		VR_AddEnemy()  ;
	} else {
		/* ルート読み込み */
		NPC_InitRouteNavi ( npc, &work->rnavi, value ) ;
	}

	/* 初期位置、方向、モーションのセット */
	if ( ( opt = GCL_GetOption( 'p' ) ) != NULL ){
		ENE_GCL_GetFV( opt, &start_pos ) ;
	} else {
		start_pos = work->rnavi.nodes[0] ;
	}
	if ( work->status & GLL_STATUS_FIGHT ) {
		NPC_InitPose( npc, &start_pos, 0, BASE_MOTION, 0 ) ;
	} else {
		NPC_InitPose( npc, &start_pos, work->rnavi.pa_dir[0], BASE_MOTION, 0 ) ;
	}

	/* 武器 */
	work->gunhand = 0 ;
	value = (work->status & GLL_STATUS_GLL ) ? GLL_WEAPON : GNO_WEAPON;

	GV_SetActorChild( work,NewConnectObject2( &work->cnct, npc->body, HUMAN21_MIGI_TE,
					NULL, NULL, value) ) ; 

	/* レーダー情報初期化 */
	NPC_InitRader( npc, &work->rctrl, &npc->ctrl->mov, 
			2000.0f, -2000.0f, 512, 4000, RADAR_COLOR_RED ) ;

	/* ヘッドマークコントロール初期化 */
	NPC_InitHeadMark( work, npc, &work->headmark, &work->body, &work->control, &work->deftrg ) ;

	/* パラメータセット */
	NPC_SetActionParam( &npc->action, SMP_LIFE, SMP_FAINT, SMP_FAINT_COUNT,
				SMP_SLEEP_COUNT, SMP_DOWN_DAM  ) ;

	/* モーションアジャスト */
	NPC_InitMotionAdjust( npc, &work->npcadjust ) ;

	/* ライフゲージ（各キャラクターが管理する) */
	if ( work->status & GLL_STATUS_FIGHT ) {
		if ( StatusFightGLL ( work->status ) ) {
		//BP - see original drop of this file, it had a timing adjustment for PAL that was removed
		//in the later version of the code.  Might have been a bugfix.
		//Need to check how timestep was handled in MGS2.
		// COUNT_VMODE( SMP_LIFE ) -> SMP_LIFE
		BP_TRIVIAL_BREAK;
			GM_InitGageSet( &work->gage, "Gurlugon", 16, 196, 3, SMP_LIFE,
						    SMP_LIFE, 0, 30, NPC_GAGE_LEVEL ) ;
		} else if ( StatusFightGME ( work->status ) ) {
			GM_InitGageSet( &work->gage, "Mech Genola", 16, 196, 3, SMP_LIFE,
						    SMP_LIFE, 0, 30, NPC_GAGE_LEVEL ) ;
		} else {
			GM_InitGageSet( &work->gage, "Genola", 16, 196, 3, SMP_LIFE,
						    SMP_LIFE, 0, 30, NPC_GAGE_LEVEL ) ;
		}
		work->gage.text_len = 23 ;	/* 見栄えで、えー感じに設定 */
		GM_SetGageColor( &work->gage, 0, 0, 0, 31, 63, 192, 31, 127, 255, 255, 0, 0 ) ;
		GM_AppendGageSet( &work->gage ) ;
		GM_VisibleGage( &work->gage ) ;
	}
	/* ゴルルゴン部位心臓 */
	if ( StatusFightGLL ( work->status ) ) {
		extern void	*NewGllHeart( OBJECT *body, int obj_n, FVECTOR *shift, int life, int *flag, int mode ) ;

		FVECTOR	mune = { 800.0, 1500.0, 200.0 } ;
		FVECTOR	migi_te = { -350.0, -500.0, 150.0 } ;
		FVECTOR	hidari_te = { 350.0, -500.0, 150.0 } ;
		FVECTOR	migi_ashi = { 350.0, -200.0, -100.0 } ;
		FVECTOR	hidari_ashi = { -350.0, -200.0, -100.0 } ;
		
		NewGllHeart( &work->body, 12, &DG_ZeroVector, 100, &work->gllheart[0], 2 ) ;
		NewGllHeart( &work->body, 2, &mune, 100, &work->gllheart[1], 0 ) ;
		NewGllHeart( &work->body, 5, &migi_te, 50, &work->gllheart[2], 1 ) ;
		NewGllHeart( &work->body, 9, &hidari_te, 50, &work->gllheart[3], 1 ) ;
		NewGllHeart( &work->body, 14, &migi_ashi, 50, &work->gllheart[4], 1 ) ;
		NewGllHeart( &work->body, 18, &hidari_ashi, 50, &work->gllheart[5], 1 ) ;
		SET_FLAG( GM_GameStatus,  STATE_GLL );/*手榴弾用*/
		work->eye_length = 28000 ;
	}

	/* メカゲノラサーチライト */
	if ( StatusFightGME ( work->status ) ) {
		extern void *NewGllSearchLight( FMATRIX *mat, float umbra, 
						float penumbra, float range, int *color ) ;
		extern void *NewGllLight( FMATRIX *, int *, int, int * ) ;

		work->spot_color = 0 ;
		work->searchlight = NewGllSearchLight( &(work->search_world),
							( 10.0F / 180.0F * 3.14159 ),( 10.0F / 180.0F * 3.14159 ), 
							SL_RANGE, &work->spot_color ) ;
		work->sw_spot = 1 ;
		NewGllLight( &work->search_world, &work->sw_spot, 1, &work->control.hzx_id ) ;
		NewGllHead(&(work->body.objs->objs[12].world), &work->head_flag, &work->sw_spot, work->lights) ;

		SET_FLAG( GM_GameStatus,  STATE_GLL );/*手榴弾用*/
		NewFootWall( ) ;
		work->eye_length = 28000 ;
	}

	if ( StatusFightGNO ( work->status ) ) {
		extern void	*NewGllTub( OBJECT *body, int map, FMATRIX *lights ) ;
		extern void *NewGllBlurEffect( OBJECT *body, u_long64 mask, int *enable_flag ) ;

		NewGllTub( &work->body, where, work->lights ) ;
		SET_FLAG( GM_GameStatus,  STATE_GNO );/*プレイヤー手アジャスト用*/
		work->head_flag = 0 ;
		NewGllBlurEffect( &work->body, 0x7f8, &work->head_flag ) ;
//		NewGllBlurEffect( &work->body, 0xffff, &work->head_flag ) ;
	}

	{
		extern void *NewGllDropShadow( OBJECT *body, CONTROL *control, FMATRIX *lights, int *flag ) ;
		work->shadow = 1 ;
		NewGllDropShadow( &work->body, &work->control, work->lights, &work->shadow ) ;
		/*ステージ終了時以外終了したらだめ！！*/
		NewGllDropShadow( &work->cnct.body, &work->control, work->lights, &work->shadow ) ;
	}

	/* 思考処理初期化 */
	StartThink( work ) ;

	work->z_limit = work->control.mov.vz + 1000.0 ;

	work->game_count = 0 ;
	work->find_flag  = 0 ;

	GllEyebeemFlag = 0 ;
#ifdef DEBUG_MODE
{
	extern void *NewEyeView4( FMATRIX *world, int len, int range, int upper, int under, int *sw ) ;

	eye_ctrl.target = &debug_flag ;
	body_light.target = &debug_flag ;
	eye_view.target = &debug_flag ;
	think_deb.target = &debug_flag ;

	GM_AddDebugMenu( &eye_ctrl ) ; 	
	GM_AddDebugMenu( &body_light ) ; 	
	GM_AddDebugMenu( &eye_view ) ; 	
	GM_AddDebugMenu( &think_deb ) ; 	

//	NewEyeView4( &(npc->body->objs->objs[12].world), work->eye_length, work->eye_range, 512, -512, &debug_flag ) ;
	NewEyeView4( &work->sight_world, work->eye_length, work->eye_range, UNDER_EYE_SIGHT, UPPER_EYE_SIGHT, &debug_flag ) ;
}
#endif
	return 0 ;
}

	/*
		シナリオ呼び出し
	*/
void		*NewGollgon( name, where )
int		name ;
int		where ;
{
	Work		*work ;

	OPERATOR() ;
//	work = (Work *)GV_NewActor(  GV_ACTOR_USER, sizeof( Work ) ) ;
    work = (Work *)GV_CreateActor( GV_ACTOR_USER, GV_CLASS_CHARA, 
				   sizeof( Work ), 0X30 ) ;
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
