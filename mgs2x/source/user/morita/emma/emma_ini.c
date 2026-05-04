//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  emma_ini.c
  エマ初期化

  2001/02/09 Y.Korekado
  2001/02/14 T.Morita Revised
  $Id: emma_ini.c,v 1.1.1.3 2002/11/19 11:46:00 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"include/emma.h"

#include "BP_EndianSupport.h"

/*----------------------------------------------------------------*/

static int	Child_Level_Num[] = { 
    EMA_TARGET_LVL1      - EMA_TARGET_LVL0, /* Level 0 のターゲット数 */
    EMA_TARGET_LVL2      - EMA_TARGET_LVL1, /* Level 1 のターゲット数 */
    EMA_TARGET_LVL3      - EMA_TARGET_LVL2, /* Level 2 のターゲット数 */
    EMA_TARGET_CHILD_NUM - EMA_TARGET_LVL3, /* Level 3 のターゲット数 */
} ;

static int	Child_Target_Connect[] = { 
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

static FVECTOR	Child_Target_Shift[] = { 
	/* level 0 は無し */
	/* level 1 */
	{   0.0F, 50.0F,    10.0F },/* 頭 */
	{  60.0F, 80.0F,   100.0F },/* 心臓 */
	/* level 2 */
	{ -20.0F, -100.0F,   0.0F },/* 右腕１ */
	{ -10.0F,  -80.0F,  20.0F },/* 右腕２ */
	{  20.0F, -100.0F,   0.0F },/* 左腕１ */
	{  10.0F,  -80.0F,  20.0F },/* 左腕２ */
	{   5.0F, -180.0F, -10.0F },/* 右足１ */
	{  10.0F, -220.0F, -20.0F },/* 右足２ */
	{  -5.0F, -180.0F, -10.0F },/* 左足１ */
	{ -10.0F, -220.0F, -20.0F },/* 左足２ */
	/* level 3 */
	{   0.0F,   40.0F,  35.0F },		/* 胴体 */
} ;

static FVECTOR	Child_Target_Size[] = { 
	/* level 0 は無し */
	/* level 1 */
	{ 60.0F, 90.0F, 60.0F },       /* 頭 */
	{ 50.0F, 50.0F, 50.0F },	/* 心臓 */
	/* level 2 */
	{ 35.0F, 130.0F, 35.0F },	/* 右腕１ */
	{ 25.0F, 100.0F, 25.0F },	/* 右腕２ */
	{ 35.0F, 130.0F, 35.0F },	/* 左腕１ */
	{ 25.0F, 100.0F, 25.0F },	/* 左腕２ */
	{ 55.0F, 230.0F, 60.0F },	/* 右足１ */
	{ 25.0F, 250.0F, 40.0F },	/* 右足２ */
	{ 55.0F, 230.0F, 60.0F },	/* 左足１ */
	{ 25.0F, 250.0F, 40.0F },	/* 左足２ */
	/* level 3 */
	{ 120.0F, 180.0F, 45.0F },	/* 胴体 */
} ;

/* ターゲットサイズ */
static FVECTOR SampleTrgSize[] = {
	{ 500.0F, 650.0F, 500.0F },	/* 立ち */
	{ 1000.0F, 200.0F, 1000.0F },	/* ダウン */
	{ 650.0F, 500.0F, 650.0F },	/* しゃがみ */
	{ 250.0F, 400.0F, 250.0F },	/* 俯瞰 */
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

/*----------------------------------------------------------------*/

FMATRIX *EMA_GetConnectObjMatrix( Work *work, TARGET *def )
{
    int i ;

    for ( i=0 ; i<EMA_TARGET_CHILD_NUM ; i++ )
	if ( &work->def_child[i] == def )
	    return &BODYWORLD(&work->body, Child_Target_Connect[i] ) ;
    return NULL ;
}

int EMA_InitTarget( Work *work, int name, int where )
{
    NPCWORK *npc = &work->npc ;
    int i ;
    static FVECTOR HangShiftSnake = { 34.0F,  -65.0F, 263.0F } ;
    static FVECTOR HangShiftVamp  = { 34.0F, -265.0F, 313.0F } ;

    /* ターゲットの初期化 */
    NPC_InitDefenceTarget( npc, &work->deftrg, BOTH_SIDE,
			   SampleTrgSize, SampleDragShift, &DG_ZeroVector ) ;
    NPC_InitDefenceChildTarget( npc, work->def_child, BOTH_SIDE,
				Child_Target_Size,
				Child_Target_Shift,
				Child_Level_Num,
				Child_Target_Connect ) ;
    work->deftrg.class |= TARGET_LOCKON ;

    NPC_InitDefenceCapture( npc, &work->capture, &work->control, &work->body ) ;
    if ( EMA_Flag( EMA_F_EVENT_SNIPE ) )
	NPC_SetHangShift( npc, &HangShiftVamp  ) ;
    else
	NPC_SetHangShift( npc, &HangShiftSnake ) ;

    /* コールバックを登録する */
    GM_SetTargetCallBack( &work->deftrg, EMA_TargetCallBack, work ) ;
    for ( i=EMA_TARGET_CHILD_NUM ; --i>=0 ; )
	GM_SetTargetCallBack( &work->def_child[i], EMA_TargetCallBack, work ) ;

    return 0 ;
}

/* NPCの初期化 */
int EMA_InitNPC( Work *work, int name, int where )
{
    NPC_InitNPC( &work->npc,
		 work,
		 &work->body,
		 &work->control,
		 work->lights ) ;

    return 0 ;
}

/* NPCのコントロールなどの初期化 */
int EMA_InitControl( Work *work, int name, int where )
{
    NPCWORK *npc = &work->npc ;

    /* コントロールの初期化 */
    NPC_InitControl( npc, name ) ;
    if ( EMA_Flag(EMA_F_EVENT_SNIPE) ) {
	work->control.seg_flag &= ~HZX_TYPE_ENEMY  ;
	work->control.seg_flag |=  HZX_TYPE_PLAYER ;
    } else {
		work->control.seg_flag |= HZX_TYPE_ENEMY  ;
		if ( GM_GameStatus & STATE_VR_ANOTHER ) {
			work->control.seg_flag |= HZX_SEG_CYPHER ;
		}
	}
//    work->control.flr_flag |=  HZX_FLOOR_IK    ;
    GM_ConfigControlHazard( &work->control, 1200, 450, 1500 ) ;

    work->height = 0.0f ;
    work->interp = ACT_INTERP_DEF ;

    if ( GCL_GetOption( 'm' ) )
    {
	//kms_id = GCL_GetNextInt() ;
	//evm_id = GCL_GetNextInt() ;
    }
    /* オブジェクトの初期化 */
    NPC_InitObject( npc, EMA_KMS_MODEL_NAME ) ;
    /* マルチウェイトモデルの初期化 */
    NPC_InitMWObject( npc, EMA_EVM_MODEL_NAME, 4000 ) ;
    /* モーションの初期化 */
    NPC_InitMotion( npc,
		    EMA_BASE,    /*default emma.mar    */
		    EMA_DAMG,  /*default emadamg.mar */
		    EMA_CAPT, /*default emacap.mar  */
		    EMA_DRAG ) ; /*default emadrag.mar */

    /* ナビシステム初期化 */
    NPC_InitNavi( npc, &work->control, &work->navigate, &work->navitrg ) ;

    return 0 ;
}

/* エマの位置,方向などの初期化 */
int EMA_InitPosition( Work *work, int name, int where )
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
				   HZX_CHK_ALL, HZX_FLOOR_IK ) )
	    pos.vy = HZX_GetFloorLevel() ;
	pos.vy -= 100.0f ;
    }    

    /* 初期位置、方向、モーションのNPCに反映 */
    NPC_InitPose( npc,
		  &pos,
		  GCL_GetOptionValue( 'd', 1024 ),
		  EMA_BASE, 0 ) ;

    return 0 ;
}


