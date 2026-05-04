//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	fanim.c
	顔アニメキャラ

	2000/06/28 K.Takabe
	$Id: fanim.c,v 1.1.1.3 2002/11/19 11:51:10 Yoshizawa1 Exp $

*/
/*

シナリオ呼び出しインターフェイス
chara 顔アニメキャラ[NewFaceAnimationSet] $s:name \
	-fanim	$s:フェイスアニメファイル指定 \
	-chara	$s:キャラクタ名
mesg 顔アニメキャラ $s:名前 モーション再生[0] $s:モーション名
mesg 顔アニメキャラ $s:名前 視線変更[1] $w:タイプ [$w:補間時間 $v:注目座標]
// マルチウェイトエンベロープモデル顔アニメキャラ
// メッセージによりコントロールする（時間は全て1/300秒単位）
// ＜視線変更コマンド捕捉＞
//  $w:タイプ = -1 視線はモーションに依存（補間時間、注目座標の指定は必要なし）
//            =  0 絶対座標による注目座標指定
//            =  1 頭部モデルからの相対座標による注目座標指定
//            =  2 カメラ目線（注目座標の指定は必要なし）
//  


プログラム呼び出しインターフェイス
	void *NewFaceAnimation( int name, DG_EVMOBJ *evmobj, int fanim_id );
	int			name ;		キャラ名
	DG_EVMOBJ	*evmobj ;	マルチウェイトエンベロープ型モデル
	int			fanim_id ;	フェイスアニメファイルＩＤ

	ＥＶＭファイルに対して臨時フェイスアニメーション機構を負荷する
	（以後メッセージにより制御）


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

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"

/* ---------------------------------------------------------------- */
/* 各種定義 */
#define TIME_BASE	(BP_BASE_TICK())

#define	JOINT_L_EYE	(21+5)
#define JOINT_R_EYE	(21+6)
//#define MAX_X_ROT	(  19.0f * 3.14159265f / 360.0f )
//#define MIN_X_ROT	( -19.0f * 3.14159265f / 360.0f )
//#define MAX_Y_ROT	(  55.0f * 3.14159265f / 360.0f )
//#define MIN_Y_ROT	( -30.0f * 3.14159265f / 360.0f )

#define MAX_X_ROT	(  37.0f * 3.14159265f / 360.0f )
#define MIN_X_ROT	( -19.0f * 3.14159265f / 360.0f )
#define MAX_Y_ROT	(  60.0f * 3.14159265f / 360.0f )
#define MIN_Y_ROT	( -60.0f * 3.14159265f / 360.0f )

/* ---------------------------------------------------------------- */
typedef struct {
	FVECTOR	root ;
	FVECTOR	joints[64] ;
	FVECTOR	quat_buffer[4] ;
	FVECTOR	vec ;
	FMATRIX	mat ;
	FMATRIX	root_mat ;
	FMATRIX	mats[64] ;
} ScrPadWork ;

/* ---------------------------------------------------------------- */
	/*
		ワーク構造体定義
	*/
typedef	struct	{
	GV_ACT_EX	actor ;
	int			name ;				/* キャラクタ名 */
	DG_EVMOBJ	*evmobj ;			/* ＥＶＭオブジェクト */
	/* フェイスアニメ */
	int				fanim_id ;		/* ファイルＩＤ */
	MT_FACE_CONTROL	f_ctrl ;		/* フェイスアニメーション制御 */
	/* 顔モーション関係 */
	FVECTOR		face_skel_trans[ FACE_JOINT_MAX ] ;	/* モーショントランスレーション配列 */
	FVECTOR		face_skel_rot[ FACE_JOINT_MAX ] ;	/* モーションクォータニオン配列 */
	/* 視線制御関係 */
	int			eye_pos_type ;	/* 座標パラメータ基準タイプ */
	int			eye_interp_count ;	/* 視線制御補間時間 */
	FVECTOR		target_param ;		/* 座標パラメータ */
	FVECTOR		eye_target ;		/* 視線方向絶対座標 */
	FVECTOR		leye_rot ;			/* 左目回転量 */
	FVECTOR		reye_rot ;			/* 右目回転量 */
} Work ;

