//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   orga_ini.c 
   オルガ 初期化関数群

   1999/12/18 T.Morita
   $Id: orga_ini.c,v 1.1.1.3 2002/11/19 11:46:20 Yoshizawa1 Exp $
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

#include "include/orga.h"


int ORG_InitControl( Work *work, int name, int where )
{
    CONTROL *ctrl = &work->control ;

    GM_InitControl( ctrl, name, where ) ;
    GM_ConfigControlHazard( ctrl, 1000, ORGA_NORMAL_SPHERE, ORGA_CHECK_SPHERE ) ;
    GM_ConfigControlTrapCheck( ctrl ) ;
    GM_ConfigControlMessageCheck( ctrl ) ;
    GM_ConfigControlObject( ctrl, &work->body ) ;

    ctrl->seg_flag  |= HZX_TYPE_ENEMY             ;
    ctrl->flr_flag  |= HZX_FLOOR_IK               ;
    ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;

    return 0 ;
}

/* オブジェクトを初期化 */
int ORG_InitObject( Work *work )
{
    OBJECT  *body = &work->body ;
    int	     id     ;

    /* モデル本体読み込み */
    id = GCL_GetOptionValue( 'm', ORGA_MODEL_NAME ) ;
    GM_InitObject( body,
				  id, 
				  DG_FLAG_IRREACTION|DG_FLAG_SHADE|DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE ) ;
    id = GCL_GetOptionValue( 'e', ORGA_EVMMODEL_NAME ) ;
    GM_ConfigObjectEvm( body, id, DG_EVMOBJ_IRREACTION ) ;
    GM_ConfigObjectLight( body, work->lights ) ;
    DG_InvisibleObjs( body->objs ) ;

    /* 胸揺れ用の初期化   手関節つきは,こっちInitEvmMMOrgaWithHand */
    if ( !(work->misc_breast = InitEvmMMOrga( body->evmobj )) )
	return -1 ;

    /* モーション読み込み */
    id = GCL_GetOptionValue( 'a', ORGA_MOTION_NAME ) ;
    GM_ConfigObjectMotion( body, 2, id, MT_FLAG_HUMAN2 ) ;
    GM_ConfigObjectStep( body, &work->control.step ) ;
    GM_ConfigObjectAction( body, 0, stll_idle, 0, ORGA_BODY_ALL, 0 ) ;

    return 0 ;
}

int ORG_InitParam( Work *work, int name )
{
    ORG_Work = work ;

    /* 初期値 */
    work->name   = name ;

    work->arm_l  = DG_ZeroVector ;
    work->arm_r  = DG_ZeroVector ;
    work->head   = DG_ZeroVector ;
    work->breast = DG_ZeroVector ;

    work->act_speed   =  ORGA_NORMAL_SPEED  ;
    work->weap_blt    =  work->weap_max ;
    work->weap_usp.vw =  0.0f ;
    work->weap_sgr.vw = -2.0f ;
    work->weap_amo    = -2    ;

    work->speed       = 1.0f  ;

    work->flag = ORGA_F_ENB_AIM| ORGA_F_ENB_AVOID ;
    work->avoid_flg = 0 ;
    work->aim_flg   = 0 ;
    work->act_flg   = 0 ;
    work->act_stop  = 0 ;

    work->hide_spot    = ORG_HideBranch ;
    work->hide_pool    = ORG_HideBranch ;
    work->n_hide_pool  = ORGA_N_HIDE_DFLT ;//ORGA_N_HIDE_POOL ;
    work->hide_hist[0] = work->hide_spot ;
    work->hide_hist[1] = work->hide_spot ;
    work->vitality    =  ORGA_MAX_VITALITY ;
    work->vitality_m9 =  ORGA_MAX_VITALITY ;
    work->trgt_pos = NULL ;
    work->trgt_eye = ORG_RecogPlayerPos( ORGA_GET_PLY_CONTROL ) ;
    work->trgt_aim =  work->trgt_eye ;
    work->trgt_hid = *work->trgt_eye ;
    work->pos_adj.vw = 0.0f ;
    work->non_dmg  = ORGA_DFLT_NONDMG_TIME ;
    work->trgt_nse.vw = -1.0f ;

    work->head_mark = 0  ;
    work->head_time = 16 ;

    work->voice     = 0 ;
    work->voice_tim = 0 ;
    work->voice_vox = 0 ;
    work->str_hdl   = 0 ;

    _sceVu0CopyVector( &ORG_HoloHide.center, &work->hide_pool[2].center ) ;
    _sceVu0CopyVector( &ORG_HoloHide.size  , &work->hide_pool[2].size   ) ;
    ORG_HoloHide.center.vz += ORG_HoloHide.size.vz ;
    ORG_HoloHide.size.vz *= 2.0f ;

    /* プレイヤー情報 */
    work->ply_time = 0 ;
    work->ply_hide = ORGA_F_PLYR_HIDEXP ;

    /* 初期モーション */
    work->stll_mtn = stll_idle ;
    work->stll_tim = 0 ;
    work->act = ORG_ActionReset ;

#if DEBUG_MODE
    work->control.mov = work->init_pos ;
    work->control.rot = work->init_rot ;

#if 0
    work->vitality =  ORGA_MAX_VITALITY/2 ;
    work->voice_vox = (ORGA_N_STREAM<<12) ;/* 第2フェーズから始まる */
    work->flag |= ORGA_F_DMG_HOROFIRE ;
    //work->flag |= ORGA_F_DMG_HOROATTK ;
#endif

#endif

    return 0 ;
}

