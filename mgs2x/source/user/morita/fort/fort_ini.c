//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_ini.c 
   フォーチュン 初期化関数群

   1999/12/18 T.Morita
   $Id: fort_ini.c,v 1.1.1.3 2002/11/19 11:46:06 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libutl.h"
#include "gameheader.h"

#include "include/fort.h"


int FRT_InitControl( Work *work, int name, int where )
{
    CONTROL *ctrl = &work->control ;

    GM_InitControl( ctrl, name, where ) ;
    GM_ConfigControlHazard( ctrl, 1000, FRT_NORMAL_SPHERE, FRT_CHECK_SPHERE ) ;
    GM_ConfigControlTrapCheck( ctrl ) ;
    GM_ConfigControlMessageCheck( ctrl ) ;
    GM_ConfigControlObject( ctrl, &work->body ) ;

    ctrl->seg_flag  |= HZX_TYPE_ENEMY             ;
    ctrl->flr_flag  |= HZX_FLOOR_IK               ;
    ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;

    return 0 ;
}


/* オブジェクトを初期化 */
int FRT_InitObject( Work *work )
{
    OBJECT  *body = &work->body ;

    /* モデル本体読み込み */
    if ( !GCL_GetOption( 'm' ) )
	PERROR( "There is no -model option found in GCL : NewFortune\n" ) ;
    GM_InitObject( body,
		   GCL_GetNextInt(), 
		   DG_FLAG_IRREACTION|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE ) ;
    GM_ConfigObjectEvm( body, GCL_GetNextInt(), DG_EVMOBJ_IRREACTION ) ;
    GM_ConfigObjectLight( body, work->lights ) ;
    DG_InvisibleObjs( body->objs ) ;

    /* モーション読み込み */
    GM_ConfigObjectMotion( body, 2, FRT_MOTION_NAME, MT_FLAG_HUMAN2 ) ;
    GM_ConfigObjectStep( body, &work->control.step ) ;
    GM_ConfigObjectAction( body, 0, stll_idle, 0, FRT_BODY_ALL, 0 ) ;

    return 0 ;
}


int FRT_InitParam( Work *work, int name )
{
    /* 初期値 */
    work->name   = name ;

    work->arm_l  = DG_ZeroVector ;
    work->arm_r  = DG_ZeroVector ;
    work->head   = DG_ZeroVector ;
    work->breast = DG_ZeroVector ;

    work->weap_blt     =  FRT_WEAPON_BULLET ;
    work->weap_lnr.vw  =  0.0f ;
    work->weap_amo     = -2    ;

    work->lgt_on = -1 ;

    work->flag = FRT_F_ENB_AIM| FRT_F_UNRECOG_POS| FRT_F_OFFENCE  ;
    work->act_flg   = 0 ;
    work->act_stop  = 0 ;
    work->act_phase = 0 ;

    work->vitality =  FRT_MAX_VITALITY ;
    work->trgt_pos = &work->trgt_frc ;
    work->trgt_eye = FRT_MiscPlayerPos( FRT_GET_PLY_CONTROL ) ;
    work->trgt_aim =  work->trgt_eye ;
    work->trgt_hid = *work->trgt_eye ;
    work->pos_adj.vw = 0.0f ;
    work->trgt_nse.vw = -1.0f ;

    work->head_mark = 0  ;
    work->head_time = 16 ;

    work->voice     = 0 ;
    work->voice_cnt = 0 ;
    work->voice_tim = 0 ;
    work->voice_vox = 0 ;
    work->str_hdl   = 0 ;

    /* プレイヤー情報 */
    work->ply_time    = 0 ;
    work->ply_tic     = 0 ;
    work->ply_hide    = FRT_F_PLYR_HIDEXP ;
    work->ply_grenade = NULL ;
    work->ply_aim_cnt = 0 ;

    /* 初期モーション */
    work->stll_mtn = stll_idle ;
    work->stll_tim = 0 ;
    work->act = FRT_ActionReset ;

    FRT_SmokeClouded = 0 ;
    FRT_FogColor.r = 59 /*DG_FogColor.r*/ ;
    FRT_FogColor.g = 45 /*DG_FogColor.g*/ ;
    FRT_FogColor.b = 20 /*DG_FogColor.b*/ ;
    FRT_FogNear = -2000.0f ;
    FRT_FogFar  =  34000   ;

    FRT_GasFire    = 0 ;
    FRT_GasExplode = 0 ;

#if DEBUG_MODE
    work->control.mov = work->init_pos ;
    work->control.rot = work->init_rot ;
#endif

    return 0 ;
}


/* 初期設定値を取得 */
int FRT_GetOptionValue( Work *work, int name, int where )
{
    int	   buf[3] ;
    int    i ;
    float  level ;

    /* 初期位置 */
    if ( GCL_GetOption( 'p' ) != NULL )
    {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	vu0_IV0toFV( (IVECTOR*)buf, &work->control.mov ) ;
	GM_ConfigControlMapID( &work->control ) ;
    } 

    /* 初期方向 */
    if ( GCL_GetOption( 'd' ) != NULL )
    {
	GCL_GetIV( GCL_NextStr(), buf ) ;
	work->control.rot.vx = work->control.turn.vx = buf[X] ;
	work->control.rot.vy = work->control.turn.vy = buf[Y] ;
	work->control.rot.vz = work->control.turn.vz = buf[Z] ;
    }    

    /* 初期床高さ */
    if ( HZX_LevelHazardCheck( work->control.hzx_id, &work->control.mov, HZX_CHK_ALL, HZX_FLOOR_ALL ) )
	level = HZX_GetFloorLevel() ;
    else
	level = 0.0F ;
    GM_ConfigControlHzxHeight( &work->control, 750.0F, level ) ;

    /* プロックの登録 */
    if ( !GCL_GetOption( 'C' ) )
	PERROR( "There is no -proc option found in GCL : NewFortune\n" ) ;
    for( i=0 ; i<FRT_N_PROCS ; i++ )
	work->procs[i] = GCL_NextStr() ? GCL_GetNextInt() : 0 ;

    /* ストリームの登録 */
    if ( !GCL_GetOption( 'v' ) )
	PERROR( "There is no -voice option found in GCL : NewFortune\n" ) ;
    for( i=0 ; i<FRT_N_STREAM ; i++ )
	work->str_id[i] = GCL_NextStr() ? GCL_GetNextInt() : 0 ;

#if DEBUG_MODE
    work->init_pos = work->control.mov ;
    work->init_rot = work->control.rot ;
#endif

    return 0 ;
}