/* ---------------------------------------------------------------- */
#if 0
/* ＥＶＭオブジェクトの一部に直接モーションを設定 */
static void EvmActMotionImmediate( DG_EVMOBJ *evmobj, int first, int n_joints, FVECTOR *skel_trans, FVECTOR *skel_rot )
{
	int			i ;
	ScrPadWork	*scrpad  = (ScrPadWork*)SCRPAD_ADDR ;
	FMATRIX		*skel_mats ;
	FVECTOR		vec ;
	FMATRIX		*mats = scrpad->mats ;
	EVM_SKEL	*skel ;

	skel_mats = evmobj->matrix[ evmobj->use_buffer ] ;

	/* オブジェクトにマトリクスを設定する */
	skel = &evmobj->def->skeleton[ first ] ;
	vec.vw = 1.0F ;
	/* モデル情報から親子関係を取得して求める */
	for ( i = first ; i < ( first + n_joints ) ; i++ ){
		FMATRIX	*parent ;
		MT_QuatToMat( mats, skel_rot );
		mats->m[3][0] = skel->rt_tx + skel_trans->vx ;
		mats->m[3][1] = skel->rt_ty + skel_trans->vy ;
		mats->m[3][2] = skel->rt_tz + skel_trans->vz ;
		parent = &skel_mats[ skel->parent ] ;
		_sceVu0MulMatrix( mats, parent, mats ) ;
		vec.vx = -skel->rt_tx ;
		vec.vy = -skel->rt_ty ;
		vec.vz = -skel->rt_tz ;
		_sceVu0ApplyMatrix( &mats->m[3][0], mats, &vec );
		skel_mats[i] = *mats ;

		skel_rot++ ;
		skel_trans++ ;
		skel++ ;
		mats++ ;
	}
}
#endif
#if 0
static void InterpVector( FVECTOR *res, FVECTOR *from, FVECTOR *to, float t )
{
#if 1
	asm ("
		mfc1		$8,%3
		lqc2		vf1,0(%1)
		lqc2		vf2,0(%2)
		qmtc2		$8,vf3
		vmulax.xyz	ACC,vf2,vf3
		vmsubax.xyz	ACC,vf1,vf3
		vmaddw.xyz	vf1,vf1,vf0
		sqc2		vf1,0(%0)
	"::"r"(res),"r"(from),"r"(to),"f"(t): "$8" );
#else
	res->vx = ( to->vx - from->vx ) * t + from->vx ;
	res->vy = ( to->vy - from->vy ) * t + from->vy ;
	res->vz = ( to->vz - from->vz ) * t + from->vz ;
#endif
}
#endif

/* マトリクスからターゲットを向くためのクォータニオンを計算 */
static void GetDirQuat( FVECTOR *res, FMATRIX *pos, FVECTOR *target )
{
	FMATRIX	inv_mat ;
	FVECTOR	trg, rot, x, y ;
	float	l ;

	_sceVu0InversMatrix( &inv_mat, pos );
	//InversMatrix( &inv_mat, pos );
	target->vw = 1.0f ;
	_sceVu0ApplyMatrix( &trg, &inv_mat, target );
	//printf("%6.4f %6.4f %6.4f,%6.4f %6.4f %6.4f \n",
	//	   target->vx, target->vy, target->vz,
	//	   trg.vx, trg.vy, trg.vz );
	l = trg.vx * trg.vx + trg.vz * trg.vz ;
	l = bp_sqrtf( l );   //BP_MATH - emulate PS2 sqrtf
	/* Ｘ軸回転量算出用 */
	x.vx = l ;
	y.vx = -trg.vy ;
	/* Ｙ軸回転量算出用 */
	x.vy = trg.vz ;
	y.vy = trg.vx ;
	/* Ｚ軸回転量算出用 */
	x.vz = 1.0f ;
	y.vz = 0.0f ;
	MT_Atan2X4( &rot, &y, &x );
	/* 角度制限チェック */
	if ( rot.vx > MAX_X_ROT ) rot.vx = MAX_X_ROT ;
	if ( rot.vx < MIN_X_ROT ) rot.vx = MIN_X_ROT ;
	if ( rot.vy > MAX_Y_ROT ) rot.vy = MAX_Y_ROT ;
	if ( rot.vy < MIN_Y_ROT ) rot.vy = MIN_Y_ROT ;
	//printf("%f %f\n", rot.vx * 360 / M_PI, rot.vy * 360 / M_PI );
	/* クォータニオンへ変換 */
	MT_EulerToQuatXYZ( res, &rot );
}

/* 指定した名前のコントロールを検索 */
static CONTROL* SearchControl( int name )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		return ( control ) ;
	}
	printf("search faild control !!\n");
	return ( NULL );
}


/* ---------------------------------------------------------------- */
	/*
		キャラアクター関数
	*/
