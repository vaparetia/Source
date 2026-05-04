//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	fanim.c
	顔アニメキャラ

	2000/06/28 K.Takabe
	$Id: emma_anm.c,v 1.1.1.3 2002/11/19 11:46:03 Yoshizawa1 Exp $

*/
/*
プログラム呼び出しインターフェイス
	void *EMA_InitFaceAnimation( DG_EVMOBJ *evmobj, int fanim_id );
	DG_EVMOBJ *evmobj ;	マルチウェイトエンベロープ型モデル
	int	  fanim_id ;	フェイスアニメファイルＩＤ

	ＥＶＭファイルに対して臨時フェイスアニメーション機構を負荷する
	（以後メッセージにより制御）

	void EMA_ActFaceCommand( void *work, int message[] ) ;

	モーション再生開始メッセージ
	message[0] = 0(128で補間時間が1/1000秒単位に)
	message[1] = ＜モーションファイルＩＤ＞

	視線変更メッセージ
	message[0] = 1(129で補間時間が1/1000秒単位に)
	message[1] = <type> -1:モーション依存 0:絶対座標指定 1:顔相対座標指定 2:カメラ目線
	message[2] = 補間時間
	message[3] = Ｘ座標
	message[4] = Ｙ座標
	message[5] = Ｚ座標
*/

#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"

#include	"gameheader.h"
#include "../../../mode/demo/libdemo.h"

/* ---------------------------------------------------------------- */
/* 各種定義 */
#define TIME_BASE	(BP_BASE_TICK())

#define	JOINT_L_EYE	(21+5)
#define JOINT_R_EYE	(21+6)

#define MAX_X_ROT	(  37.0f * 3.14159265f / 360.0f )
#define MIN_X_ROT	( -19.0f * 3.14159265f / 360.0f )
#define MAX_Y_ROT	(  60.0f * 3.14159265f / 360.0f )
#define MIN_Y_ROT	( -60.0f * 3.14159265f / 360.0f )

/* ---------------------------------------------------------------- */
	/*
		ワーク構造体定義
	*/
typedef	struct	{
    DG_EVMOBJ      *evmobj ;			/* ＥＶＭオブジェクト */
    /* フェイスアニメ */
    int		fanim_id ;		/* ファイルＩＤ */
    int		inf_id   ;		/* 口パクＩＤ */
    MT_FACE_CONTROL	f_ctrl ;		/* フェイスアニメーション制御 */
    /* 顔モーション関係 */
    FVECTOR		face_skel_trans[ FACE_JOINT_MAX ] ;	/* モーショントランスレーション配列 */
    FVECTOR		face_skel_rot[ FACE_JOINT_MAX ] ;	/* モーションクォータニオン配列 */
    /* 視線制御関係 */
    int		eye_pos_type ;	/* 座標パラメータ基準タイプ */
    int		eye_interp_count ;	/* 視線制御補間時間 */
    FVECTOR		target_param ;		/* 座標パラメータ */
    FVECTOR		eye_target ;		/* 視線方向絶対座標 */
    FVECTOR		leye_rot ;			/* 左目回転量 */
    FVECTOR		reye_rot ;			/* 右目回転量 */
} Work ;



/* マトリクスからターゲットを向くためのクォータニオンを計算 */
static void GetDirQuat( FVECTOR *res, FMATRIX *pos, FVECTOR *target )
{
    FMATRIX  inv_mat ;
    FVECTOR  trg, rot ;
    FVECTOR  x, y ;

    _sceVu0InversMatrix( &inv_mat, pos );
    target->vw = 1.0f ;
    _sceVu0ApplyMatrix( &trg, &inv_mat, target );

    /* Ｘ軸回転量算出用 */
    x.vx = sceVu0Sqrt( trg.vx * trg.vx + trg.vz * trg.vz ) ;
    y.vx = -trg.vy ;
    /* Ｙ軸回転量算出用 */
    x.vy = trg.vz ;
    y.vy = trg.vx ;
    /* Ｚ軸回転量算出用 */
    x.vz = 1.0f ;
    y.vz = 0.0f ;
    MT_Atan2X4( &rot, &y, &x );
    /* 角度制限チェック */
    if ( rot.vx > MAX_X_ROT )
	rot.vx = MAX_X_ROT ;
    if ( rot.vx < MIN_X_ROT )
	rot.vx = MIN_X_ROT ;
    if ( rot.vy > MAX_Y_ROT )
	rot.vy = MAX_Y_ROT ;
    if ( rot.vy < MIN_Y_ROT )
	rot.vy = MIN_Y_ROT ;

    /* クォータニオンへ変換 */
    MT_EulerToQuatXYZ( res, &rot );
}