/* 初期設定値を取得 */
int ORG_GetOptionValue( Work *work, int name )
{
    int	   buf[3], flag ;
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

    /* 開始時フラグによる設定 */
    flag = GCL_GetOptionValue( 'f', 0 ) ;
    if ( !(flag & ORGA_START_NOBODYSPLASH) )
    {
	static int body_sph = 1 ;
	/* 飛沫 */
	NewBodySplash2( work->body.objs, &work->control, GV_StrCode( "org_sgl" ), &body_sph ) ;
	NewFootSplash( &work->body, &work->control ) ;
    }
    if ( !(flag & ORGA_START_NOFOOTSHADOW) || 1 )
    {
	extern void *NewDropShadow( OBJECT *body, CONTROL *control, FMATRIX *lights, int *flag ) ;
	/* 影起動 */
	NewShadow( &work->body.objs->objs[16], &work->body.objs->objs[20],   
		   &work->control, work->lights, NULL ) ;
	//NewDropShadow( &work->body, &work->control, work->lights, NULL ) ;
    }
    if ( !(flag & ORGA_START_NOBODYSHADOW) )
    {
	//NewBodyShadow( work->body.objs ) ;
    }
    if ( (flag & ORGA_START_MERYLE) ) {
	extern void *NewMeryleEquip( int name, CONTROL *ctrl, int flag ) ;
	NewMeryleEquip( 61864634, &work->control, 0x0007 ) ;
    }
    if ( (flag & ORGA_START_HAIRMOVE) )
    {
	NewOrgaHair( &BODYWORLD(&work->body, HUMAN21_ATAMA),
		     GV_StrCode("org_hair_front"),
		     work->lights ) ;
	NewOrgaHair( &BODYWORLD(&work->body, HUMAN21_ATAMA),
		     GV_StrCode("org_hair_back") ,
		     work->lights ) ;
    }

    /* プロックの登録 */
    if ( !GCL_GetOption( 'h' ) )
	PERROR( "There is no -h option found in GCL : NewOrga\n" ) ;
    for( i=0 ; i<ORGA_N_PROCS ; i++ )
	work->procs[i] = GCL_NextStr() ? GCL_GetNextInt() : 0 ;

    /* ストリームの登録 */
    if ( GCL_GetOption( 'v' ) ){
	for( i=0 ; i<ORGA_N_STREAM ; i++ )
	    work->str_id[i] = GCL_NextStr() ? GCL_GetNextInt() : 0 ;
    } else {
	for( i=0 ; i<ORGA_N_STREAM ; i++ )
	    work->str_id[i] = 0 ;
    }
    /* ダメージの登録 */
    if ( !GCL_GetOption( 'c' ) )
	PERROR( "There is no -c option found in GCL : NewOrga\n" ) ;


#define USP_WEAP 123600  /*GV_StrCode( "usp" )    */
#define DSE_WEAP 106181  /*GV_StrCode( "dse" )    */
#define USP_AMMO 3223612 /*GV_StrCode( "usp_amo" )*/
#define DSE_AMMO 8465403 /*GV_StrCode( "dse_amo" )*/

    /* 武器 */
	{
		int  param1, param2, param3 ;
		NewOrgaWeaponUSPAMO( &BODYWORLD( &work->body, HUMAN21_MIGI_TE ),
							&BODYWORLD( &work->body, HUMAN21_HIDARI_TE ),
							&work->weap_amo,
						 (flag & ORGA_START_MERYLE ? DSE_AMMO : USP_AMMO) ) ;
		NewOrgaWeaponUSP( &BODYWORLD( &work->body, HUMAN21_KOSHI     ),
						  &BODYWORLD( &work->body, HUMAN21_MIGI_TE   ),
						  &BODYWORLD( &work->body, HUMAN21_HIDARI_TE ),
						  &work->weap_usp, &work->weap_blt, GCL_GetNextInt(),
						 (flag & ORGA_START_MERYLE ? DSE_WEAP : USP_WEAP) ) ;

		param1 = GCL_GetNextInt() ;
		param2 = GCL_GetNextInt() ;
		param3 = GCL_GetNextInt() ;
		NewOrgaWeaponSGR( &BODYWORLD( &work->body, HUMAN21_MIGI_TE ),
						 &work->weap_sgr, param1, param2, param3 ) ;

		/* 弾数の変更 */
		work->weap_max = (flag & ORGA_START_MERYLE ? ORGA_DSE_BULLET : ORGA_WEAPON_BULLET) ;
		work->weap_blt = work->weap_max ;
	}

	/* オルガのヘッドマーク管理 */
    NewControl_Headmark2( &BODYWORLD( &work->body, HUMAN21_ATAMA ),
			  &work->head_mark, NULL, &work->control ) ;


#if DEBUG_MODE
#if 1
    {
	extern void *NewOrgaDebugDump( int name, int where ) ;
	NewOrgaDebugDump( GV_StrCode( "debug_mode_for_orga" ), GM_CurrentMap ) ;
    }
#endif

    work->init_pos = work->control.mov ;
    work->init_rot = work->control.rot ;
#endif

    return 0 ;
}