static void Act( Work *work )
{
	int		n_msg ;
	GV_MSG	*msg ;
	DG_EVMOBJ	*evmobj ;

	/* メッセージチェック */
	if( ( n_msg = GV_ReceiveMessage( work->name, &msg ) ) ){
		for ( ; n_msg > 0 ; n_msg--, msg++ ){
			switch ( msg->message[0] ){

			  case 0:/* モーション発動 */
			  case 128:
				MT_SetFaceMotion( &work->f_ctrl, msg->message[1] );
				break ;

			  case 1:/* 視線変更 */
			  case 129:
				work->eye_pos_type = msg->message[1] ;
				work->eye_interp_count = msg->message[2] ;
				if ( msg->message[0] >= 128 ){
					work->eye_interp_count = msg->message[2] * 300 / 1000 ;
				}
				work->target_param.vx = msg->message[3] ;
				work->target_param.vy = msg->message[4] ;
				work->target_param.vz = msg->message[5] ;
				work->target_param.vw = 1.0f ;
				break ;

			}
		}
	}

#if 0
	if ( GV_PadData[0].press & PAD_A ){
		MT_SetFaceMotion( &work->f_ctrl, 0 );
	}
	if ( GV_PadData[0].press & PAD_B ){
		MT_SetFaceMotion( &work->f_ctrl, 1 );
	}
	if ( GV_PadData[0].press & PAD_X ){
		MT_SetFaceMotion( &work->f_ctrl, 3 );
	}
#endif

	evmobj = work->evmobj ;

	MT_ActFaceMotion( &work->f_ctrl );

	if ( work->eye_pos_type != -1 ){/* 視線制御処理 */
		FMATRIX		mat ;
		FVECTOR		trans ;
		EVM_SKEL	*l_eye_skel, *r_eye_skel ;
		float		t ;

		/* 眼球用スケルトン取得 */
		l_eye_skel = &evmobj->def->skeleton[ JOINT_L_EYE ] ;
		r_eye_skel = &evmobj->def->skeleton[ JOINT_R_EYE ] ;

		trans.vw = 1.0f ;
		/* 視線方向絶対座標を求める */
		switch ( work->eye_pos_type ){
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
		if ( work->eye_interp_count < TIME_BASE ) work->eye_interp_count = TIME_BASE ;
		t = (float)TIME_BASE / work->eye_interp_count ;
		if ( work->eye_interp_count <= TIME_BASE ){
			/* 補間なし */
			work->face_skel_rot[JOINT_L_EYE-21] = work->leye_rot ;
			work->face_skel_rot[JOINT_R_EYE-21] = work->reye_rot ;
		} else {
			/* 補間あり */
			MT_QuatSlerp( &work->face_skel_rot[JOINT_L_EYE-21],
						 &work->face_skel_rot[JOINT_L_EYE-21],
						 &work->leye_rot, t );
			MT_QuatSlerp( &work->face_skel_rot[JOINT_R_EYE-21],
						 &work->face_skel_rot[JOINT_R_EYE-21],
						 &work->reye_rot, t );
			MT_QuatNormalize( &work->face_skel_rot[JOINT_L_EYE-21], &work->face_skel_rot[JOINT_L_EYE-21] );
			MT_QuatNormalize( &work->face_skel_rot[JOINT_R_EYE-21], &work->face_skel_rot[JOINT_R_EYE-21] );
		}

		work->f_ctrl.fix_face.quat[JOINT_L_EYE-21] = work->face_skel_rot[JOINT_L_EYE-21] ;
		work->f_ctrl.fix_face.quat[JOINT_R_EYE-21] = work->face_skel_rot[JOINT_R_EYE-21] ;
	}

	/* 顔モーション設定 */
	//EvmActMotionImmediate( work->evmobj, 21, work->mtn_data.joints, work->face_skel_trans, work->face_skel_rot );
	//EvmActMotionImmediate( work->evmobj, 21, 32, work->face_skel_trans, work->face_skel_rot );
	MT_PutFaceMotion( work->evmobj, &work->f_ctrl );

}

/* ---------------------------------------------------------------- */
	/*
		キャラアクター終了関数
	*/
static void Die( Work *work )
{
}

/* ---------------------------------------------------------------- */
	/*
		キャラクタ初期化関数
	*/
static int GetResources_S( Work *work, int name, int where )
{
	int		chara_id ;
	CONTROL	*ctrl ;
	OBJECT	*object ;

	work->name = name ;

	/* データファイル取得 */
	if ( GCL_GetOption( 'f' ) == NULL ) return ( -1 );
	work->fanim_id = GCL_GetNextInt() ;
	MT_InitFaceControl( &work->f_ctrl, work->fanim_id, 0 );

	/* キャラクタ取得 */
	if ( GCL_GetOption( 'c' ) == NULL ) return ( -1 );
	chara_id = GCL_GetNextInt() ;
	ctrl = SearchControl( chara_id );
	if ( ctrl == NULL ){
		printf("e3_fanim.c: search error control\n");
		return ( -1 );
	}
	if ( ctrl->object == NULL ){
		printf("e3_fanim.c: control initialize miss error\n");
		return ( -1 );
	}
	object = ctrl->object ;
	if ( object->evmobj == NULL ){
		printf("e3_fanim.c: error: no support chara\n");
		return ( -1 );
	}

	work->evmobj = object->evmobj ;

	work->eye_pos_type = -1 ;
	//work->eye_pos_type = 2 ;

	return ( 0 );
}

static int GetResources_P( Work *work, int name, DG_EVMOBJ *evmobj, int fanim_id )
{
	work->name = name ;
	work->evmobj = evmobj ;
	work->fanim_id = fanim_id ;

	MT_InitFaceControl( &work->f_ctrl, work->fanim_id, 0 );

	work->eye_pos_type = -1 ;

	return ( 0 );
}

/* ---------------------------------------------------------------- */
	/*
		シナリオインターフェイス
	*/
void *NewFaceAnimationSet( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources_S( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/* ---------------------------------------------------------------- */
	/*
		プログラムインターフェイス
	*/
void *NewFaceAnimation( int name, DG_EVMOBJ *evmobj, int fanim_id )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources_P( work, name, evmobj, fanim_id ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

