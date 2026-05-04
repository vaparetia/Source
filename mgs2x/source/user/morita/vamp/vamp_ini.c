//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  vamp_ini.c
  ヴァンプ初期化

  2001/03/23 T.Morita
  $Id: vamp_ini.c,v 1.1.1.3 2002/11/19 11:46:35 Yoshizawa1 Exp $
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


/*----------------------------------------------------------------*/

int VMPS_ChildLevelNum[] =
{
    VMPS_TARGET_LVL1      - VMPS_TARGET_LVL0, /* Level 0 のターゲット数 */
    VMPS_TARGET_LVL2      - VMPS_TARGET_LVL1, /* Level 1 のターゲット数 */
    VMPS_TARGET_LVL3      - VMPS_TARGET_LVL2, /* Level 2 のターゲット数 */
    VMPS_TARGET_CHILD_NUM - VMPS_TARGET_LVL3, /* Level 3 のターゲット数 */
} ;

int VMPS_ChildTargetConnect[] =
{
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

	HUMAN21_ONAKA
} ;

FVECTOR	VMPS_ChildTargetShift[] =
{
	/* level 0 は無し */
	/* level 1 */
	{   0.0F,   80.0F,   0.0F },/* 頭 */
	{ 100.0F,   60.0F, 100.0F },/* 心臓 */
	/* level 2 */
	{ -40.0F, -130.0F,   0.0F },/* 右腕１ */
	{ -30.0F, -130.0F,   0.0F },/* 右腕２ */
	{  40.0F, -130.0F,   0.0F },/* 左腕１ */
	{  30.0F, -130.0F,   0.0F },/* 左腕２ */
	{ -10.0F, -200.0F, -10.0F },/* 右足１ */
	{ -20.0F, -250.0F, -10.0F },/* 右足２ */
	{ -10.0F, -200.0F, -10.0F },/* 左足１ */
	{ -20.0F, -250.0F, -10.0F },/* 左足２ */
	/* level 3 */
	{   0.0F,  120.0F,   0.0F },/* 胴体 */
} ;

FVECTOR	VMPS_ChildTargetSize[] =
{
	/* level 0 は無し */
	/* level 1 */
	{ 96.0F, 130.0F, 96.0F },		/* 頭 */
	{ 50.0F, 50.0F, 50.0F },	/* 心臓 */
	/* level 2 */
	{ 75.0F, 150.0F, 75.0F },	/* 右腕１ */
	{ 75.0F, 150.0F, 75.0F },	/* 右腕２ */
	{ 75.0F, 150.0F, 75.0F },	/* 左腕１ */
	{ 75.0F, 150.0F, 75.0F },	/* 左腕２ */
	{ 85.0F, 250.0F, 120.0F },	/* 右足１ */
	{ 85.0F, 250.0F, 100.0F },	/* 右足２ */
	{ 85.0F, 250.0F, 120.0F },	/* 左足１ */
	{ 85.0F, 250.0F, 100.0F },	/* 左足２ */
	/* level 3 */
	{ 200.0F, 300.0F, 150.0F },	/* 胴体 */
} ;

/* ターゲットサイズ */
static FVECTOR VMPS_TrgSize[] =
{
	{ 400.0F, 1000.0F, 400.0F },	/* 立ち */
	{ 1000.0F, 200.0F, 1000.0F },	/* ダウン */
	{ 700.0F, 650.0F, 700.0F },		/* しゃがみ */
} ;

/* 引き摺りシフト */
static FVECTOR VMPS_DragShift[] =
{
	{ 26.483F, -393.755F, 940.002F },	/* 仰向け頭16 */
	{ 0.0F, -113.487F, 291.057F },		/* 仰向け頭 */
	{ 0.0F, -421.119F, 1192.692F },		/* 仰向け足16 */
	{ 0.0F, -382.115F, 422.438F },		/* 仰向け足 */
	{ 0.0F, -346.867F, 1192.629F },		/* うつぶせ足16 */
	{ 0.0F, -294.030F, 463.501F },		/* うつぶせ足 */
	{ 45.761F, -317.266F, 946.003F },	/* うつ伏せ頭16 */
} ;

/*----------------------------------------------------------------*/
static void VMPS_TargetCallBack( TARGET *off, TARGET *def, void *ptr )
{
    if ( def->damaged & TARGET_POWER )
	if ( off->weapon_type & WP_BULLET )
	    NewBlood( &def->world, &def->hit, &off->power->force, 0, 0 ) ;
}