int ORG_InitPartAndTarget( Work *work, int map )
{
    int           i, j ;
    TARGET	 *t  = work->target ;
    POWER_TARGET *p  = work->power  ;
    PART         *pt = work->parts  ;
    FVECTOR      *s ;
    OBJECT       *o ;
    static FVECTOR size_and_offset[] = {
	{ 800.0f, 800.0f, 800.0f	}, /* parent target */
	{  96.0f, 130.0f,  96.0f, HUMAN21_ATAMA       },
	{  0.0f,  30.0f,  0.0f,(32<<17)        |damg_head },/*lvl0 head */
	{  50.0f,  65.0f, 165.0f, HUMAN21_MUNE        },
	{100.0f, 100.0f,  0.0f,(32<<17)        |damg_body },/*lvl0 heart */

	{  45.0f, 150.0f,  65.0f, HUMAN21_MIGI_UDE1   },
	{  0.0f,-130.0f,  0.0f, (8<<17)|(1<<16)|damg_arm_l },/*lvl2 armR1*/
	{  45.0f, 150.0f,  65.0f, HUMAN21_MIGI_UDE2   },
	{  0.0f,-130.0f,  0.0f, (8<<17)|(1<<16)|damg_arm_l },/*lvl2 armR2*/
	{  45.0f, 150.0f,  65.0f, HUMAN21_HIDARI_UDE1 },
	{  0.0f,-130.0f,  0.0f, (8<<17)        |damg_arm_l },/*lvl2 armL1*/
	{  45.0f, 150.0f,  65.0f, HUMAN21_HIDARI_UDE2 },
	{  0.0f,-130.0f,  0.0f, (8<<17)        |damg_arm_l },/*lvl2 armL2*/
	{  85.0f, 200.0f, 100.0f, HUMAN21_MIGI_ASHI1  },
	{-10.0f,-200.0f,  0.0f, (8<<17)|(1<<16)|damg_leg_l },/*lvl2 legR1*/
	{  85.0f, 200.0f, 120.0f, HUMAN21_MIGI_ASHI2  },
	{-20.0f,-250.0f,-20.0f, (8<<17)|(1<<16)|damg_leg_l },/*lvl2 legR2*/
	{  85.0f, 250.0f, 100.0f, HUMAN21_HIDARI_ASHI1},
	{-10.0f,-200.0f,  0.0f, (8<<17)        |damg_leg_l },/*lvl2 legL1*/
	{  85.0f, 250.0f, 120.0f, HUMAN21_HIDARI_ASHI2},
	{-20.0f,-250.0f,-20.0f, (8<<17)        |damg_leg_l },/*lvl2 legL2*/

	{ 200.0f, 300.0f, 100.0f, HUMAN21_ONAKA       },
	{  0.0f,   0.0f,  0.0f,(10<<17)        |damg_body  },/*lvl3 tammy*/
    } ;
    int n_lvls[] = { ORGA_N_LVL0_PARTS, ORGA_N_LVL1_PARTS, ORGA_N_LVL2_PARTS } ;

    o = &work->body ;
    s = size_and_offset ;
    GM_SetTarget( t, ORGA_MAIN_TRG_FLG, map, ENEMY_SIDE, s++, &DG_ZeroVector ) ;
    GM_SetPowerTarget( t, p++, POWER_DECREASE, GM_Vitality, 0, 0, &DG_ZeroVector ) ;
    GM_SetTargetCallBack( t, ORG_MainTargetCallBack, &work->parts[10] ) ;/*俯瞰時のダメージは,お腹*/
    //NewTargetView( t, 34, 184, 200 ) ;
    GM_PutTarget( t++ ) ;

    if ( GCL_GetOption( 'c' ) )
	GCL_GetNextInt(), GCL_GetNextInt(), GCL_GetNextInt(), GCL_GetNextInt() ;/*skip USP,SGR damage*/

    for ( j=0 ; j<3 ; j++ )
    {
	GM_SetTargetParts( &work->target[0], t, n_lvls[j], j ) ;
	for ( i=n_lvls[j] ; --i>=0 ; p++, t++, s+=2, pt++ )
	{
	    /* 部位情報の初期化 */
	    pt->world   = &BODYWORLD( o, (int)(s->vw) ) ;
	    pt->work    = work ;
	    pt->part_id = (int)(s+0)->vw ;
	    pt->damage  = (int)(s+1)->vw ;
	    pt->motion  = (pt->damage & 0x10000 ? -1 : 1 )* (pt->damage & 0xffff) ;
	    pt->damage  = GCL_NextStr() ? GCL_GetNextInt() : (pt->damage >> 17) ; 
	    /* 部位ターゲットの設定 */
	    GM_SetTarget( t, ORGA_PART_TRG_FLG, map, ENEMY_SIDE, s, s+1 ) ;
	    GM_SetTargetWeaponType( t, 0 ) ;
	    GM_SetPowerTarget( t, p, POWER_DECREASE , 5, 3, 3, &DG_ZeroVector ) ;
	    GM_SetTargetCallBack( t, ORG_PartsTargetCallBack, pt ) ;

	    //NewTargetView( t, 200, 34, 184 ) ;
	}
    }

    /* ホーミングターゲットの設定(これでスネークがホーミング出来る) */
    if ( GM_GameLevel < GM_LEVEL_EXTREME )
    {
	GM_SetHomingTrg( &work->homing, &BODYWORLD(o, HUMAN21_KUBI),
			 o, &work->control.hzx_id, NULL, 0 ) ;
	GM_PutHomingTrg( &work->homing ) ;
    }

    return 0 ;
}