void FRT_InitTarget( Work *work, int where )
{
    static FVECTOR size = { 800.0f, 800.0f, 800.0f, 0.0f } ; /* parent target */

    /* 弾があたったかどうかの判定用  かなり大きめ */
    GM_SetTarget( &work->target, FRT_MAIN_TRG_FLG, where, ENEMY_SIDE, &size, &DG_ZeroVector ) ;
    GM_SetPowerTarget( &work->target, &work->power, POWER_DECREASE,
		       GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( &work->target, FRT_MainTargetCallBack, work ) ;
    //NewTargetView( &work->target, 34, 184, 200 ) ;
    GM_PutTarget( &work->target ) ;

    /* 弾が何に当たるか検知するためのターゲット */
    GM_SetTarget( &work->seeker, FRT_HIT_TRG_FLG, 0, PLAYER_SIDE, &DG_ZeroVector, &DG_ZeroVector ) ;
    GM_SetTargetWeaponType( &work->seeker, WP_LASERSIGHT ) ;
    GM_SetTargetCallBack( &work->seeker, FRT_HitTargetCallBack, work ) ;
}


void FRT_InitHomingTarget( Work *work )
{
    /* ホーミングターゲットの設定(これでスネークがホーミング出来る) */
    GM_SetHomingTrg( &work->homing,
		     &BODYWORLD(&work->body, HUMAN21_KUBI),
		     &work->body,
		     &work->control.hzx_id, NULL, 0 ) ;
    GM_PutHomingTrg( &work->homing ) ;
}


void FRT_InitVitalityGage( Work *work )
{
    GM_InitGageSet( &work->gage, "FORTUNE", 16, FRT_MAX_VITALITY, 3, work->vitality, 
		    FRT_MAX_VITALITY, 0, 30, 1 ) ;
    GM_SetGageColor( &work->gage, 0,0,0, 40,128,118, 110,190,118, 255,0,0 ) ;
    GM_InitGageM9( &work->gage, FRT_MAX_VITALITY, work->vitality, 0, 30 ) ;
    GM_AppendGageSet( &work->gage ) ;
    GM_VisibleGage( &work->gage ) ;
}


/* レーダー初期化 */
void FRT_InitRadar( Work *work )
{
    GM_InitRadarControl( &work->radar, &work->control.mov,
			 RADAR_VISIBLE|RADAR_SIGHT|RADAR_NOFIX_SIGHT,
			 work->control.map ) ;
    GM_RadarSetVRange( &work->radar, 2*RADAR_VRANGE_UPPER, 2*RADAR_VRANGE_LOWER ) ;
    GM_RadarSetSight( &work->radar, work->control.rot.vy, 512, 8128.0f, RADAR_COLOR_BLUE ) ;
}

/* メッセージ処理 */
void FRT_InitMessages( Work *work )
{
}

void FRT_LaunchExternalChara( Work *work, int name, int where )
{
    int flag ;

    /* 開始時フラグによる設定 */
    flag = GCL_GetOptionValue( 'f', 0 ) ;
    if ( !(flag & FRT_START_NOFOOTSHADOW) || 1 )
    {
	extern void *NewDropShadow( OBJECT *body, CONTROL *control, FMATRIX *lights, int *flag ) ;
	/* 影起動 */
	NewShadow( &work->body.objs->objs[16], &work->body.objs->objs[20],   
		   &work->control, work->lights, NULL ) ;
    }
    if ( !(flag & FRT_START_NOBODYSHADOW) )
    {
	//NewBodyShadow( work->body.objs ) ;
    }
    if (  (flag & FRT_START_BODYSPECULAR) )
    {
    }
    if (  (flag & FRT_START_HAIRMOVE) )
    {
    }

    /* 武器 */
    NewFortWeaponLNRAMO( &BODYWORLD( &work->body, HUMAN21_MIGI_TE   ),
			 &BODYWORLD( &work->body, HUMAN21_HIDARI_TE ),
			 &work->weap_amo ) ;
    NewFortWeaponLNR( where, work->body.objs,
		      &work->weap_lnr, &work->weap_blt, 40 ) ;
    NewControl_Headmark2( &BODYWORLD( &work->body, HUMAN21_ATAMA ),
			  &work->head_mark, NULL, &work->control ) ;

    NewFortEquip( GV_StrCode( "装備品フォーチュン" ), &work->body, 0xffff, 0/*武器はなし*/ ) ;

#if DEBUG_MODE
    {
	extern void *NewFortDebugDump( int name, int where ) ;
	NewFortDebugDump( GV_StrCode( "debug_mode_for_fort" ), GM_CurrentMap ) ;

	FRT_InitDbgConfiguration() ;
    }
#endif
}