int VMPS_InitTarget( Work *work, int name, int where )
{
    NPCWORK *npc = &work->npc ;
    int i ;

    /* ターゲットの初期化 */
    NPC_InitDefenceTarget( npc, &work->deftrg, ENEMY_SIDE,
			   VMPS_TrgSize, VMPS_DragShift, &DG_ZeroVector ) ;
    NPC_InitDefenceChildTarget( npc, work->def_child, ENEMY_SIDE,
				VMPS_ChildTargetSize,
				VMPS_ChildTargetShift,
				VMPS_ChildLevelNum,
				VMPS_ChildTargetConnect ) ;
    NPC_InitDefenceCapture( npc, &work->capture, &work->control, &work->body ) ;
    work->deftrg.class |= TARGET_LOCKON ;

    /* コールバックを登録する */
    GM_SetTargetCallBack( &work->deftrg, VMPS_TargetCallBack, work ) ;
    for ( i=VMPS_TARGET_CHILD_NUM ; --i>=0 ; )
	GM_SetTargetCallBack( &work->def_child[i], VMPS_TargetCallBack, work ) ;

    VMPS_CaptureEmma( work ) ;

    return 0 ;
}

/* NPCの初期化 */
int VMPS_InitNPC( Work *work, int name, int where )
{
    NPC_InitNPC( &work->npc,
		 work,
		 &work->body,
		 &work->control,
		 work->lights ) ;

    return 0 ;
}

/* NPCのコントロールなどの初期化 */
int VMPS_InitControl( Work *work, int name, int where )
{
    NPCWORK *npc = &work->npc ;
    int kms_id = VMPS_KMS_MODEL_NAME ;
    int evm_id = VMPS_EVM_MODEL_NAME ;

    /* コントロールの初期化 */
    NPC_InitControl( npc, name ) ;
    work->control.seg_flag &= ~HZX_TYPE_ENEMY ;/* 橋の周りのハザードはプレイヤー壁 */
    work->control.seg_flag |=  HZX_TYPE_PLAYER ;
    GM_ConfigControlHazard( &work->control, 1200, 450, 700 ) ;
    GM_ConfigControlHzxHeight( &work->control, 750.0f, -39450.0f ) ;

    if ( GCL_GetOption( 'm' ) )
    {
	kms_id = GCL_GetNextInt() ;
	evm_id = GCL_GetNextInt() ;
    }
    /* オブジェクトの初期化 */
    NPC_InitObject( npc, kms_id ) ;
    /* マルチウェイトモデルの初期化 */
    NPC_InitMWObject( npc, evm_id, 200000 ) ;
    /* モーションの初期化 */
    NPC_InitMotion( npc, VMPS_BASE_MOTION, 0, 0, 0 ) ;    /*default vamp.mar */

    return 0 ;
}

/* ヴァンプの位置,方向などの初期化 */
int VMPS_InitPosition( Work *work, int name, int where )
{
    NPCWORK *npc = &work->npc ;
    FVECTOR pos ;

    /* 初期位置 */
    if ( GCL_GetOption( 'p' ) != NULL )
    {
	pos.vx = (float)GCL_GetNextInt() ;
	pos.vy = (float)GCL_GetNextInt() + 100.0f ;
	pos.vz = (float)GCL_GetNextInt() ;

	if ( HZX_LevelHazardCheck( work->control.hzx_id, &pos,
				   HZX_CHK_ALL, HZX_FLOOR_ALL ) )
	    pos.vy = HZX_GetFloorLevel() ;
	pos.vy -= 100.0f ;
    }    

    /* 初期位置、方向、モーションのNPCに反映 */
    NPC_InitPose( npc,
		  &pos,
		  GCL_GetOptionValue( 'd', 1024 ),
		  VMPS_BASE_MOTION, 0 ) ;
    return 0 ;
}


/* ヴァンプのアニメキャラ起動 */
int VMPS_InitFaceAnime( Work *work, int name, int where )
{
#if 0
    extern void *NewFaceAnimation( int name, DG_EVMOBJ *evmobj, int fanim_id ) ;
    void *anim ;

    if ( work->body.evmobj )
    {
	if ( (anim = NewFaceAnimation( VMPS_FACEANIME_CHARA,
				       work->body.evmobj,
				       GCL_GetOptionValue( 'F', VMPS_FAR_MOTION ) )) )
	    GV_SetActorChild( work, anim ) ;
	else
	    PERROR( "NewFaceAnimation() failed. Something wrong: NewVamp\n" ) ;
    }
#endif
    return 0 ;
}