/*

 顔アニメメッセージ代用

 */
void EMA_ActFaceCommand( Work *work, int message[] )
{
    switch ( message[0] )
    {
    case 0:/* モーション発動 */
    case 128:
	MT_SetFaceMotion( &work->f_ctrl, message[1] );
	break ;

    case 1:/* 視線変更 */
    case 129:
	work->eye_pos_type = message[1] ;
	work->eye_interp_count = message[2] ;
	if ( message[0] >= 128 ){
	    work->eye_interp_count = message[2] * 300 / 1000 ;
	}
	work->target_param.vx = (float)message[3] ;
	work->target_param.vy = (float)message[4] ;
	work->target_param.vz = (float)message[5] ;
	work->target_param.vw = 1.0f ;
	break ;
    }
}



/*

  顔のモーションのストリーミング再生

 */
static inline void EMA_StreamMouth( MT_FACE_CONTROL *f_ctrl,
				    int inf_id,
				    int mask )
{
    extern DEMO_MOTION * GM_StreamGetMotion( int id ) ;
    DEMO_MOTION *mtn ;
    int       i ;

    /* 現在のフレームのモーションを得る */
    if( (mtn = GM_StreamGetMotion( inf_id ) ) )
    {
	/* クォータニオンとトランスレーションの取得 */
	for( i=0 ; i<32 ; i++ )
	    if ( mask & (1<<i) )
	    {
		f_ctrl->fix_face.quat[i] = mtn->motion[i*2  ] ;
		f_ctrl->fix_face.trans[i] = mtn->motion[i*2+1] ;
	    }
    }
}


/*

  キャラアクター関数

  */