/* エマのアニメキャラ起動 */
int EMA_InitFaceAnime( Work *work, int name, int where )
{
    int   far_leng  ;

    if ( !(work->flag & EMA_F_EVENT_SNIPE) )
	if ( work->body.evmobj )
	{
	    far_leng = GCL_GetOptionValue( 'R', EMA_FAR ) ;
	    if ( GV_GetCache( GV_CacheID( far_leng, 'f' ) ) )
	    {
#if 0
		extern void *NewFaceAnimation( int name, DG_EVMOBJ *evmobj,
					       int fanim_id );
		void *anim ;

		if ( (anim = NewFaceAnimation( EMA_FACEANIME_CHARA,
					       work->body.evmobj, far_leng )) )
		    GV_SetActorChild( work, anim ) ;
		else
		    PERROR( "NewFaceAnimation() failed : NewEmma\n" ) ;
#else
		work->face_h = EMA_InitFaceAnimation( work->body.evmobj,
						      far_leng,
						      5584254/*エマ*/ ) ;
#endif
	    }
	}
    return 0 ;
}

/* エマの表示系を初期化 */
int EMA_InitInfoDisp( Work *work, int name, int where )
{
    NPCWORK *npc = &work->npc ;

    /* ヘッドマーク関係 */
    NPC_InitHeadMark( work, npc, &work->headmark, &work->body, &work->control, &work->deftrg ) ;

    /* レーダー関係 */
#if 0
    NPC_InitRader( npc, &work->rctrl, &npc->ctrl->mov, 
		   2000.0f, -2000.0f, 512, 4000, RADAR_COLOR_BLUE ) ;
#else
    GM_InitRadarControl( &work->rctrl, &npc->ctrl->mov, RADAR_VISIBLE, -1 ) ;
    GM_RadarSetVRange( &work->rctrl,2000.0f, -2000.0f );
#endif

    /* ゲージ関係 */
    GM_InitGageSet( &work->gage, "EMMA", 16,
		    work->vital_max, 3, work->npc.action.life, 
		    work->vital_max, 0, 30, GM_GAGE_LEVEL_NPC_LIFE ) ;
    GM_SetGageColorType( &work->gage, GM_GAGE_COLOR_TYPE_NPC_LIFE ) ;
    GM_AppendGageSet( &work->gage ) ;
    GM_VisibleGage( &work->gage ) ;

    return 0 ;
}