/* ヴァンプの表示系を初期化 */
int VMPS_InitInfoDisp( Work *work, int name, int where )
{
    NPCWORK *npc = &work->npc ;

    /* ヘッドマーク関係 */
    NPC_InitHeadMark( work, npc, &work->headmark, &work->body, &work->control, &work->deftrg ) ;

    /* レーダー関係 */
    NPC_InitRader( npc, &work->rctrl, &npc->ctrl->mov, 
		   2000.0f, -2000.0f, 512, 4000, RADAR_COLOR_RED ) ;

    /* ゲージ関係 */
    GM_InitGageSet( &work->gage, "VAMP", 16,
		    work->vital_max, 3, work->npc.action.life, 
		    work->vital_max, 0, 30, GM_GAGE_LEVEL_ENEMY ) ;
    GM_SetGageColorType( &work->gage, GM_GAGE_COLOR_TYPE_ENEMY_LIFE ) ;
    //GM_SetGageColor( &work->gage, 0,0,0, 40,128,118, 110,190,118, 255,0,0 ) ;
    GM_InitGageM9( &work->gage, work->vital_max, work->vital_m9, 0, 30 ) ;
    GM_AppendGageSet( &work->gage ) ;
    GM_VisibleGage( &work->gage ) ;

    return 0 ;
}

/* パラメータの初期化 */
int VMPS_InitParams( Work *work, int name, int where )
{
    int  i ;
    int  vitality ;

    /* 思考処理初期化 */
    NPC_SetCheckPad( &work->npc, VMPS_ActCheckPad ) ;
    NPC_SetCheckDamage( &work->npc, VMPS_ActCheckDamage ) ;
    NPC_SetModeFromPad( &work->npc, NPC_ActLoopMotion, work->npc.base_mar, 
			HANG_IDLE, HANG_IDLE ) ;
    work->npc.action.pad = HANG_IDLE ;
    work->tic       = 0 ;
    work->time      = 0 ;
    work->blink_tic = irnd() & 255 ;
    work->aim_pose  = 0 ;
    work->hang_tim  = GCL_GetOptionValue( 'h', 16 ) ;

    /* パラメータセット */
    vitality        = VMPS_MAX_VITALITY ;
    work->vital_max = VMPS_MAX_VITALITY ;
    if ( GCL_GetOption( 'L' ) )
    {
	vitality = GCL_GetNextInt() ;
	if ( GCL_NextStr() )
	    work->vital_max = GCL_GetNextInt() ;
    }
    work->vital_m9 = vitality ;

    work->damage      =  vitality / 2  ;
    work->damage_m9   =  vitality / 10 ;
    work->damage_body    = vitality / 10 ;
    work->damage_m9_body = vitality / 15 ;
    if ( GCL_GetOption( 'A' ) )
    {
	work->damage    =  vitality / GCL_GetNextInt() ;
	if ( GCL_NextStr() )
	    work->damage_m9 = vitality / GCL_GetNextInt() ;
	if ( GCL_NextStr() )
	    work->damage_body = vitality / GCL_GetNextInt() ;
	if ( GCL_NextStr() )
	    work->damage_m9_body = vitality / GCL_GetNextInt() ;
    }

    NPC_SetActionParam( &work->npc.action,
			vitality,
			VMPS_FAINT,
			VMPS_FAINT_COUNT,
			VMPS_SLEEP_COUNT,
			work->damage ) ;

    /* フラグの初期化 */
    work->flag = GCL_GetOptionValue( 'f', VMPS_F_NONE ) ;
    //VMPS_SetFlag( VMPS_F_NON_FACING ) ;

    /* プロックの登録 */
    if ( GCL_GetOption( 'C' ) )
	for ( i=0 ; i<VMPS_P_N_PROC ; i++ )
	    work->proc[i] = GCL_NextStr() ? GCL_GetNextInt() : 0 ;

    return 0 ;
}

/* ヴァンプの表示系を初期化 */
int VMPS_InitDynamicFloor( Work *work, int name, int where )
{
    IVECTOR hzx_pos ;

    _sceVu0FTOI0Vector( &hzx_pos, &work->control.mov ) ;
    work->d_floor = HZX_AddDynamicFloor( work->control.hzx_id,
					 &hzx_pos, &hzx_pos, &hzx_pos, &hzx_pos, 4,
					 HZX_FLOOR_NO_BLOOD| HZX_FLOOR_NO_BULLETHOLE ) ;
    if ( !work->d_floor )
	PERROR( "Cannot make dynamic floor. No Memory or Queue\n" ) ;

    return 0 ;
}

int VMPS_InitHomingTarget( Work *work, int name, int where )
{
    NPC_InitHomingTarget( &work->npc,
				 &work->homing,
				 &work->body,
				 &work->control,
				 &work->deftrg ) ;
    return 0 ;
}
