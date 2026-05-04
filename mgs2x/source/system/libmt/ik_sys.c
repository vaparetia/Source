//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ik_sys.c
	インバースキネマティクス基本ルーチン

	2000/10/14 K.Takabe
	$Id: ik_sys.c,v 1.4 2002/11/23 11:49:51 Yoshizawa1 Exp $

*/
/*









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

#include	"libgv.h"
#include	"libdg.h"
#include	"libmt.h"
#include	"utl_dma.h"
#include	"def_dma.h"


/* ---------------------------------------------------------------- */
typedef struct _scrpad_work {
	FMATRIX		root ;
	FMATRIX		matrix[ 32 ];		/* ＩＫは最大３２関節までとする */
	FVECTOR		rot[ 32 ];
	FVECTOR		trans[ 32 ];
	float		weight[ 32 ];
} ScrpadWork ;

/* ---------------------------------------------------------------- */
	/*
		ＩＫコントロールの作成
	*/
MT_IK_CONTROL* MT_MakeIKControl( int flag, int n_joints, DG_OBJS *objs )
{
	MT_IK_CONTROL	*ik_ctrl ;
	MT_IK_JOINT		*ik_joint ;
	int				i, size ;

	/* メモリ確保（ＩＫ関節数＋１分ワークを確保） */
	size = sizeof(MT_IK_CONTROL) + sizeof(MT_IK_JOINT) * ( n_joints + 1 ) ;
	ik_ctrl = GV_Malloc( size );
	ASSERT( ik_ctrl != NULL );
	if ( ik_ctrl == NULL ) return ( NULL );

	/* 初期設定 */
	GV_ZeroMemory( ik_ctrl, size );
	ik_ctrl->flag = flag ;
	ik_ctrl->n_joints = n_joints ;
	ik_ctrl->n_loop = 1 ;
	ik_ctrl->objs = objs ;

	ik_joint = ik_ctrl->joints ;
	for ( i = n_joints ; i >= 0 ; i--, ik_joint++ ){
		ik_joint->rotate = DG_ZeroVector ;
		ik_joint->weight = 1.0f ;
	}

	return ( ik_ctrl );
}

	/*
		ＩＫコントロール破棄
	*/
void MT_FreeIKControl( MT_IK_CONTROL *ik_ctrl )
{
	GV_Free( ik_ctrl );
}

/* ---------------------------------------------------------------- */
	/*
		関節情報を設定（パラメータ直接指定）
	*/
void MT_ConfigIKJointObj( MT_IK_CONTROL *ik_ctrl, int n_joint, int obj_num )
{
	MT_IK_JOINT		*ik_joint ;

	if ( ik_ctrl == NULL ) return ;
	ik_joint = &ik_ctrl->joints[ n_joint ] ;
	ik_joint->trans = ik_ctrl->objs->objs[ obj_num ].trans ;
	ik_joint->trans.vw = 1.0f ;	/* 念のため */
	ik_joint->joint_num = obj_num ;

}
void MT_ConfigIKJointParam( MT_IK_CONTROL *ik_ctrl, int n_joint, FVECTOR *max, FVECTOR *min )
{
	MT_IK_JOINT		*ik_joint ;

	if ( ik_ctrl == NULL ) return ;
	ik_joint = &ik_ctrl->joints[ n_joint ] ;
	ik_joint->max = *max ;
	ik_joint->min = *min ;
}
void MT_ConfigIKJointParamX( MT_IK_CONTROL *ik_ctrl, int n_joint, float max, float min )
{
	MT_IK_JOINT		*ik_joint ;

	if ( ik_ctrl == NULL ) return ;
	ik_joint = &ik_ctrl->joints[ n_joint ] ;
	ik_joint->max.vx = max ;
	ik_joint->min.vx = min ;
}
void MT_ConfigIKJointParamY( MT_IK_CONTROL *ik_ctrl, int n_joint, float max, float min )
{
	MT_IK_JOINT		*ik_joint ;

	if ( ik_ctrl == NULL ) return ;
	ik_joint = &ik_ctrl->joints[ n_joint ] ;
	ik_joint->max.vy = max ;
	ik_joint->min.vy = min ;
}
void MT_ConfigIKJointParamZ( MT_IK_CONTROL *ik_ctrl, int n_joint, float max, float min )
{
	MT_IK_JOINT		*ik_joint ;

	if ( ik_ctrl == NULL ) return ;
	ik_joint = &ik_ctrl->joints[ n_joint ] ;
	ik_joint->max.vz = max ;
	ik_joint->min.vz = min ;
}




