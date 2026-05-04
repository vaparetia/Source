//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	eye_ctrl.c
	デモ用視線制御プログラム

	2000/11/10 K.Takabe
	$Id: eye_ctrl2.c,v 1.1.1.3 2002/11/19 11:48:31 Yoshizawa1 Exp $

*/
/*

	void *NewEyeControl( int name, OBJECT *body, FMATRIX *target,
						FVECTOR *rmax, FVECTOR *rmin, FVECTOR *lmax, FVECTOR *lmin, int flag )
	int		name ;		エフェクト名（現在使用）
	OBJECT	*body ;		目を動かすキャラのオブジェクトのポインタ
	FMATRIX	*target ;	視線制御用ダミーオブジェクトのマトリクスへのポインタ
	FVECTOR	*rmax ;		右目最大角度（Ｘ、Ｙのみ有効。ラジアンで指定）
	FVECTOR	*rmin ;		右目最小角度（Ｘ、Ｙのみ有効。ラジアンで指定）
	FVECTOR	*lmax ;		左目最大角度（Ｘ、Ｙのみ有効。ラジアンで指定）
	FVECTOR	*lmin ;		左目最小角度（Ｘ、Ｙのみ有効。ラジアンで指定）
	int		flag ;		将来の拡張用フラグ

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

#include "../../mode/demo/eft_con.h"

/* ---------------------------------------------------------------- */
/* 各種定義 */

#define	JOINT_L_EYE	(21+5)
#define JOINT_R_EYE	(21+6)


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
	int			target_name;
	DG_EVMOBJ	*evmobj ;			/* ＥＶＭオブジェクト */
	FVECTOR		*target_pos ;		/* 視線方向用ダミーオブジェクトの位置 */
	/* 視線制御関係 */
	FVECTOR		eye_target ;		/* 視線方向絶対座標 */
	FVECTOR		leye_max ;
	FVECTOR		leye_min ;
	FVECTOR		reye_max ;
	FVECTOR		reye_min ;

	int			mode ;
} Work ;
#if 0
static CONTROL* SearchControl( int name, int map )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		//if ( ( control->map & map ) == 0 ) continue ;
		return ( control ) ;
	}
	printf("eye_ctrl.c: search faild control !!\n");
	return ( NULL );
}

static int CheckControl( int name, int map )
{
	int		i ;
	CONTROL	*control, **w_list ;

	w_list = GM_WhereList ;
	for ( i = GM_N_WhereList ; i > 0 ; w_list++, i-- ){
		control = *w_list ;
		if ( control->name != name ) continue ;
		return (0) ;
	}
	return (1);
}
#endif
/* ---------------------------------------------------------------- */
/* マトリクスからターゲットを向くためのマトリクスを計算 */
static void GetDirMat( FMATRIX *res, FMATRIX *pos, FVECTOR *target, FVECTOR *max, FVECTOR *min )
{
	FMATRIX	inv_mat ;
	FVECTOR	trg, rot, x, y ;
	float	l ;

	_sceVu0InversMatrix( &inv_mat, pos );
	target->vw = 1.0f ;
	_sceVu0ApplyMatrix( &trg, &inv_mat, target );
	l = trg.vx * trg.vx + trg.vz * trg.vz ;
	l = DG_SQRT( l );
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
	rot.vx = DG_MIN( rot.vx, max->vx );
	rot.vy = DG_MIN( rot.vy, max->vy );
	rot.vx = DG_MAX( rot.vx, min->vx );
	rot.vy = DG_MAX( rot.vy, min->vy );
	//printf("%f %f\n", rot.vx, rot.vy );
	/* マトリクスへ変換 */
	_sceVu0UnitMatrix( res );
	_sceVu0RotMatrixX( res, res, rot.vx );
	_sceVu0RotMatrixY( res, res, rot.vy );
}
/* ---------------------------------------------------------------- */
	/*
		キャラアクター関数
	*/