int ORG_InitVitalityGage( Work *work )
{
    GM_GageSet	*gs = &work->gage ;
    int flag = GCL_GetOptionValue( 'f', 0 ) ;
   
    GM_InitGageSet( gs,
				   (flag & ORGA_START_MERYLE ? "MERYL" : "OLGA"),
				   16, ORGA_MAX_VITALITY, 3, work->vitality, 
		    ORGA_MAX_VITALITY, 0, 30, 1 ) ;
    GM_SetGageColorType( gs, GM_GAGE_COLOR_TYPE_ENEMY_LIFE );
    GM_InitGageM9( gs, ORGA_MAX_VITALITY, work->vitality_m9, 0, 30 ) ;
    GM_AppendGageSet( gs ) ;
    GM_VisibleGage( gs ) ;

    return 0 ;
}


/* レーダー初期化 */
int ORG_InitRadar( Work *work )
{
    GM_InitRadarControl( &work->radar, &work->control.mov,
			 RADAR_VISIBLE|RADAR_SIGHT|RADAR_NOFIX_SIGHT,
			 work->control.map ) ;
    GM_RadarSetVRange( &work->radar, 2*RADAR_VRANGE_UPPER, 2*RADAR_VRANGE_LOWER ) ;
    GM_RadarSetSight( &work->radar, work->control.rot.vy, 512, 8128.0f, RADAR_COLOR_BLUE ) ;

    return 0 ;
}

/* メッセージ処理 */
void ORG_InitMessages( Work *work )
{
    /* ホロからflagのポインタをもらうメッセージ */
    ORG_SendHoloGetFlagAddrMessage( &work->misc_holo_stat ) ;
    /* スポットライトからflagのポインタをもらうメッセージ */
    ORG_SendSpotLightGetFlagAddrMessage( &work->misc_spot_stat ) ;
}