/* ---------------------------------------------------------------- */
/* 間接情報を元にＩＫ計算を行う */
void MT_CalcInversKinematic( MT_IK_CONTROL *ik_ctrl )
{
	//static FVECTOR	zero_vec = {0,0,0,1};
	int		i, j, k, n_joints ;
	FVECTOR		dir1, dir2, quat, from, to, euler ;
	//FVECTOR		tmp_vec ;
	FMATRIX		*matrix, *top_matrix, inv_mat ;
	FVECTOR		*rotate, *trans ;
	float		*weight ;
	ScrpadWork	*scrpad = (ScrpadWork *) SCRPAD_ADDR ;
#ifdef KP_WINDOWS
	MT_IK_JOINT		*joints;			/* ＩＫ関節情報 */
#endif

	if ( ik_ctrl == NULL ) return ;
	n_joints = ik_ctrl->n_joints ;
	/* マトリクスの再計算処理 */
	scrpad->root = ik_ctrl->base_world ;
	matrix = scrpad->matrix ;
	rotate = scrpad->rot ;
	trans = scrpad->trans ;
	weight = scrpad->weight ;
#ifndef KP_WINDOWS
	for ( i = 0 ; i <= n_joints ; i++ ){
		/* その関節のマトリクスを計算 */
		*rotate = ik_ctrl->joints[i].rotate ;
		*trans = ik_ctrl->joints[i].trans ;
		*weight = ik_ctrl->joints[i].weight ;

		MT_QuatToMat( matrix, rotate );
		*(FVECTOR*)matrix->m[3] = *trans ;

		_sceVu0MulMatrix( matrix, matrix-1, matrix );

		rotate++ ;
		trans++ ;
		matrix++ ;
		weight++ ;
	}
#else
	joints = ik_ctrl->joints ;
	for ( i = n_joints + 1 ; i > 0 ; i-- ){
		/* その関節のマトリクスを計算 */
		*rotate = joints->rotate ;
		*trans  = joints->trans ;
		*weight = joints->weight ;

		MT_QuatToMat( matrix, rotate );
		*(FVECTOR*)matrix->m[3] = *trans ;

		_sceVu0MulMatrix( matrix, matrix-1, matrix );

		rotate++ ;
		trans++ ;
		matrix++ ;
		weight++ ;
		joints++ ;
	}
#endif
	top_matrix = &scrpad->matrix[ n_joints ] ;	/* 一番先端のマトリクス */
#ifndef KP_WINDOWS
	for ( i = 0 ; i < ik_ctrl->n_loop ; i++ ){
#else
	for ( i = ik_ctrl->n_loop ; i > 0 ; i-- ){
#endif
		/* 先端から根元に向かって順番に方向修正を行っていく */
		matrix = &scrpad->matrix[ n_joints - 1 ] ;
		rotate = &scrpad->rot[ n_joints - 1 ] ;
		trans = &scrpad->trans[ n_joints - 1 ] ;
		weight = &scrpad->weight[ n_joints - 1 ] ;
#ifndef KP_WINDOWS
		for ( j = n_joints - 1 ; j >= 0 ; j--, matrix--, rotate--, trans--, weight-- ){
#else
		joints = &ik_ctrl->joints[n_joints - 1] ;
		for ( j = n_joints - 1 ; j >= 0 ; j--, matrix--, rotate--, trans--, weight--, joints-- ){
#endif
			/* 現在の間接から先端までの方向ベクトルを求める */
			_sceVu0SubVector( &dir1,
							 (FVECTOR*)&top_matrix->m[3],
							 (FVECTOR*)&matrix->m[3] );
			/* 現在の間接から目標座標までの方向ベクトルを求める */
			_sceVu0SubVector( &dir2,
							 &ik_ctrl->target_pos,
							 (FVECTOR*)&matrix->m[3] );
			/* その関節からの相対座標を計算 */
			_sceVu0InversMatrix( &inv_mat, matrix );
			dir1.vw = 0.0f ;
			dir2.vw = 0.0f ;
			_sceVu0ApplyMatrix( &dir1, &inv_mat, &dir1 );
			_sceVu0ApplyMatrix( &dir2, &inv_mat, &dir2 );

			/* 正規化したベクトルを生成 */
			from = dir1 ;
			to = dir2 ;
			_sceVu0Normalize( &from, &from );
			_sceVu0Normalize( &to, &to );
			/* 変化量クォータニオン計算 */
			MT_QuatSetFromAx( &quat, &from, &to );
			/* クォータニオンの合成 */
			if ( *weight != 1.0f ){
				FVECTOR	tmp_quat ;
				MT_QuatMul( &tmp_quat, rotate, &quat );
				MT_QuatSlerp( &quat, &quat, &tmp_quat, *weight );	/* 重みに応じて補正量を減衰させる */
				MT_QuatNormalize( &quat, &quat );
				*weight = 1.0f ;
			} else {
				MT_QuatMul( &quat, rotate, &quat );
			}

#if 0
			/* ベース角度と変化角度に分離 */
			//MT_QuatInverse( &tmp_vec, &joints[j].base );
			MT_QuatInverse( &tmp_vec, &DG_ZeroVector );
			MT_QuatMul( &tmp_vec, &quat, &tmp_vec );

			/* 角度制限チェック */
			MT_QuatToEulerXZY( &euler, &tmp_vec );
			euler.vx = DG_MIN( euler.vx, ik_ctrl->joints[j].max.vx );
			euler.vy = DG_MIN( euler.vy, ik_ctrl->joints[j].max.vy );
			euler.vz = DG_MIN( euler.vz, ik_ctrl->joints[j].max.vz );
			euler.vx = DG_MAX( euler.vx, ik_ctrl->joints[j].min.vx );
			euler.vy = DG_MAX( euler.vy, ik_ctrl->joints[j].min.vy );
			euler.vz = DG_MAX( euler.vz, ik_ctrl->joints[j].min.vz );

			/* オイラー角からクォータニオンに戻す */
			MT_EulerToQuatXZY( &tmp_vec, &euler );

			/* ベース角度と変化角度を合成 */
			//MT_QuatMul( &quat, &tmp_vec, &joints[j].base );
			MT_QuatMul( &quat, &tmp_vec, &DG_ZeroVector );
#else
#if 1
			/* 角度制限チェック */
			MT_QuatToEulerXZY( &euler, &quat );
#ifndef KP_WINDOWS
			euler.vx = DG_MIN( euler.vx, ik_ctrl->joints[j].max.vx );
			euler.vy = DG_MIN( euler.vy, ik_ctrl->joints[j].max.vy );
			euler.vz = DG_MIN( euler.vz, ik_ctrl->joints[j].max.vz );
			euler.vx = DG_MAX( euler.vx, ik_ctrl->joints[j].min.vx );
			euler.vy = DG_MAX( euler.vy, ik_ctrl->joints[j].min.vy );
			euler.vz = DG_MAX( euler.vz, ik_ctrl->joints[j].min.vz );
#else
			euler.vx = DG_MIN( euler.vx, joints->max.vx );
			euler.vy = DG_MIN( euler.vy, joints->max.vy );
			euler.vz = DG_MIN( euler.vz, joints->max.vz );
			euler.vx = DG_MAX( euler.vx, joints->min.vx );
			euler.vy = DG_MAX( euler.vy, joints->min.vy );
			euler.vz = DG_MAX( euler.vz, joints->min.vz );
#endif

			/* オイラー角からクォータニオンに戻す */
			MT_EulerToQuatXZY( &quat, &euler );
#endif
#endif
			*rotate = quat ;
			
			{/* マトリクス再計算 */
				FMATRIX		*mat ;
				FVECTOR		*rot, *trn ;
				mat = matrix ;
				rot = rotate ;
				trn = trans ;
#ifndef KP_WINDOWS
				for ( k = j ; k <= n_joints ; k++ ){
#else
				for ( k = (n_joints-j)+1 ; k > 0 ; k-- ){
#endif
					/* その関節のマトリクスを計算 */
					MT_QuatToMat( mat, rot );
					*(FVECTOR*)mat->m[3] = *trn ;
					_sceVu0MulMatrix( mat, mat-1, mat );
					rot++ ;
					trn++ ;
					mat++ ;
				}
			}
		}
	}

	/* 結果の書き戻し */
	for ( i = 0 ; i < n_joints ; i++ ){
		ik_ctrl->joints[ i ].rotate = scrpad->rot[ i ] ;
	}

}
/* ---------------------------------------------------------------- */
