//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   fort_dsp.c 
   フォーチュン 表示用関数群

   1999/12/22 T.Morita
   $Id: fort_dsp.c,v 1.1.1.3 2002/11/19 11:46:06 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "libdg.h"
#include "libutl.h"
#include "gameheader.h"

#include "include/fort.h"
#include "../../mode/demo/libdemo.h"




/* 顔のモーションのストリーミング再生 */
static void FRT_DispFaceAnimation( Work *work )
{
    DEMO_MOTION *mtn ;
    int       i ;
    FMATRIX  *skel_mats, *mats ;
    EVM_SKEL *skel ;
    FVECTOR  *rot, *trans, vec ;
    extern DEMO_MOTION * GM_StreamGetMotion( int id ) ;

    /* 現在のフレームのモーションを得る */
    if( (mtn = GM_StreamGetMotion( GV_StrCode( "フォーチュン" ) ) ) )
    {
	/* 顔の全関節の値を設定する */
	skel_mats = work->body.evmobj->matrix[ work->body.evmobj->use_buffer ] ; 
	skel = work->body.evmobj->def->skeleton + 21 ;
	vec.vw = 1.0F;
	for( i=0 ; i<32 ; i++, skel++ )
	{
	    mats = &skel_mats[i + 21] ;

	    /* クォータニオンとトランスレーションの取得 */
	    rot   = &mtn->motion[i*2  ] ;
	    trans = &mtn->motion[i*2+1] ;
	    /* ローカルマトリックスを作成 */
	    MT_QuatToMat( mats, rot ) ;
	    mats->m[W][X] = skel->rt_tx + trans->vx ;
	    mats->m[W][Y] = skel->rt_ty + trans->vy ;
	    mats->m[W][Z] = skel->rt_tz + trans->vz ;
	    /* 親を掛け ワールドマトリックスに変換 */
	    _sceVu0MulMatrix( mats, &skel_mats[ skel->parent ], mats ) ;

	    vec.vx = -skel->rt_tx;
	    vec.vy = -skel->rt_ty;
	    vec.vz = -skel->rt_tz;
	    _sceVu0ApplyMatrix( (FVECTOR *)mats->m[W], mats, &vec ) ;
	}
    }
}


#if 0
/*
  頭の向き 頭のオブジェクトから work->trgt_eye への方向に向く
*/
static void FRT_DispFaceToPlayer_Head( Work *work, FVECTOR *head, FVECTOR *breast )
{
    FVECTOR d ;

    _sceVu0SubVector( &d, work->trgt_eye, BODYPOS(&work->body,HUMAN21_ATAMA) ) ;

    d.vw = atan2f( d.vx, d.vz ) ;
    d.vw = work->control.turn.vy * M_PI / 2048.0f ;
    d.vw += d.vw<-M_PI ? M_PI*2 : d.vw>M_PI ? -M_PI*2 : 0.0f ;
    head->vy = d.vw * 0.5f ;
    head->vx = -atan2f( d.vy, bp_sqrtf( d.vx*d.vx + d.vz*d.vz )) / 2 - breast->vx ; //BP_MATH - emulate PS2 sqrtf
    if ( work->body.m_ctrl->flag & MT_FLAG_REVERSAL1 )
	head->vy *= -1 ;
    head->vy -= breast->vy ;
    _sceVu0ClampVector( head, head, -M_PI/6, M_PI/6 ) ;/*一定以上 曲がらない*/
    head->vz = -head->vy/32 ;
}
#endif

/*
  両手撃ちの狙い  右手のオブジェクトから work->trgt_aim への方向に向く
 */
static void FRT_DispFaceToPlayer_AimBothArms( Work *work, FVECTOR *breast,
					      FVECTOR *arm_l, FVECTOR *arm_r )
{
    FVECTOR d ;

    /* 左右は腰と胸 */
    _sceVu0SubVector( &d,
		      FRT_RecogAimPos( work ),
		      BODYPOS(&work->body,HUMAN21_MIGI_TE) ) ;

    d.vw = atan2f( d.vx, d.vz ) ;
    d.vw -= work->control.turn.vy * M_PI / 2048.0f ;
    d.vw += d.vw<-M_PI ? M_PI*2 : d.vw>M_PI ? -M_PI*2 : 0.0f ;
    breast->vy =  d.vw*0.5f ;
    if ( work->body.m_ctrl->flag & MT_FLAG_REVERSAL1 )
	breast->vy *= -1 ;
    _sceVu0ClampVector( breast, breast, -M_PI/6, M_PI/6 ) ;/* 一定以上曲がらないようクランプする */

    /* 上下は腕 */
    arm_r->vx = arm_l->vx = -atan2f( d.vy, bp_sqrtf( d.vx*d.vx + d.vz*d.vz ) ) / 2 ;   //BP_MATH - emulate PS2 sqrtf
}




