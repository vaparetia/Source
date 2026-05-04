//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  vamp_dsp.c 
  ヴァンプ表示用関数群

  2001/03/23 T.Morita
  $Id: vamp_dsp.c,v 1.1.1.3 2002/11/19 11:46:35 Yoshizawa1 Exp $
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

#include "include/vamp.h"
#include "../../mode/demo/libdemo.h"



#if 0
/* 顔のモーションのストリーミング再生 */
static void VMPS_DispFaceAnimation( Work *work )
{
    DEMO_MOTION *mtn ;
    int       i ;
    FMATRIX  *skel_mats, *mats ;
    EVM_SKEL *skel ;
    FVECTOR  *rot, *trans, vec ;
    extern DEMO_MOTION * GM_StreamGetMotion( int id ) ;

    /* 現在のフレームのモーションを得る */
    if( (mtn = GM_StreamGetMotion( GV_StrCode( "エマ" ) ) ) )
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
#endif


/*
  方向を見る
 */
static inline void VMPS_DispFaceSomething( Work *work, FVECTOR *dir )
{
    FVECTOR d ;
    float   turn_y ;

    /* 左右は腰と胸 */
    _sceVu0SubVector( &d, &VMPS_Directions[work->aim_pose], &work->control.mov ) ;
    turn_y  = atan2f( d.vx, d.vz ) ;
    turn_y -= work->control.rot.vy * M_PI / 2048.0f ;
    turn_y += turn_y<-M_PI ? M_PI*2 : turn_y>M_PI ? -M_PI*2 : 0.0f ;
    dir->vy = turn_y*0.5f ;
    if ( work->body.m_ctrl->flag & MT_FLAG_REVERSAL1 )
	dir->vy *= -1 ;
    dir->vx = VMPS_Directions[work->aim_pose].vw ;
    _sceVu0ClampVector( dir, dir, -M_PI/6, M_PI/6 ) ;/* 一定以上曲がらないようクランプする */
}

/*
  間接の線形補間
*/
static inline void VMPS_DispFaceToInterpolation( Work *work,
						FVECTOR *org, FVECTOR *quat,
						int id1, int id2 )
{
    FVECTOR d ;
    static int reversal_flag ;

    if ( (work->body.m_ctrl->flag & MT_FLAG_REVERSAL1) ^ reversal_flag ) /*Reverse のON/OFFの*/
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
    reversal_flag = (work->body.m_ctrl->flag & MT_FLAG_REVERSAL1) ;
}


/* 方向向き表示 */
static inline void VMPS_DisplayDirection( Work *work )
{
    FVECTOR  direction ;

    /* リセット */
    _sceVu0CopyVector( &direction, &DG_ZeroVector ) ;
    work->body.m_ctrl->adjust_flag = 0 ;

    /* 顔向け */
    if ( !(work->flag & VMPS_F_NON_FACING) )
	VMPS_DispFaceSomething( work, &direction ) ;
    VMPS_DispFaceToInterpolation( work, &work->head_dir  , &direction,
				 HUMAN21_ATAMA, HUMAN21_KUBI ) ;
    /* 胸向け */
    if ( !(work->flag & VMPS_F_NON_FACING) )
	_sceVu0ScaleVector( &direction, &direction, -1.0f ) ;
    VMPS_DispFaceToInterpolation( work, &work->breast_dir, &direction,
				 HUMAN21_ONAKA, HUMAN21_MUNE ) ;
}


/* レーダー表示*/
static inline void VMPS_DisplayRadar( Work *work )
{
    float(*m)[4] = BODYWORLD( &work->body, HUMAN21_ATAMA ).m ;
    float sb = m[X][Y] ;
    float ry ;

    /* 頭の向きを計算する(マトリックスからオイラー角を計算する) */
    if ( sb <= -1.0f || sb >= 1.0f )
        ry = atan2f(  m[Z][X], m[Z][Z] ) ;
    else
	ry = atan2f( -m[X][Z], m[X][X] ) ;
    ry = ry*2048.0f/M_PI ;

    GM_RadarSetSight( &work->rctrl, (int)ry, 512, 8128.0f, RADAR_COLOR_RED ) ;
}

/* ゲージを更新 */
static inline void VMPS_DisplayGage( Work *work )
{
    work->gage.value = work->npc.action.life ;
    work->gage.m9_value = work->vital_m9 ;
}

#if 0
static inline void VMPS_DisplayFaceAnime( Work *work )
{
    if ( work->npc.action.face_anime )
		VMPS_SendMessageFaceAnimePlayMotion( work->npc.action.face_anime ) ;
    else
    {
	int tic = GM_StagePlayTime & 0xff ;

	/* 目パチ制御 */
	if ( tic == work->blink_tic )
	{
	    work->blink_tic = irnd()&255 ;
	    work->blink_tic = work->blink_tic<30 ? 30 : work->blink_tic ;
	    work->blink_tic = (tic + work->blink_tic) & 255 ;
	    VMPS_SendMessageFaceAnimePlayMotion( 1 ) ;
	}
    }
}
#endif

/*

  Prepareing for display

*/
void VMPS_Display( Work *work )
{
    VMPS_DisplayDirection( work ) ;

#if 0
    if ( work->str_hdl )
    {
	if ( work->flag & ORGA_F_DEMO_MOVIE || GM_Item == IT_Camera || GM_Item == IT_Scope )
	{
	    /*マルチウェイト表示*/
	    work->body.evmobj->flag &= ~DG_EVMOBJ_INVISIBLE ;
	    DG_InvisibleObjs( work->body.objs ) ;
	    /* 口パクアニメ */
	    VMPS_DispFaceAnimation( work ) ;
	}
	else
	{
	    /*シングルウェイト表示*/
	    work->body.evmobj->flag |=  DG_EVMOBJ_INVISIBLE ;
	    DG_VisibleObjs( work->body.objs ) ;
	}
    }
#endif

    VMPS_DisplayGage( work ) ;
    VMPS_DisplayRadar( work ) ;
    //VMPS_DisplayFaceAnime( work ) ;
}