/* ルートの初期化 */
int EMA_InitRoute( Work *work, int name, int where )
{
    if ( GCL_GetOption( 'r' ) )
    {
	EMA_SetRoutePatrol( work, GCL_GetNextInt() ) ;
	EMA_SetRoutePoint( work, GCL_GetNextInt() ) ;
    }
    return 0 ;
}

int EMA_InitFlags( Work *work, int name, int where )
{
    /* フラグの初期化 */
    work->flag = GCL_GetOptionValue( 'f', EMA_F_NONE ) ;
    EMA_SetFlag( EMA_F_ENB_STUNGRE|EMA_F_ENB_VIBRATE|
		 EMA_F_ENB_WANDER|EMA_F_ENB_LINK|EMA_F_ENB_RECOVER ) ;
    if ( work->flag & EMA_F_EVENT_SNIPE )
	EMA_SetFlag( EMA_F_NON_FACING ) ;

    return 0  ;
}

/* パラメータの初期化 */
int EMA_InitParams( Work *work, int name, int where )
{
    int  i ;
    int  vitality ;

    /* IK初期化 */
    if ( !(work->flag & EMA_F_EVENT_SNIPE) )
	if ( !(work->arm_ik = EMA_MakePuppetIK( &work->control, &work->body )) )
	    PERROR( "EMA_MakePuppetIK() failed. There no memory : NewEmma\n" ) ;

    /* 思考・行動処理初期化 */
    if ( work->flag & EMA_F_EVENT_SNIPE )
	work->npc.action.pad = SNIPE_RUN ;
    else
	work->npc.action.pad = PAL_IDLE ;
    NPC_SetCheckPad( &work->npc, EMA_ActCheckPad ) ;
    NPC_SetCheckDamage( &work->npc, EMA_ActCheckDamage ) ;
    NPC_SetModeFromPad( &work->npc, NPC_ActLoopMotion, work->npc.base_mar,
			work->npc.action.pad, work->npc.action.pad ) ;
    work->time      = 0 ;
    work->walk_cnt  = 0 ;
    work->stare_cnt = 0 ;
    work->blink_tic = irnd() & 255 ;
    work->fake_cnt  = 50 ;
    work->voice_str = -1 ;

    /* ステージ初期化 */
    if ( GCL_GetOption( 'S' ) )
	work->stage = EMA_STG_W31B ;
    else
	work->stage = GCL_GetOptionValue( 'T', -1 ) ;

    /* 壁あたり用(think/.hで使用) */
    work->wall_rot = 0.82f ;

    /* ライデンがエマをじっと見るためのもの(emma_utl.cで使用) */
    work->ply_stare_cnt  = -EMA_NONSTARE_CNT ;
    work->ply_stare_rotx = 0 ;
    work->ply_stare_roty = 0 ;

    /* パラメータセット */
    vitality        = EMA_MAX_VITALITY ;
    work->vital_max = EMA_MAX_VITALITY ;
    if ( GCL_GetOption( 'L' ) )
	work->vital_max = GCL_GetNextInt(), vitality = GCL_GetNextInt() ;
    if ( vitality == 0 )
	vitality = 1 ;

    work->damage_gun  = EMA_DOWN_DAM   ;
    work->damage_bomb = EMA_DOWN_DAM*5 ;
    if ( GCL_GetOption( 'A' ) )
	work->damage_gun  = GCL_GetNextInt(), work->damage_bomb = GCL_GetNextInt() ;
    NPC_SetActionParam( &work->npc.action,
			vitality,
			EMA_FAINT,
			EMA_FAINT_COUNT,
			EMA_SLEEP_COUNT,
			work->damage_gun ) ;

    /* 歩くスピードを設定する(狙撃イベント専用) */
    work->walk_speed = (float)GCL_GetOptionValue( 's', 100 ) / 100.0f ;

    /* 隠れ場所の登録 */
    work->n_hide = 0 ;
    if ( GCL_GetOption( 'h' ) )
	while( GCL_NextStr() && work->n_hide<EMA_MAX_HIDE )
	{
	    work->hide[work->n_hide].vx = (float)GCL_GetNextInt() ;
	    work->hide[work->n_hide].vy = (float)GCL_GetNextInt() ;
	    work->hide[work->n_hide].vz = (float)GCL_GetNextInt() ;
	    HZX_Pos2Zone( &work->hide[work->n_hide],
			  &work->hide_g[work->n_hide],
			  &work->hide_a[work->n_hide] ) ;
	    work->n_hide++ ;
	}

    /* プロックの登録 */
    if ( GCL_GetOption( 'C' ) )
	for ( i=0 ; i<EMA_P_N_PROC ; i++ )
	    work->proc[i] = GCL_NextStr() ? GCL_GetNextInt() : 0 ;


    return 0 ;
}