void EMA_ActFaceAnimation( Work *work )
{
    DG_EVMOBJ	*evmobj ;
    FMATRIX		mat ;
    FVECTOR		trans ;
    EVM_SKEL	*l_eye_skel, *r_eye_skel ;
    float		t ;

    /* 顔アニメ */
    MT_ActFaceMotion( &work->f_ctrl );

    /* 口パク */
    EMA_StreamMouth( &work->f_ctrl, work->inf_id, 0x0073f00f ) ;

    /* 視線制御処理 */
    if ( work->eye_pos_type != -1 )
    {
	evmobj = work->evmobj ;

	/* 眼球用スケルトン取得 */
	l_eye_skel = &evmobj->def->skeleton[ JOINT_L_EYE ] ;
	r_eye_skel = &evmobj->def->skeleton[ JOINT_R_EYE ] ;

	trans.vw = 1.0f ;
	/* 視線方向絶対座標を求める */
	switch ( work->eye_pos_type )
	{
	case 0:/* 絶対座標 */
	    work->eye_target = work->target_param ;
	    break ;
	case 1:/* 頭部モデル相対座標 */
	    mat = evmobj->matrix[ evmobj->use_buffer ][ 8 ] ;
	    trans.vx = evmobj->def->skeleton[ 8 ].rt_tx ;
	    trans.vy = evmobj->def->skeleton[ 8 ].rt_ty ;
	    trans.vz = evmobj->def->skeleton[ 8 ].rt_tz ;
	    _sceVu0ApplyMatrix( (FVECTOR*)&mat.m[3][0], &mat, &trans );
	    _sceVu0ApplyMatrix( &work->eye_target, &mat, &work->target_param );
	    break ;
	case 2:/* カメラ目線 */
	    work->eye_target.vx = DG_Chanl(0)->eye.m[3][0] ;
	    work->eye_target.vy = DG_Chanl(0)->eye.m[3][1] ;
	    work->eye_target.vz = DG_Chanl(0)->eye.m[3][2] ;
	    break ;
	}

	/* 左目の位置のマトリクスを求める */
	mat = evmobj->matrix[ evmobj->use_buffer ][ l_eye_skel->parent ] ;
	trans.vx = l_eye_skel->rt_tx ;
	trans.vy = l_eye_skel->rt_ty ;
	trans.vz = l_eye_skel->rt_tz ;
	_sceVu0ApplyMatrix( (FVECTOR*)&mat.m[3][0], &mat, &trans );
	/* 左目のクォータニオンを算出 */
	GetDirQuat( &work->leye_rot, &mat, &work->eye_target );

	/* 右目の位置のマトリクスを求める */
	mat = evmobj->matrix[ evmobj->use_buffer ][ r_eye_skel->parent ] ;
	trans.vx = r_eye_skel->rt_tx ;
	trans.vy = r_eye_skel->rt_ty ;
	trans.vz = r_eye_skel->rt_tz ;
	_sceVu0ApplyMatrix( (FVECTOR*)&mat.m[3][0], &mat, &trans );
	/* 右目のクォータニオンを算出 */
	GetDirQuat( &work->reye_rot, &mat, &work->eye_target );

	/* 眼球用スケルトン回転の設定 */
	work->eye_interp_count -= TIME_BASE ;
	if ( work->eye_interp_count < TIME_BASE )
	    work->eye_interp_count = TIME_BASE ;
	t = (float)TIME_BASE / work->eye_interp_count ;
	if ( work->eye_interp_count <= TIME_BASE )
	{
	    /* 補間なし */
	    work->face_skel_rot[JOINT_L_EYE-21] = work->leye_rot ;
	    work->face_skel_rot[JOINT_R_EYE-21] = work->reye_rot ;
	}
	else
	{
	    /* 補間あり */
	    MT_QuatSlerp( &work->face_skel_rot[JOINT_L_EYE-21],
			  &work->face_skel_rot[JOINT_L_EYE-21],
			  &work->leye_rot, t );
	    MT_QuatSlerp( &work->face_skel_rot[JOINT_R_EYE-21],
			  &work->face_skel_rot[JOINT_R_EYE-21],
			  &work->reye_rot, t );
	    MT_QuatNormalize( &work->face_skel_rot[JOINT_L_EYE-21],
			      &work->face_skel_rot[JOINT_L_EYE-21] );
	    MT_QuatNormalize( &work->face_skel_rot[JOINT_R_EYE-21],
			      &work->face_skel_rot[JOINT_R_EYE-21] );
	}

	work->f_ctrl.fix_face.quat[JOINT_L_EYE-21] =
	    work->face_skel_rot[JOINT_L_EYE-21] ;
	work->f_ctrl.fix_face.quat[JOINT_R_EYE-21] =
	    work->face_skel_rot[JOINT_R_EYE-21] ;
    }

    /* 顔モーション設定 */
    MT_PutFaceMotion( work->evmobj, &work->f_ctrl );
}

/*

  ワークの確保

 */
void *EMA_InitFaceAnimation( DG_EVMOBJ *evmobj, int fanim_id, int inf_id )
{
    Work *work ;

    if ( (work = GV_Malloc( sizeof(Work) )) )
    {
	work->evmobj   = evmobj ;
	work->fanim_id = fanim_id ;
	work->inf_id   = inf_id   ;

	MT_InitFaceControl( &work->f_ctrl, work->fanim_id, 0 );

	work->eye_pos_type = -1 ;
    }

    return work ;
}

void EMA_FreeFaceAnimation( void *work ) 
{
    if ( work )
	GV_Free( work ) ;
}