/*
  間接の線形補間
*/
static void FRT_DispFaceToInterpolation( Work *work,
					 FVECTOR *org, FVECTOR *quat,
					 int id1, int id2,
					 int flag )
{
    FVECTOR d ;

    if ( (work->body.m_ctrl->flag & MT_FLAG_REVERSAL1) ^ flag ) /*Reverse のON/OFFの*/
	org->vy *= -1 ;
    _sceVu0SubVector( &d, quat, org ) ;
    _sceVu0ScaleVector( &d, &d, 0.25f ) ;
    if ( org->vx != 0.0f || org->vy != 0.0f || org->vz != 0.0f ||
	 d.vx    != 0.0f || d.vy    != 0.0f || d.vz    != 0.0f )
    {
	_sceVu0AddVector( org, org, &d ) ;
	work->body.m_ctrl->adjust_flag |= (1 << id1)|(1 << id2) ;
	MT_EulerToQuatXYZ( &work->body.m_ctrl->adjust[id1], org ) ;
	MT_EulerToQuatXYZ( &work->body.m_ctrl->adjust[id2], org ) ;
    }
}


static inline void FRT_DisplayFaceAnime( Work *work )
{
    if ( work->str_hdl )
    {
	if ( work->flag & FRT_F_DEMO_MOVIE || GM_Item == IT_Camera || GM_Item == IT_Scope )
	{
	    /*マルチウェイト表示*/
	    work->body.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE ;
	    DG_InvisibleObjs( work->body.objs ) ;
	    /* 口パクアニメ */
	    FRT_DispFaceAnimation( work ) ;
	}
	else
	{
	    /*シングルウェイト表示*/
	    work->body.evmobj->flag |=  DG_EVMOBJ_INVISIBLE ;
	    DG_VisibleObjs( work->body.objs ) ;
	}
    }
}



static inline void FRT_DisplayRadar( Work *work )
{
    float(*m)[4] = BODYWORLD( &work->body, HUMAN21_ATAMA ).m ;
    float sb = m[X][Y] ;
    float ry ;
    int   color ;

    /* 頭の向きを計算する(マトリックスからオイラー角を計算する) */
    if ( sb <= -1.0f || sb >= 1.0f )
        ry = atan2f(  m[Z][X], m[Z][Z] ) ;
    else
	ry = atan2f( -m[X][Z], m[X][X] ) ;
    ry = ry*2048.0f/M_PI ;

    if ( work->flag & FRT_F_UNRECOG_POS && !work->act_flg )
	color = RADAR_COLOR_YELOW ;
    else
	color = RADAR_COLOR_RED ;

    /* レーダー表示*/
    GM_RadarSetSight( &work->radar, (int)ry, 512, 8128.0f, color ) ;
}


/*

  Prepareing for display

*/
void FRT_Display( Work *work )
{
    FVECTOR  breast = DG_ZeroVector ;
    FVECTOR  arm_r  = DG_ZeroVector ;
    FVECTOR  arm_l  = DG_ZeroVector ;
    static int reversal_flag ;
#if 0
    FVECTOR  head   = DG_ZeroVector ;
#endif

    work->body.m_ctrl->adjust_flag = 0 ;

    if ( work->flag & FRT_F_FACETO_AIM  )
	FRT_DispFaceToPlayer_AimBothArms( work, &breast, &arm_r, &arm_l ) ;
#if 0
    if ( work->flag & FRT_F_FACETO_HEAD )
	FRT_DispFaceToPlayer_Head( work, &head, &breast ) ;
#endif
    FRT_DispFaceToInterpolation( work, &work->arm_r , &arm_r ,
				 HUMAN21_MIGI_KATA  , HUMAN21_MIGI_UDE1  , reversal_flag ) ;
    FRT_DispFaceToInterpolation( work, &work->arm_l , &arm_l ,
				 HUMAN21_HIDARI_KATA, HUMAN21_HIDARI_UDE1, reversal_flag ) ;
#if 0
    FRT_DispFaceToInterpolation( work, &work->head  , &head  ,
				 HUMAN21_ATAMA      , HUMAN21_KUBI       , reversal_flag ) ;
#endif
    FRT_DispFaceToInterpolation( work, &work->breast, &breast,
				 HUMAN21_ONAKA      , HUMAN21_MUNE       , reversal_flag ) ;

    reversal_flag = (work->body.m_ctrl->flag & MT_FLAG_REVERSAL1) ;

    FRT_DisplayFaceAnime( work ) ;

    FRT_DisplayRadar( work ) ;
}