/* 音声初期化 */
int EMA_InitVoice( Work *work, int name, int where )
{
    int i;

    if ( GCL_GetOption( 'v' ) )
    {
       work->voice_le =  GCL_GetNextResource_LE() ;
       for ( i = 0; i < EMA_V_N_VOICE; ++i )
       {
          BP_LE_Swap4Bytes( work->voice + i, work->voice_le + i );
       }
    }
    return 0 ;
}

/* エマ 装備品初期化 */
int EMA_InitEquipment( Work *work, int name, int where )
{
    extern void *NewEmmaEquip( int name, OBJECT *body, int flag ) ;
    int flag ;

    if ( work->flag & EMA_F_EVENT_SNIPE )
	flag = 0x0006 ;/* 乾いた髪の毛 と 眼鏡だけ */
    else
	flag = 0x000f ;

    if ( !GCL_GetOptionValue( 'D', 1 ) )/* IDカードの有無 */
	flag &= ~0x0001 ;
    if ( GCL_GetOption( 'I' ) )/* IDカード無し */
	flag &= ~0x0001 ;
    if ( GCL_GetOption( 'G' ) )/* めがね無し   */
	flag &= ~0x0002 ;
    if ( !NewEmmaEquip( 11734585 /*NewEmmaEquip*/, &work->body, flag ) )
	return -1 ;

    /* 映り込み用の髪&眼鏡 */
    if ( !(work->flag & EMA_F_EVENT_SNIPE) )
    {
	work->hair_obj = EMA_InitMirrorObj( work->body.objs,
					    5608863/*ema_hair_for_sngle*/,
					    HUMAN21_ATAMA ) ;
	work->glass_obj = EMA_InitMirrorObj( work->body.objs,
					     11219402/*ema_glasses_mt*/,
					     HUMAN21_ATAMA ) ;
    }

    EMA_SendMessageInvisbleWetHair( work ) ;/* 濡れ髪の毛は消しておく */
    
    return 0 ;
}

HOMING_TRG * Emma_Trg ;
/* エマ ホーミングターゲット */
int EMA_InitHoming( Work *work, int name, int where )
{
    Emma_Trg = &work->homing ;
    GM_SetHomingTrg( &work->homing, &BODYWORLD(&work->body, HUMAN21_KUBI),
		     &work->body, &work->control.hzx_id, NULL, 0 ) ;
    GM_PutHomingTrg( &work->homing ) ;
    work->homing.status |= HOMING_SKIP ;
    return 0 ;
}



int EMA_EffectStart( Work *work, int where )
{
    if ( work->flag & EMA_F_EVENT_SNIPE && 0 )
	NewFootSplash( &work->body, &work->control ) ;

    work->shadow_flg = 1 ;
    /* 影起動 */
    NewShadow( &work->body.objs->objs[16], &work->body.objs->objs[20],   
	       &work->control, work->lights, &work->shadow_flg ) ;

    if ( !(work->flag & EMA_F_EVENT_SNIPE) )
	NewFootPrint( &work->body, &work->control, &work->foot_flag, 3000 ) ;
    return 0 ;
}