static void Act( Work *work )
{
//	int		n_msg, i ;
//	GV_MSG	*msg ;
	DG_EVMOBJ	*evmobj ;
	
	if( !DM_GetEftControl( work->target_name ) ){
		GV_DestroyActor( work );
		return  ;
	}
	if ( work->evmobj == NULL ) return ;
	evmobj = work->evmobj ;

	{/* 視線制御処理 */
		FMATRIX		mat, eye_mat ;
		FVECTOR		trans ;
		EVM_SKEL	*l_eye_skel, *r_eye_skel ;
//		float		t ;

		/* 眼球用スケルトン取得 */
		l_eye_skel = &evmobj->def->skeleton[ JOINT_L_EYE ] ;
		r_eye_skel = &evmobj->def->skeleton[ JOINT_R_EYE ] ;

		trans.vw = 1.0f ;
		/* 視線方向絶対座標を求める */
#if 1
		work->eye_target = *(FVECTOR*)work->target_pos ;
#else
		work->eye_target.vx = DG_Chanl(0)->eye.m[3][0] ;
		work->eye_target.vy = DG_Chanl(0)->eye.m[3][1] ;
		work->eye_target.vz = DG_Chanl(0)->eye.m[3][2] ;
#endif

		/* 左目の位置のマトリクスを求める */
		mat = evmobj->matrix[ evmobj->use_buffer ][ l_eye_skel->parent ] ;
		trans.vx = l_eye_skel->rt_tx ;
		trans.vy = l_eye_skel->rt_ty ;
		trans.vz = l_eye_skel->rt_tz ;
		_sceVu0ApplyMatrix( (FVECTOR*)&mat.m[3][0], &mat, &trans );
		/* 左目のマトリクスを算出 */
		GetDirMat( &eye_mat, &mat, &work->eye_target, &work->leye_max, &work->leye_min );
		_sceVu0MulMatrix( &eye_mat, &mat, &eye_mat );
		trans.vx = -l_eye_skel->rt_tx ;
		trans.vy = -l_eye_skel->rt_ty ;
		trans.vz = -l_eye_skel->rt_tz ;
		_sceVu0ApplyMatrix( (FVECTOR*)eye_mat.m[3], &eye_mat, &trans );
		evmobj->matrix[ evmobj->use_buffer ][ JOINT_L_EYE ] = eye_mat ;

		/* 右目の位置のマトリクスを求める */
		mat = evmobj->matrix[ evmobj->use_buffer ][ r_eye_skel->parent ] ;
		trans.vx = r_eye_skel->rt_tx ;
		trans.vy = r_eye_skel->rt_ty ;
		trans.vz = r_eye_skel->rt_tz ;
		_sceVu0ApplyMatrix( (FVECTOR*)&mat.m[3][0], &mat, &trans );
		/* 右目のマトリクスを算出 */
		GetDirMat( &eye_mat, &mat, &work->eye_target, &work->reye_max, &work->reye_min );
		_sceVu0MulMatrix( &eye_mat, &mat, &eye_mat );
		trans.vx = -r_eye_skel->rt_tx ;
		trans.vy = -r_eye_skel->rt_ty ;
		trans.vz = -r_eye_skel->rt_tz ;
		_sceVu0ApplyMatrix( (FVECTOR*)eye_mat.m[3], &eye_mat, &trans );
		evmobj->matrix[ evmobj->use_buffer ][ JOINT_R_EYE ] = eye_mat ;

	}


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
static int GetResources_P( Work *work, int name, OBJECT *body, int target_name,
						   FVECTOR *rmax, FVECTOR *rmin, FVECTOR *lmax, FVECTOR *lmin )
{
//	CONTROL	*ctrl = SearchControl( target_name, 0);
//	if(!ctrl) return -1;

	EFTCONTROL *ctrl = DM_GetEftControl( target_name );
	if(!ctrl) return -1;

	work->name = name ;
	work->target_name = target_name;
	work->evmobj = body->evmobj ;
	//work->target_pos = target ;
	work->target_pos = &ctrl->mov;
	work->leye_max = *lmax ;
	work->leye_min = *lmin ;
	work->reye_max = *rmax ;
	work->reye_min = *rmin ;

	return ( 0 );
}

/* ---------------------------------------------------------------- */
	/*
		プログラムインターフェイス
	*/
void *NewEyeControl2( int name, OBJECT *body, int target_name,
					  FVECTOR *rmax, FVECTOR *rmin, FVECTOR *lmax, FVECTOR *lmin, int flag )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_AFTER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources_P( work, name, body, target_name, rmax, rmin, lmax, lmin ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
