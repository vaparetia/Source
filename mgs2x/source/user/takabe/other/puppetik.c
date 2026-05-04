//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	puppetik.c
	ＩＫ実験プログラム

	2000/10/16 K.Takabe
	$Id: puppetik.c,v 1.1.1.3 2002/11/19 11:51:18 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <stdarg.h>
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
#include	"def_dma.h"
#include	"utl_dma.h"
#include	"gameheader.h"

#define PUPPETIK_SOURCE
#include	"puppetik.h"

#include "bp_matrix.h"

/* ---------------------------------------------------------------- */
#define SET_VEC( _v, _x, _y, _z, _w ) { (_v)->vx = _x ; (_v)->vy = _y ; (_v)->vz = _z ; (_v)->vw = _w ;  }
//static void PrintVec( char *header, FVECTOR *vec );
#if defined(PSX2)
#define RAND(_n)	( ( ( BP_PS2_rand() >> 16 ) * (_n) ) >> 15 )
#else
#define RAND(_n)	( ( rand() * (_n) ) >> 15 )
#endif

//#define CHECK_SEG_FLAG	(work->control->seg_flag)
#define CHECK_SEG_FLAG		(HZX_SEG_NO_ENEMY_IK|HZX_SEG_NO_ENEMY_EYES)

#define CHECK_FLOOR_FLAG	(HZX_FLOOR_IK)

/* ---------------------------------------------------------------- */
/* ---------------------------------------------------------------- */

/* ＩＫ計算用関節情報構造体 */
typedef struct _mt_joint_info2 {
	FMATRIX		matrix ;	/* 関節のマトリクス */
	FVECTOR		trans ;		/* 親からのオフセット座標 */
	FVECTOR		rotate ;	/* 親関節からの回転量（クォータニオン） */
	FVECTOR		base ;		/* ベースとなる回転量 */
	FVECTOR		max, min ;	/* 関節の最大・最小角度 */
	float		joint_len ;	/* 関節の長さ */
	int			total_joints ;		/* 最大関節数（配列先頭要素でのみ有効） */
	int			joint_parent_num ;	/* 割り当てられる関節の親の関節番号 */
	int			joint_num ;			/* 割り当てられる関節番号 */
} MT_JOINT_INFO2 ;

/* ＩＫターゲット管理構造体 */
typedef struct _trg_ctrl{
	FVECTOR		pos ;			/* 座標 */
	FVECTOR		piku_offset ;	/* ピク付き用 */
	FVECTOR		old_pos ;		/* 前回の座標 */
	FVECTOR		v_vec ;			/* 座標移動量 */
	FVECTOR		a_vec ;			/* 座標移動変化量 */
	float		last_h ;		/* 最後にチェックした床の座標 */
	float		max_len ;		/* 関節最大長 */
	int			pad[2] ;
} TRG_CTRL ;


typedef	struct	{
	MT_JOINT_INFO2	head[5] ;
	MT_JOINT_INFO2	left_arm[4] ;
	MT_JOINT_INFO2	right_arm[4] ;
	MT_JOINT_INFO2	left_leg[4] ;
	MT_JOINT_INFO2	right_leg[4] ;
	CONTROL		*control ;
	OBJECT		*object ;
	TRG_CTRL	trg_left_arm ;
	TRG_CTRL	trg_right_arm ;
	TRG_CTRL	trg_left_leg ;
	TRG_CTRL	trg_right_leg ;
	TRG_CTRL	trg_head ;

	FVECTOR		weist_base ;
	FVECTOR		r_leg_pos ;
	FVECTOR		l_leg_pos ;

	int			mode ;
	int			before_mode ;			/* 静止状態になる前のモード */
	int			flag ;
	int			time ;
	int			static_count ;			/* 静止カウント */
	int			static_flag ;			/* 静止フラグ */
	FVECTOR		last_ctrl_pos ;			/* 静止時の最終位置 */
	SVECTOR		last_ctrl_rot ;			/* 静止時の最終方向 */

	int			repulsion_time ;		/* 反発処理有効時間 */
	float		repulsion_len ;
	FVECTOR		repulsion_vec ;
	FVECTOR		repulsion_pos ;

} TAKABE_PuppetIK ;

enum {
	MODE_NO_ACTIVE				= 0,		/* ＩＫルーチン完全停止 */
	MODE_ACTIVE					= 1,		/* ＩＫ発動中 */
	MODE_STATIC					= 2,		/* 静止状態（モーションのポーズに対してのみＩＫを行う） */
	MODE_SLEEP					= 3,		/* モーションの設定のみ */
};

enum {
	FLAG_REQUEST_INIT_ROT		= 0x00000001,	/* 関節角度初期化要求 */
	FLAG_REQUEST_INIT_TRG		= 0x00000002,	/* ＩＫ基準点初期化要求 */
	FLAG_ACTIVE_RARM			= 0x00000100,	/*  */
	FLAG_ACTIVE_LARM			= 0x00000200,	/*  */
	FLAG_ACTIVE_RLEG			= 0x00000400,	/*  */
	FLAG_ACTIVE_LLEG			= 0x00000800,	/*  */
	FLAG_ACTIVE_HEAD			= 0x00001000,	/*  */
	FLAG_ENABLE_DEBUG			= 0x80000000,	/* デバッグの有効フラグ */
};



/* ---------------------------------------------------------------- */
#if 0 /* libmtへ移動済み */
#if 0
/* 間違えて作ってしまった */
void MT_EulerToQuatYZX( FVECTOR *quat, FVECTOR *rot)
{
	float cr, cp, cy, sr, sp, sy;
	FVECTOR	func_res, param ;

	param.vx = rot->vx / 2 ;
	param.vy = rot->vy / 2 ;
	param.vz = rot->vz / 2 ;
	MT_CosX4( &func_res, &param );
	cr = func_res.vx ;
	cp = func_res.vy ;
	cy = func_res.vz ;
	MT_SinX4( &func_res, &param );
	sr = func_res.vx ;
	sp = func_res.vy ;
	sy = func_res.vz ;

	quat->vw = cr * cp * cy - sr * sp * sy ;
	quat->vx = sr * cp * cy + cr * sp * sy ;
	quat->vy = sr * cp * sy + cr * sp * cy ;
	quat->vz = cr * cp * sy - sr * sp * cy ;
}
/* ほかから持ってきたソースを書き直しただけであっているかどうか保証なし */
void MT_QuatToEulerZYX( FVECTOR *euler, FVECTOR *quat )
{
	FMATRIX	matrix;
	FVECTOR	c_vec, s_vec ;
	float	div ;

	MT_QuatToMat( &matrix, quat );

	s_vec.vy = -matrix.m[2][0] ;
	c_vec.vy = DG_SQRT(1.0f - ( s_vec.vy * s_vec.vy ) ) ;

	/* ０除算回避 */
	if ( s_vec.vy != 1.0f && s_vec.vy != -1.0f ) {
		div = 1.0f / c_vec.vy ;
		c_vec.vx = matrix.m[2][2] * div ;
		s_vec.vx = matrix.m[2][1] * div ;

		c_vec.vz = matrix.m[0][0] * div ;
		s_vec.vz = matrix.m[1][0] * div ;
	} else {
		c_vec.vx = matrix.m[1][1] ;
		s_vec.vx = -matrix.m[1][2] ;

		c_vec.vz = 1.0f ;
		s_vec.vz = 0.0f ;
	}
	MT_Atan2X4( euler, &s_vec, &c_vec );
}
#endif
/* 関節ＩＫ用 */
void MT_QuatToEulerXZY( FVECTOR *euler, FVECTOR *quat )
{
	FMATRIX	matrix;
	FVECTOR	c_vec, s_vec ;
	float	div ;

	MT_QuatToMat( &matrix, quat );

	s_vec.vz = -matrix.m[1][0] ;
	c_vec.vz = DG_SQRT( 1.0f - s_vec.vz * s_vec.vz ) ;

	/* ０除算回避 */
	if ( s_vec.vz != 1.0f && s_vec.vz != -1.0f ) {
		div = 1.0f / c_vec.vz ;
		s_vec.vx = matrix.m[1][2] ;
		c_vec.vx = matrix.m[1][1] ;

		s_vec.vy = matrix.m[2][0] ;
		c_vec.vy = matrix.m[0][0] ;
	} else {
		c_vec.vx = matrix.m[0][1] ;
		s_vec.vx = -matrix.m[2][1] ;

		c_vec.vy = 1.0f ;
		s_vec.vy = 0.0f ;
	}
	MT_Atan2X4( euler, &s_vec, &c_vec );
}
void MT_EulerToQuatXZY( FVECTOR *quat, FVECTOR *rot)
{
	float cr, cp, cy, sr, sp, sy;
	FVECTOR	func_res, param ;

	param.vx = rot->vx / 2 ;
	param.vy = rot->vy / 2 ;
	param.vz = rot->vz / 2 ;
	MT_CosX4( &func_res, &param );
	cr = func_res.vx ;
	cp = func_res.vy ;
	cy = func_res.vz ;
	MT_SinX4( &func_res, &param );
	sr = func_res.vx ;
	sp = func_res.vy ;
	sy = func_res.vz ;

	quat->vw = cr * cp * cy + sr * sp * sy ;
	quat->vx = sr * cp * cy - cr * sp * sy ;
	quat->vy = cr * sp * cy - sr * cp * sy ;
	quat->vz = cr * cp * sy + sr * sp * cy ;
}
#endif


/* 間接情報を元にＩＫ計算を行う */
void MT_CalcInversKinematic2( MT_JOINT_INFO2 *joints, int n_joints, FVECTOR *target_pos )
{
	int		i, j, k ;
	FVECTOR		dir1, dir2, quat, from, to, euler, tmp_vec ;
	FMATRIX		mat, tmp_mat, inv_mat ;

	/* マトリクスの再計算処理 */
	mat = joints[0].matrix ;
	for ( i = 1 ; i < n_joints ; i++ ){
		joints[i].trans.vw = 1.0f ;	/* 念のため */
		/* その関節のマトリクスを計算 */
		MT_QuatToMat( &tmp_mat, &joints[i].rotate );
		*(FVECTOR*)tmp_mat.m[3] = joints[i].trans ;
		_sceVu0MulMatrix( &mat, &mat, &tmp_mat );
		joints[i].matrix = mat ;
	}

	for ( i = 0 ; i < 1 ; i++ ){
		/* 先端から根元に向かって順番に方向修正を行っていく */
		for ( j = n_joints - 2 ; j > 0 ; j-- ){
			/* 現在の間接から先端までの方向ベクトルを求める */
			_sceVu0SubVector( &dir1, (FVECTOR*)&joints[n_joints-1].matrix.m[3], (FVECTOR*)&joints[j].matrix.m[3] );
			/* 現在の間接から目標座標までの方向ベクトルを求める */
			_sceVu0SubVector( &dir2, target_pos, (FVECTOR*)&joints[j].matrix.m[3] );
			/* その関節からの相対座標を計算 */
			_sceVu0InversMatrix( &inv_mat, &joints[j].matrix );
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
			MT_QuatMul( &quat, &joints[j].rotate, &quat );

			/* ベース角度と変化角度に分離 */
			MT_QuatInverse( &tmp_vec, &joints[j].base );
			MT_QuatMul( &tmp_vec, &quat, &tmp_vec );

			/* 角度制限チェック */
			MT_QuatToEulerXZY( &euler, &tmp_vec );
			euler.vx = DG_MIN( euler.vx, joints[j].max.vx );
			euler.vy = DG_MIN( euler.vy, joints[j].max.vy );
			euler.vz = DG_MIN( euler.vz, joints[j].max.vz );
			euler.vx = DG_MAX( euler.vx, joints[j].min.vx );
			euler.vy = DG_MAX( euler.vy, joints[j].min.vy );
			euler.vz = DG_MAX( euler.vz, joints[j].min.vz );

			/* オイラー角からクォータニオンに戻す */
			MT_EulerToQuatXZY( &tmp_vec, &euler );

			/* ベース角度と変化角度を合成 */
			MT_QuatMul( &quat, &tmp_vec, &joints[j].base );
			joints[j].rotate = quat ;
			
			/* マトリクス再計算 */
			mat = joints[j-1].matrix ;
			for ( k = j ; k < n_joints ; k++ ){
				/* その関節のマトリクスを計算 */
				MT_QuatToMat( &tmp_mat, &joints[k].rotate );
				*(FVECTOR*)tmp_mat.m[3] = joints[k].trans ;
				_sceVu0MulMatrix( &mat, &mat, &tmp_mat );
				joints[k].matrix = mat ;
			}
		}
	}
}
/* ---------------------------------------------------------------- */
typedef struct {
	FVECTOR	root ;
	FVECTOR	joints[128] ;
	FVECTOR	quat_buffer[4] ;
	FVECTOR	vec ;
	FMATRIX	mat ;
	FMATRIX	root_mat ;
	FMATRIX	mats[128] ;
} ScrPadWork ;
static void ActMotion2( MOTION_CONTROL *m_ctrl, DG_OBJS *objs )
{
	ScrPadWork	*scrpad  = (ScrPadWork*)SCRPAD_ADDR ;

	scrpad->root_mat = objs->world ;

	MT_StartMemToSpr( scrpad->joints, m_ctrl->abs_rots, m_ctrl->n_joints );
	MT_WaitMemToSpr();

	{/* オブジェクトにマトリクスを設定する */
		FVECTOR			*joints = scrpad->joints ;
		FMATRIX			*mats = scrpad->mats ;
		DG_DEF	*def ;
		DG_MDL	*mdl ;
		DG_OBJ	*obj = objs->objs ;
		int		i ;

		def = objs->def ;
		scrpad->vec.vw = 1.0F ;
		{/* モデル情報から親子関係を取得して求める */
			for ( i = def->n_models ; i > 0 ; i-- ){	/* 拡張モデルは無視する */
				FMATRIX	*parent ;
				mdl = obj->model ;
				MT_QuatToMat( mats, joints );
				_sceVu0MulMatrix( mats, &scrpad->root_mat, mats ) ;
				scrpad->vec.vx = mdl->tx ;
				scrpad->vec.vy = mdl->ty ;
				scrpad->vec.vz = mdl->tz ;
				parent = &scrpad->mats[ mdl->parent ] ;
				_sceVu0ApplyMatrix( &mats->m[3][0], parent, &scrpad->vec );
				obj->world = *mats ;
				obj++ ;
				mats++ ;
				joints++ ;
			}
		}
	}

}
static void EvmActMotion( MOTION_CONTROL *m_ctrl, DG_EVMOBJ *evmobj )
{
	ScrPadWork	*scrpad  = (ScrPadWork*)SCRPAD_ADDR ;
	FMATRIX		*skel_mats ;

	if ( evmobj->root != NULL )	scrpad->root_mat = *( evmobj->root );
	else						scrpad->root_mat = evmobj->world ;
	skel_mats = evmobj->matrix[ evmobj->use_buffer ] ;

	UTL_StartMemToSpr( scrpad->joints, m_ctrl->abs_rots, m_ctrl->n_joints );
	UTL_EndMemToSpr();

	{/* オブジェクトにマトリクスを設定する */
		FVECTOR			*joints = scrpad->joints ;
		FMATRIX			*mats = scrpad->mats ;
		EVM_SKEL		*skel ;
		int		i ;

		skel = evmobj->def->skeleton ;
		scrpad->vec.vw = 1.0F ;
		{/* モデル情報から親子関係を取得して求める */
			for ( i = 0 ; i < evmobj->n_skeleton ; i++ ){
				FMATRIX	*parent ;
				if ( i < m_ctrl->n_joints ){
					MT_QuatToMat( mats, joints );
					_sceVu0MulMatrix( mats, &scrpad->root_mat, mats ) ;
					scrpad->vec.vx = skel->rt_tx + 0.0f ;
					scrpad->vec.vy = skel->rt_ty + 0.0f ;
					scrpad->vec.vz = skel->rt_tz + 0.0f ;
					parent = &scrpad->mats[ skel->parent ] ;
					_sceVu0ApplyMatrix( &mats->m[3][0], parent, &scrpad->vec );
					scrpad->vec.vx = -skel->rt_tx ;
					scrpad->vec.vy = -skel->rt_ty ;
					scrpad->vec.vz = -skel->rt_tz ;
					_sceVu0ApplyMatrix( &mats->m[3][0], mats, &scrpad->vec );
					*skel_mats = *mats ;
				} else {
					*mats = *skel_mats = scrpad->mats[ skel->parent ] ;
				}
				skel++ ;
				mats++ ;
				skel_mats++ ;
				joints++ ;
			}
		}
	}

}
/* ---------------------------------------------------------------- */
#if 0
/* デバッグ用 */
static void PrintVec( char *header, FVECTOR *vec )
{
	float	len ;
	len = bp_sqrtf( vec->vx * vec->vx + vec->vy * vec->vy + vec->vz * vec->vz );  //BP_MATH - emulate PS2 sqrtf
	printf("%s %f %f %f %f (%f)\n", header, vec->vx, vec->vy, vec->vz, vec->vw, len );
	
}
/* デバッグ用 */
static void PrintQuat( char *header, FVECTOR *quat )
{
	FVECTOR	tmp ;
	MT_QuatGetValue( &tmp, quat );
	tmp.vw = tmp.vw * 180.0f / 3.14159265f ;
	printf("%s %f %f %f %f\n", header, tmp.vx, tmp.vy, tmp.vz, tmp.vw );
	MT_QuatToEulerXZY( &tmp, quat );
	printf("     (euler: %f %f %f)\n",
		   tmp.vx * 180.0f / (float)M_PI, tmp.vy * 180.0f / (float)M_PI, tmp.vz * 180.0f / (float)M_PI );
}
static void PrintMat( char *header, FMATRIX *mat )
{
	printf("%s\n", header );
	printf("%f %f %f %f \n", mat->m[0][0], mat->m[1][0], mat->m[2][0], mat->m[3][0] );
	printf("%f %f %f %f \n", mat->m[0][1], mat->m[1][1], mat->m[2][1], mat->m[3][1] );
	printf("%f %f %f %f \n", mat->m[0][2], mat->m[1][2], mat->m[2][2], mat->m[3][2] );
	printf("%f %f %f %f \n", mat->m[0][3], mat->m[1][3], mat->m[2][3], mat->m[3][3] );
	
}
#endif
/* ---------------------------------------------------------------- */
/* ベクトルの長さを取得 */
static float GetPosLength( FVECTOR *v1, FVECTOR *v2 )
{
	FVECTOR		v ;
	float		len ;
	_sceVu0SubVector( &v, v1, v2 );
	len = v.vx * v.vx + v.vy * v.vy + v.vz * v.vz ;
	return ( DG_SQRT( len ) );
}
/* ---------------------------------------------------------------- */
static void GetJointPos( FVECTOR *pos, DG_OBJS *objs, int n_joint )
{
	*pos = *(FVECTOR*)objs->objs[ n_joint ].world.m[3] ;
}
/* ---------------------------------------------------------------- */
/* 仮想座標と実座標が指定した距離以上離れている場合には仮想座標位置を補正する */
static int PositionCorrect( FVECTOR *v_pos, FVECTOR *r_pos, float range )
{
	FVECTOR	vec ;
	float	len ;
	_sceVu0SubVector( &vec, v_pos, r_pos );
	len = vec.vx * vec.vx + vec.vy * vec.vy + vec.vz * vec.vz ;
	if ( len > ( range * range ) ){
		*v_pos = *r_pos ;
		return ( 1 );
	}
	return ( 0 );
}
/* 指定座標のＹ座標での床の高さを求める */
static float CalcFloorLevel( HZX_FLR	*flr, FVECTOR *pos )
{
	FVECTOR		v, n, tmp_vec ;
	float		len, add_y ;
	v.vx = flr->p1.x ; v.vy = flr->p1.y ; v.vz = flr->p1.z ;	/* 床上の任意の座標 */
	n.vx = flr->p1.h ; n.vy = flr->p3.h ; n.vz = flr->p2.h ;	/* 床の法線 */
	_sceVu0Normalize( &n, &n );
	/* 床までの距離を求める */
	_sceVu0SubVector( &tmp_vec, &v, pos );
	len = tmp_vec.vx * n.vx + tmp_vec.vy * n.vy + tmp_vec.vz * n.vz ;
	add_y = len / n.vy ;	/* = len / ( dir * n ) */
#if 0
	vec.vx = pos->vx ;
	vec.vy = pos->vy + add_y ;
	vec.vz = pos->vz ;
#endif
	return ( pos->vy + add_y + 1.0f );
}
/* ---------------------------------------------------------------- */
/* 関節情報設定 */
static void SetJointInfo( MT_JOINT_INFO2 *joint, int joint_num, DG_OBJS *objs,
						 int max_x, int min_x, int max_y, int min_y, int max_z, int min_z )
{
	joint->joint_num = joint_num ;
	joint->joint_parent_num = objs->def->models[joint_num].parent ;
	/* 一応クォータニオンを初期化しておく */
	joint->rotate = DG_ZeroVector ;
	joint->base = DG_ZeroVector ;
	/* 関節のオフセットを設定 */
	joint->trans.vx = objs->def->models[joint_num].tx ;
	joint->trans.vy = objs->def->models[joint_num].ty ;
	joint->trans.vz = objs->def->models[joint_num].tz ;
	joint->trans.vw = 1.0f ;
	/* 関節長さ設定 */
	joint->joint_len = DG_SQRT( joint->trans.vx * joint->trans.vx +
							   joint->trans.vy * joint->trans.vy + joint->trans.vz * joint->trans.vz );
	/* 回転角度の最大・最小角度をラジアンで設定 */
	joint->max.vx =  max_x / 180.0f * 3.14159265f ;
	joint->max.vy =  max_y / 180.0f * 3.14159265f ;
	joint->max.vz =  max_z / 180.0f * 3.14159265f ;
	joint->min.vx =  min_x / 180.0f * 3.14159265f ;
	joint->min.vy =  min_y / 180.0f * 3.14159265f ;
	joint->min.vz =  min_z / 180.0f * 3.14159265f ;
}
/* ＩＫターゲットの初期化 */
static void InitIkTarget( TAKABE_PuppetIK *work, TRG_CTRL *trg, FVECTOR *pos )
{
	FVECTOR		ctrl_pos ;

	/* 階段の突き抜けを考慮し、オフセットを加えたものを腰の中心とみなす */
	ctrl_pos = work->control->mov ;
	ctrl_pos.vy += 250.0f ;
	/* 座標の設定 */
	trg->pos = *pos ;
	/* 壁突き抜けチェック */
	if ( HZX_OnlineHazardCheck( work->control->hzx_id, &ctrl_pos, &trg->pos
							   , HZX_CHK_ALL, CHECK_SEG_FLAG, CHECK_FLOOR_FLAG ) ){
		HZX_GetOnlinePoint( &trg->pos );
	}

	trg->old_pos = trg->pos ;
	/* 加速度の初期化（適当） */
	trg->a_vec.vy = -2.0f ;
	/* 最終タッチフロアの高さを設定 */
	HZX_LevelHazardCheck( work->control->hzx_id, &trg->pos,
						 HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR, CHECK_FLOOR_FLAG );
	trg->last_h = HZX_GetFloorLevel();
}
/* ＩＫ関節の初期化 */
static void InitIkJoint( TAKABE_PuppetIK *work, MT_JOINT_INFO2 *joint )
{
	MOTION_CONTROL	*m_ctrl ;
	FVECTOR			local_quat, parent_quat, inv_quat ;
	int		i, p ;

	m_ctrl = work->object->m_ctrl ;
	for ( i = 1 ; i < joint[0].total_joints - 1 ; i++ ){
		/* 絶対回転クォータニオンからローカルクォータニオンを生成 */
		if ( ( p = joint[i].joint_parent_num ) != -1 ){
			parent_quat = m_ctrl->abs_rots[ p ] ;
			MT_QuatInverse( &inv_quat, &parent_quat );
			MT_QuatMul( &local_quat, &inv_quat, &m_ctrl->abs_rots[ joint[i].joint_num ] );
		} else {
			local_quat = m_ctrl->abs_rots[ joint[i].joint_num ];
		}
		joint[i].rotate = local_quat ;
		joint[i].base = local_quat ;
	}
}

/* ＩＫ計算ルーチンを呼び出す */
static void CalcIK( MT_JOINT_INFO2 *joint, FVECTOR *trg_pos, DG_OBJS *objs, int loop )
{
	int			i ;

	joint[0].matrix = objs->objs[joint[0].joint_num].world ;
	for ( i = 0 ; i < loop ; i++ ){
		MT_CalcInversKinematic2( joint, joint[0].total_joints, trg_pos );
	}
}

/* ＩＫ結果をモーションにフィードバックさせる */
static void FreedbackJoint( TAKABE_PuppetIK *work, MT_JOINT_INFO2 *joint )
{
	MOTION_CONTROL	*m_ctrl ;
	int		i ;

	m_ctrl = work->object->m_ctrl ;
	for ( i = 1 ; i < joint[0].total_joints - 1 ; i++ ){
		MT_QuatMul( &m_ctrl->abs_rots[joint[i].joint_num],
				   &m_ctrl->abs_rots[joint[i].joint_parent_num], &joint[i].rotate );
	}
}


/* ＩＫ起点ターゲットの制御処理 */
static void CheckIkTarget3( TAKABE_PuppetIK *work, MT_JOINT_INFO2 *joint, TRG_CTRL *trg, int type )
{
	FVECTOR		vec, old_vec, near_react_vec, check_center, motion_pos ;
	int		total_joints, near_flag = 0, n_touches, online_flag = 0, res ;
	int		chk_flag, seg_flag, flr_flag ;		/* online check param */
	float	h, r_sphere ;
	//float	len, near_size ;
	//int		i ;

	total_joints = joint[0].total_joints ;

	/* 新ＩＫ起点の更新 */
	if ( work->mode == MODE_ACTIVE ){
		/* 通常ＩＫ時 */
		vec = trg->old_pos ;	/* 前回のＩＫ適用後の座標 */
		//chk_flag = HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT ;
		chk_flag = HZX_CHK_ALL ;
		seg_flag = CHECK_SEG_FLAG ;
		flr_flag = CHECK_FLOOR_FLAG ;
	} else {
		/* 待機状態時 */
		GetJointPos( &vec, work->object->objs, joint[total_joints-1].joint_num );/* 現在のＩＫ適用前の座標 */
		_sceVu0AddVector( &vec, &vec, &trg->piku_offset );	/* ピク付き用 */
		trg->piku_offset.vx *= 0.8f ;						/* ピク付き用減衰 */
		trg->piku_offset.vy *= 0.8f ;						/* ピク付き用減衰 */
		trg->piku_offset.vz *= 0.8f ;						/* ピク付き用減衰 */
		chk_flag = HZX_CHK_ALL ;
		seg_flag = CHECK_SEG_FLAG ;
		//flr_flag = HZX_FLOOR_ALL ;
		flr_flag = CHECK_FLOOR_FLAG ;
	}

	/* 最終チェック床からの変化チェック */
	if ( vec.vy < ( trg->last_h + 100.0f ) &&
		vec.vy > ( trg->last_h - 1000.0f ) ) vec.vy = trg->last_h + 100.0f ;
	/* 壁抜けのチェック */
	check_center = work->control->mov ;
	motion_pos = vec ;
	if ( HZX_OnlineHazardCheck( work->control->hzx_id, &check_center, &vec, 
							   chk_flag, seg_flag, flr_flag ) ){
		old_vec = vec ;
		if ( HZX_GetOnlineHazardType() == 1 ){
			/* 壁とぶつかっていた場合 */
			online_flag = 1 ;
			HZX_GetOnlinePoint( &vec );
			vec.vx += ( vec.vx > old_vec.vx ) ? 8.0f : -8.0f ;
			vec.vy += ( vec.vy > old_vec.vy ) ? 2.0f : -2.0f ;
			vec.vz += ( vec.vz > old_vec.vz ) ? 8.0f : -8.0f ;
		} else {
			/* 床とぶつかっていた場合 */
			HZX_FLR	flr ;
			int		atr ;
			if ( work->mode == MODE_ACTIVE && type == 1 ){
				/* 引きずり時の上半身場合にはさらに詳しくチェック */
				/* まずは胸でチェック */
				GetJointPos( &vec, work->object->objs, 2 );
				//vec = *(FVECTOR*)joint[2].matrix.m[3] ;/* 前回の胸の位置に相当 */
				if ( HZX_OnlineHazardCheck( work->control->hzx_id, &check_center, &vec, 
										   chk_flag, seg_flag, flr_flag ) ){
					if ( HZX_GetOnlineHazardType() == 1 ){
						HZX_GetOnlinePoint( &vec );
						vec.vx += ( vec.vx > old_vec.vx ) ? 8.0f : -8.0f ;
						vec.vy += ( vec.vy > old_vec.vy ) ? 2.0f : -2.0f ;
						vec.vz += ( vec.vz > old_vec.vz ) ? 8.0f : -8.0f ;
					} else {
						HZX_GetOnlineHazard( &flr, &atr );
						vec.vy = CalcFloorLevel( &flr, &vec ) + 51.0f ;
					}
				}
				/* 次は胸の位置から頭の位置をチェック */
				check_center = vec ;
				//GetJointPos( &vec, work->object->objs, 12 );
				vec = trg->old_pos ;	/* 前回の頭の位置 */
				if ( HZX_OnlineHazardCheck( work->control->hzx_id, &check_center, &vec, 
										   chk_flag, seg_flag, flr_flag ) ){
					if ( HZX_GetOnlineHazardType() == 1 ){
						HZX_GetOnlinePoint( &vec );
						vec.vx += ( vec.vx > old_vec.vx ) ? 8.0f : -8.0f ;
						vec.vy += ( vec.vy > old_vec.vy ) ? 2.0f : -2.0f ;
						vec.vz += ( vec.vz > old_vec.vz ) ? 8.0f : -8.0f ;
					} else {
						HZX_GetOnlineHazard( &flr, &atr );
						vec.vy = CalcFloorLevel( &flr, &vec ) + 51.0f ;
					}
				}
			} else {
				HZX_GetOnlineHazard( &flr, &atr );
				vec.vy = CalcFloorLevel( &flr, &vec ) + 51.0f ;
			}
		}
	}

#if 0
	/* 隣接壁チェック */
	if ( type == 1 )	r_sphere = 200 ;	/* 頭の場合 */
	else				r_sphere = 150 ;	/* 手足の場合 */
	if ( n_touches = HZX_NearHazardCheck( work->control->hzx_id, &vec ,
										 200, /* 検出半径 */
										 HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT, seg_flag,
										 r_sphere /* 反発半径 */) ){
		FVECTOR		vecs[2] ;
		//HZX_GetReactVector(&near_react_vec);/* 廃止された関数 */
		HZX_GetNearVector( vecs );				/* 隣接する壁の反射ベクトル取得 */
		GM_GetHazardReaction( &near_react_vec, n_touches, vecs, r_sphere /* 反発半径 */ ) ;
		_sceVu0AddVector( &vec, &vec, &near_react_vec );
		near_flag = 1 ;
		/* 上半身のときのみめり込み対策 */
		if ( online_flag && type == 1  && work->mode == MODE_STATIC ){
			vec.vy += GetPosLength( &vec, &old_vec );
		}

	}

	/* 不要なブレを抑えるため一定距離以内であればＩＫ基準点の補正を行わない */
	if ( PositionCorrect( &trg->pos, &vec, 20.0f ) == 0 ){
		/* 関節は動いていない */
		work->static_flag |= 1 ;
	}
#if 0
	/* あたり判定によるキャラクタ移動量チェック */
	if ( work->mode == MODE_STATIC && near_flag ){
		if ( GetPosLength( &trg->old_pos, &trg->pos ) > 2.0f ){
			/* 反発力の計算 */
			printf("a\n");
			len = near_react_vec.vx * near_react_vec.vx + near_react_vec.vz * near_react_vec.vz ;/* 高さは考慮しない */
			if ( work->repulsion_len < len ){
				work->repulsion_len = len ;				/* 反発力 */
				work->repulsion_vec = near_react_vec ;	/* 反発ベクトル */
				work->repulsion_pos = trg->pos ;		/* 反発ポイント */
				printf("b\n");
			}
		}
	}
#endif

	/* 等速落下のみ対応 */
	trg->pos.vy -= 50.0f ;

	/* 床チェック */
	HZX_LevelHazardCheck( work->control->hzx_id, &trg->pos,
						 HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR, CHECK_FLOOR_FLAG);
	h = HZX_GetFloorLevel();
	if ( trg->pos.vy < ( h + 100.0f ) ){
		trg->pos.vy = h + 100.0f ;		/* 自由落下による突き抜けを防止するために少し浮かせる */
	}
	trg->last_h = h ;
#else
	/* 隣接壁チェック */
	if ( type == 1 )	r_sphere = 200 ;	/* 頭の場合 */
	else				r_sphere = 150 ;	/* 手足の場合 */
	if ( ( n_touches = HZX_NearHazardCheck( work->control->hzx_id, &vec ,
										 200, /* 検出半径 */
										 HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT, seg_flag,
										 r_sphere /* 反発半径 */) ) ){
		FVECTOR		vecs[2] ;
		//HZX_GetReactVector(&near_react_vec);/* 廃止された関数 */
		HZX_GetNearVector( vecs );				/* 隣接する壁の反射ベクトル取得 */
		GM_GetHazardReaction( &near_react_vec, n_touches, vecs, r_sphere /* 反発半径 */ ) ;
		_sceVu0AddVector( &vec, &vec, &near_react_vec );
		near_flag = 1 ;
	}

	/* 等速落下のみ対応 */
	if ( work->mode == MODE_ACTIVE ){
		/* 通常ＩＫ時のみ落下処理を入れる */
		vec.vy -= 50.0f ;
	}

	/* 床チェック */
	res = HZX_LevelHazardCheck( work->control->hzx_id, &vec,
							   HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR, CHECK_FLOOR_FLAG);
	if ( res != 0 ){
		h = HZX_GetFloorLevel();
		if ( vec.vy < ( h + 100.0f ) ){
			vec.vy = h + 100.0f ;		/* 自由落下による突き抜けを防止するために少し浮かせる */
		}
		trg->last_h = h ;
	}

	/* 上半身のときのみめり込み対策 */
	if ( near_flag /* && online_flag */ && type == 1  && work->mode == MODE_STATIC ){
#if 0
		vec.vy += GetPosLength( &vec, &old_vec );
#else
		float	now_len, org_len ;
		org_len = GetPosLength( &check_center, &motion_pos );
		now_len = GetPosLength( &check_center, &vec );
		if ( org_len > 0.000001f ){
			now_len -= org_len / 2 ;
			org_len -= org_len / 2 ;
			//printf("%f %f %f\n", DG_SQRT( org_len * org_len - now_len * now_len ), org_len, now_len );
			vec.vy += DG_SQRT( org_len * org_len - now_len * now_len );
		}
#endif
	}

	/* 不要なブレを抑えるため一定距離以内であればＩＫ基準点の補正を行わない */
	if ( PositionCorrect( &trg->pos, &vec, 20.0f ) == 0 ){
		/* 関節は動いていない */
		work->static_flag |= 1 ;
	}

#endif

}
#if 0
/* ＩＫ起点ターゲットの制御処理 */
static void CheckJointReact( TAKABE_PuppetIK *work, TRG_CTRL *trg )
{
	FVECTOR		vec ;
	float		len ;

	_sceVu0SubVector( &vec, &trg->pos, &trg->old_pos );
	len = vec.vx * vec.vx + vec.vz * vec.vz ;	/* 高さは考慮しない */
	if ( len < ( 20 * 20 ) ) return ;
	if ( len > work->repulsion_len ){
		work->repulsion_len = len ;
		work->repulsion_pos = trg->pos ;
		_sceVu0SubVector( &work->repulsion_vec, &trg->old_pos, &trg->pos ) ;
	}

}
#endif
/* ---------------------------------------------------------------- */
/* ＩＫターゲット座標などの初期化 */
static void ResetPosition( TAKABE_PuppetIK *work )
{
	FMATRIX	inv_mat ;

	InitIkTarget( work, &work->trg_right_arm, (FVECTOR*)work->object->objs->objs[6].world.m[3] );
	InitIkTarget( work, &work->trg_left_arm, (FVECTOR*)work->object->objs->objs[10].world.m[3] );
	InitIkTarget( work, &work->trg_right_leg, (FVECTOR*)work->object->objs->objs[15].world.m[3] );
	InitIkTarget( work, &work->trg_left_leg, (FVECTOR*)work->object->objs->objs[19].world.m[3] );
	InitIkTarget( work, &work->trg_head, (FVECTOR*)work->object->objs->objs[12].world.m[3] );

	work->flag |= FLAG_REQUEST_INIT_ROT ;

	work->weist_base = work->object->m_ctrl->abs_rots[0] ;
	_sceVu0InversMatrix( &inv_mat, &work->object->objs->world );
	GetJointPos( &work->r_leg_pos, work->object->objs, 15 );
	GetJointPos( &work->l_leg_pos, work->object->objs, 19 );
	_sceVu0ApplyMatrix( &work->r_leg_pos, &inv_mat, &work->r_leg_pos );
	_sceVu0ApplyMatrix( &work->l_leg_pos, &inv_mat, &work->l_leg_pos );
}

/* ---------------------------------------------------------------- */
	/*
		死体兵ＩＫの各種制御コマンド
	*/
void TAKABE_CmdPuppetIK( TAKABE_PuppetIK *work, int command, int param )
{
	switch ( command ){
	  case TAKABE_PUPPETIK_START:
		if ( work->mode != MODE_ACTIVE && !( work->mode == MODE_SLEEP && work->before_mode == MODE_ACTIVE ) ){
			work->mode = MODE_ACTIVE ;
		}
		break ;
	  case TAKABE_PUPPETIK_END:
		if ( work->mode != MODE_NO_ACTIVE && !( work->mode == MODE_SLEEP && work->before_mode == MODE_NO_ACTIVE ) ){
			work->mode = MODE_NO_ACTIVE ;
		}
		break ;
	  case TAKABE_PUPPETIK_IDLE:
		if ( work->mode != MODE_STATIC && !( work->mode == MODE_SLEEP && work->before_mode == MODE_STATIC ) ){
			work->mode = MODE_STATIC ;
			work->repulsion_time = 0 ;
		}
		break ;
	  case TAKABE_PUPPETIK_INIT_ROT:
		work->flag |= FLAG_REQUEST_INIT_ROT ;
		break ;
	  case TAKABE_PUPPETIK_INIT_TRG:
		work->flag |= FLAG_REQUEST_INIT_TRG ;
		break ;
	  case TAKABE_PUPPETIK_ENABLE_ARM:
		if ( param ){
			work->flag |= (FLAG_ACTIVE_RARM|FLAG_ACTIVE_LARM) ;
		} else {
			work->flag &= ~(FLAG_ACTIVE_RARM|FLAG_ACTIVE_LARM) ;
		}
		break ;
	  case TAKABE_PUPPETIK_ENABLE_LEG:
		if ( param ){
			work->flag |= (FLAG_ACTIVE_RLEG|FLAG_ACTIVE_LLEG) ;
		} else {
			work->flag &= ~(FLAG_ACTIVE_RLEG|FLAG_ACTIVE_LLEG) ;
		}
		break ;
	  case TAKABE_PUPPETIK_ENABLE_BODY:
		if ( param ){
			work->flag |= (FLAG_ACTIVE_HEAD) ;
		} else {
			work->flag &= ~(FLAG_ACTIVE_HEAD) ;
		}
		break ;
#ifdef DEBUG_MODE
	  case TAKABE_PUPPETIK_DEBUG_MODE:
		if ( param ){
			if ( !( work->flag & FLAG_ENABLE_DEBUG ) ) printf("puppetik.c: debug mode on\n");
			work->flag |= (FLAG_ENABLE_DEBUG) ;
		} else {
			work->flag &= ~(FLAG_ENABLE_DEBUG) ;
			if ( ( work->flag & FLAG_ENABLE_DEBUG ) ) printf("puppetik.c: debug mode off\n");
		}
		break ;
#endif
	}
}
/* ---------------------------------------------------------------- */
	/*
		死体兵ＩＫのメイン処理
	*/
int TAKABE_ActPuppetIK( TAKABE_PuppetIK *work )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	if ( work->mode == MODE_NO_ACTIVE ) return ( 1 );

	//if ( GV_PadData[1].press & PAD_X ){
	//	TAKABE_UtilPuppetIK_Piku( work, RAND(22) - 1 );
	//}

	ctrl = work->control ;
	body = work->object ;

	if ( work->flag & FLAG_REQUEST_INIT_TRG ){
		ResetPosition( work );
		work->flag &= ~FLAG_REQUEST_INIT_TRG ;
	}

	/* 基準ポーズの更新要求処理 */
	if ( work->flag & FLAG_REQUEST_INIT_ROT ){
		InitIkJoint( work, work->right_arm );
		InitIkJoint( work, work->right_leg );
		InitIkJoint( work, work->left_arm );
		InitIkJoint( work, work->left_leg );
		InitIkJoint( work, work->head );
		work->flag &= ~FLAG_REQUEST_INIT_ROT ;
	}

#if 0
	/* 状態によりモデルの中心位置を補正する */
	if ( work->mode == MODE_ACTIVE ){
		/* 足から引きずるときに上半身のめり込みを回避するために中心座標を補正 */
		FVECTOR		base, top, center, tmp_vec, rot_param, quat ;
		FMATRIX		mat, tmp_mat ;
		float		rate_param, rate_base, angle ;

		GetJointPos( &base, GM_PlayerBody->objs, 2 );
		GetJointPos( &center, body->objs, 0 );
		GetJointPos( &top, body->objs, 11 );
		//top.vx = center.vx + ( center.vx - base.vx );
		//top.vy = center.vy + ( center.vy - base.vy );
		//top.vz = center.vz + ( center.vz - base.vz );
		if ( HZX_OnlineHazardCheck( work->control->hzx_id, &base, &top, 
								   HZX_CHK_ALL, CHECK_SEG_FLAG, CHECK_FLOOR_FLAG ) ){
			if ( HZX_GetOnlineHazardType() == 2 ){
				HZX_FLR	flr ;
				int		atr ;
				FVECTOR	vec1, vec2, tmp_vec ;
				HZX_GetOnlineHazard( &flr, &atr );
				tmp_vec = top ;
				tmp_vec.vy = CalcFloorLevel( &flr, &top );
				_sceVu0SubVector( &vec1, &top, &center );		/* 旧方向 */
				_sceVu0SubVector( &vec2, &tmp_vec, &center );	/* 補正後方向 */
				_sceVu0Normalize( &vec1, &vec1 );
				_sceVu0Normalize( &vec2, &vec2 );
				
				MT_QuatSetFromAx( &quat, &vec1, &vec2 );
				MT_QuatGetValue( &rot_param, &quat );
				rot_param.vx = -1.0f ;
				rot_param.vy = 0.0f ;
				rot_param.vz = 0.0f ;
				rot_param.vw *= 0.5f ;
				MT_QuatSetValue( &quat, &rot_param );
				MT_QuatMul( &body->m_ctrl->abs_rots[0], &quat, &body->m_ctrl->abs_rots[0] );

#if 0
				center.vx = ( tmp_vec.vx + base.vx ) * 0.5f ;
				center.vy = ( tmp_vec.vy + base.vy ) * 0.5f ;
				center.vz = ( tmp_vec.vz + base.vz ) * 0.5f ;

				/* プレイヤーの立ち位置での床の高さと頭のオンラインチェックによる衝突座標の高さによるパラメータを計算 */
				rate_base = center.vy - GM_PlayerControl->levels[0] ;
				angle = atan2f( len, rate_base/2 );
				//printf("%f %f %f\n", rate_base, rate_param, angle * 180.0f / 3.14159265f );
				/* 角度補正の方向（に垂直なベクトル）を求める */
				{
					FVECTOR		y_vec = {0,-1,0,1} ;
					_sceVu0SubVector( &tmp_vec, &center, &GM_PlayerControl->mov );
					_sceVu0OuterProduct( &rot_param, &y_vec, &tmp_vec );
				}
				/* ベースから中心へのベクトルを求める */
				_sceVu0SubVector( &tmp_vec, &center, &base );
				/* 補正回転マトリクスを求める */
				rot_param.vw = angle ;
				MT_QuatSetValue( &quat, &rot_param );
				PrintQuat( "quat", &quat );
				MT_QuatToMat( &mat, &quat );
				/* 新センターの決定 */
				tmp_vec.vw = 1.0f ;
				_sceVu0ApplyMatrix( &tmp_vec, &mat, &tmp_vec );
				//_sceVu0AddVector( &center, &base, &tmp_vec );
				AN_Test_Eye2( &base, 3 );
				AN_Test_Eye2( &center, 3 );
				/* モデルの中心位置を補正する */
				tmp_mat = body->objs->world ;
				//_sceVu0MulMatrix( &tmp_mat, &mat, &tmp_mat );
				tmp_mat.m[3][0] = center.vx ;
				tmp_mat.m[3][1] = center.vy ;
				tmp_mat.m[3][2] = center.vz ;
				DG_SetPos( &tmp_mat );
#if 0
				/* 腰関節の回転も補正する */
				rot_param.vx = -1.0f ;
				rot_param.vy = 0.0f ;
				rot_param.vz = 0.0f ;
				MT_QuatSetValue( &quat, &rot_param );
				MT_QuatMul( &body->m_ctrl->abs_rots[0], &quat, &work->weist_base );
#endif
#if 0
				/* 足の座標をＩＫ基準位置として保存する（モデル表示位置の補正による足のずれをＩＫで補正するため） */
				GetJointPos( &work->trg_right_leg.pos, body->objs, 15 );
				work->trg_right_leg.old_pos = work->trg_right_leg.pos ;
				GetJointPos( &work->trg_left_leg.pos, body->objs, 19 );
				work->trg_left_leg.old_pos = work->trg_left_leg.pos ;
#else
				_sceVu0ApplyMatrix( &work->trg_right_leg.pos, &mat, &work->r_leg_pos );
				_sceVu0ApplyMatrix( &work->trg_left_leg.pos, &mat, &work->l_leg_pos );
				work->trg_right_leg.old_pos = work->trg_right_leg.pos ;
				work->trg_left_leg.old_pos = work->trg_left_leg.pos ;
#endif
#endif
				InitIkJoint( work, work->head );
				/* 再計算 */
				//GM_ActObject2( body );
				ActMotion2( body->m_ctrl, body->objs );

#if 0
				/* 足の位置をＩＫで補正 */
				CalcIK( work->right_leg, &work->trg_right_leg.pos, body->objs, 1 );
				CalcIK( work->left_leg, &work->trg_left_leg.pos, body->objs, 1 );
				FreedbackJoint( work, work->right_leg );
				FreedbackJoint( work, work->left_leg );
				/* ＩＫで計算されない先端部分を設定 */
				body->m_ctrl->abs_rots[15] = body->m_ctrl->abs_rots[14] ;
				body->m_ctrl->abs_rots[16] = body->m_ctrl->abs_rots[14] ;
				body->m_ctrl->abs_rots[19] = body->m_ctrl->abs_rots[18] ;
				body->m_ctrl->abs_rots[20] = body->m_ctrl->abs_rots[18] ;
#endif

			}

		}
	}
#endif

	/* 静止関係チェック初期化 */
	work->static_flag = 0 ;

	/* ＩＫ計算処理 */

	/* ＩＫターゲット位置決定 */
	if ( work->mode != MODE_SLEEP ){
		if ( work->flag & FLAG_ACTIVE_HEAD ) CheckIkTarget3( work, work->head, &work->trg_head, 1 );
		if ( work->flag & FLAG_ACTIVE_RARM ) CheckIkTarget3( work, work->right_arm, &work->trg_right_arm, 0 );
		if ( work->flag & FLAG_ACTIVE_LARM ) CheckIkTarget3( work, work->left_arm, &work->trg_left_arm, 0 );
		if ( work->flag & FLAG_ACTIVE_RLEG ) CheckIkTarget3( work, work->right_leg, &work->trg_right_leg, 0 );
		if ( work->flag & FLAG_ACTIVE_LLEG ) CheckIkTarget3( work, work->left_leg, &work->trg_left_leg, 0 );
	}

	/* 腕のＩＫに影響するため、体のＩＫを先に行う */
	if ( work->flag & FLAG_ACTIVE_HEAD ){
		CalcIK( work->head, &work->trg_head.pos, body->objs, 2 );
		FreedbackJoint( work, work->head );
		/* ＩＫで計算されない先端部分を設定 */
		body->m_ctrl->abs_rots[12] = body->m_ctrl->abs_rots[11] ;
		/* 一度関節位置を再計算する */
		//GM_ActObject2( body );
		ActMotion2( body->m_ctrl, body->objs );
	}

	/* 各関節のＩＫ計算 */
	if ( work->flag & FLAG_ACTIVE_RARM ) CalcIK( work->right_arm, &work->trg_right_arm.pos, body->objs, 2 );
	if ( work->flag & FLAG_ACTIVE_LARM ) CalcIK( work->left_arm, &work->trg_left_arm.pos, body->objs, 2 );
	if ( work->flag & FLAG_ACTIVE_RLEG ) CalcIK( work->right_leg, &work->trg_right_leg.pos, body->objs, 2 );
	if ( work->flag & FLAG_ACTIVE_LLEG ) CalcIK( work->left_leg, &work->trg_left_leg.pos, body->objs, 2 );

	/* ＩＫ計算結果をモーションに反映 */
	if ( work->flag & FLAG_ACTIVE_RARM ){
		FreedbackJoint( work, work->right_arm );
		/* ＩＫで計算されない先端部分を設定 */
		body->m_ctrl->abs_rots[6] = body->m_ctrl->abs_rots[5] ;
		body->m_ctrl->abs_rots[10] = body->m_ctrl->abs_rots[9] ;
	}
	if ( work->flag & FLAG_ACTIVE_LARM ){
		FreedbackJoint( work, work->left_arm );
		/* ＩＫで計算されない先端部分を設定 */
		body->m_ctrl->abs_rots[10] = body->m_ctrl->abs_rots[9] ;
	}
	if ( work->flag & FLAG_ACTIVE_RLEG ){
		FreedbackJoint( work, work->right_leg );
		/* ＩＫで計算されない先端部分を設定 */
		body->m_ctrl->abs_rots[15] = body->m_ctrl->abs_rots[14] ;
		body->m_ctrl->abs_rots[16] = body->m_ctrl->abs_rots[14] ;
	}
	if ( work->flag & FLAG_ACTIVE_LLEG ){
		FreedbackJoint( work, work->left_leg );
		/* ＩＫで計算されない先端部分を設定 */
		body->m_ctrl->abs_rots[19] = body->m_ctrl->abs_rots[18] ;
		body->m_ctrl->abs_rots[20] = body->m_ctrl->abs_rots[18] ;
	}

	/* モーションをモデルに反映 */
	//GM_ActObject2( body );
	ActMotion2( body->m_ctrl, body->objs );
	if ( body->evmobj != NULL ){
		EvmActMotion( body->m_ctrl, body->evmobj );
	}

	if ( work->mode != MODE_SLEEP ){
		GetJointPos( &work->trg_head.old_pos, work->object->objs, 12 );
		GetJointPos( &work->trg_right_arm.old_pos, work->object->objs, 6 );
		GetJointPos( &work->trg_left_arm.old_pos, work->object->objs, 10 );
		GetJointPos( &work->trg_right_leg.old_pos, work->object->objs, 15 );
		GetJointPos( &work->trg_left_leg.old_pos, work->object->objs, 19 );
		/* 非アクティブの関節用に最終検出床高さを設定する */
		if ( !( work->flag & FLAG_ACTIVE_HEAD ) ) work->trg_head.last_h = work->trg_head.old_pos.vy - 101.0f ;
		if ( !( work->flag & FLAG_ACTIVE_RARM ) ) work->trg_right_arm.last_h = work->trg_right_arm.old_pos.vy - 101.0f ;
		if ( !( work->flag & FLAG_ACTIVE_LARM ) ) work->trg_left_arm.last_h = work->trg_left_arm.old_pos.vy - 101.0f ;
		if ( !( work->flag & FLAG_ACTIVE_RLEG ) ) work->trg_right_leg.last_h = work->trg_right_arm.old_pos.vy - 101.0f ;
		if ( !( work->flag & FLAG_ACTIVE_LLEG ) ) work->trg_left_leg.last_h = work->trg_left_arm.old_pos.vy - 101.0f ;

#if 0
		if ( work->mode == MODE_STATIC ){
			float	len ;

			/* 反発関係チェック初期化 */
			work->repulsion_len = 0 ;
			work->repulsion_time++ ;

			/* ＩＫ起点と最終的なモデルの関節位置の差をチェックする（＝反発力） */
			if ( work->flag & FLAG_ACTIVE_HEAD ) CheckJointReact( work, &work->trg_head );
			if ( work->flag & FLAG_ACTIVE_RARM ) CheckJointReact( work, &work->trg_right_arm );
			if ( work->flag & FLAG_ACTIVE_LARM ) CheckJointReact( work, &work->trg_left_arm );
			if ( work->flag & FLAG_ACTIVE_RLEG ) CheckJointReact( work, &work->trg_right_leg );
			if ( work->flag & FLAG_ACTIVE_LLEG ) CheckJointReact( work, &work->trg_left_leg );

			/* 反発力のチェック */
			if ( work->repulsion_len != 0.0f && work->repulsion_time < 180 ){
				FVECTOR		vec1, vec2 ;
				float		c_omega, len1, len2, d, angle ;
				_sceVu0SubVector( &vec1, &work->repulsion_pos, &ctrl->mov );
				_sceVu0SubVector( &vec2, &vec1, &work->repulsion_vec );
				len1 = DG_SQRT( vec1.vx * vec1.vx + vec1.vz * vec1.vz );
				len2 = DG_SQRT( vec2.vx * vec2.vx + vec2.vz * vec2.vz );
				d = 1.0f / ( len1 * len2 );
				c_omega = vec1.vx * vec2.vx + vec1.vz * vec2.vz ;
				c_omega *= d ;
				angle = acosf( c_omega );
				if ( ( vec2.vz * vec1.vx - vec2.vx * vec1.vz ) > 0.0f ){
					/* 右回りに補正 */
					ctrl->rot.vy -= DG_FTOI( angle * 2048 / 3.14159265f );
					//ctrl->rot.vy -= 4 ;
				} else {
					/* 左回りに補正 */
					ctrl->rot.vy += DG_FTOI( angle * 2048 / 3.14159265f );
					//ctrl->rot.vy += 4 ;
				}
				PrintVec("old mov", &ctrl->mov );
				ctrl->turn.vy = ctrl->rot.vy ;
				//ctrl->mov.vx += work->repulsion_vec.vx ;
				//ctrl->mov.vz += work->repulsion_vec.vz ;
				PrintVec("mov", &ctrl->mov );
				PrintVec("repusion", &work->repulsion_vec );
#if 0
				/* そのフレーム内で移動・回転を反映させる */
				DG_SetPos2( &ctrl->mov, &ctrl->rot );
				/* モーションをモデルに反映 */
				//GM_ActObject2( body );
				ActMotion2( body->m_ctrl, body->objs );
				GetJointPos( &work->trg_head.old_pos, work->object->objs, 12 );
				GetJointPos( &work->trg_right_arm.old_pos, work->object->objs, 6 );
				GetJointPos( &work->trg_left_arm.old_pos, work->object->objs, 10 );
				GetJointPos( &work->trg_right_leg.old_pos, work->object->objs, 15 );
				GetJointPos( &work->trg_left_leg.old_pos, work->object->objs, 19 );
#endif
			}
		}
#endif

	}

	if ( work->mode != MODE_SLEEP ){
		/* 静止状態移行へのチェック */
		if ( work->mode == MODE_STATIC ){
			if ( work->static_flag ){
				if ( ++( work->static_count ) > 10 ){
					work->before_mode = work->mode ;
					work->mode = MODE_SLEEP ;
//					printf("ik sleep\n");
				}
			} else {
				work->static_count = 0 ;
			}
			if ( GetPosLength( &ctrl->mov, &work->last_ctrl_pos ) > 5.0f ) work->static_count = 0 ;
		}
		work->last_ctrl_pos = ctrl->mov ;
		work->last_ctrl_rot = ctrl->rot ;
	} else {
		/* 静止状態からの復帰チェック */
		if ( work->mode == MODE_SLEEP ){
			if ( GetPosLength( &ctrl->mov, &work->last_ctrl_pos ) > 5.0f ||
				ctrl->rot.vx != work->last_ctrl_rot.vx ||
				ctrl->rot.vy != work->last_ctrl_rot.vy ||
				ctrl->rot.vz != work->last_ctrl_rot.vz ){
				work->mode = work->before_mode ;
				work->static_count = 0 ;
//				printf("ik wake up\n");
			}
		}
	}

	if ( work->mode == MODE_SLEEP ) return ( 1 );
	return ( 0 );
}

void TAKABE_FreePuppetIK( TAKABE_PuppetIK *work )
{
	if ( work == NULL ) return ;
	GV_Free( work );
}

TAKABE_PuppetIK *TAKABE_MakePuppetIK( CONTROL *ctrl, OBJECT *body )
{
	TAKABE_PuppetIK	*work ;

	if ( ( work = GV_Malloc( sizeof(TAKABE_PuppetIK) ) ) == NULL ) return ( NULL );
	GV_ZeroMemory( work, sizeof(TAKABE_PuppetIK) );

	work->control = ctrl ;
	work->object = body ;

	work->right_arm[ 0 ].total_joints = 4 ;
	/* ＩＫ関連初期化 */
	/* ＩＫ終点関節（ダミー用） */
	SetJointInfo( &work->right_arm[ 0 ], 3, body->objs, 0, 0, 0, 0, 0, 0 );
	/* 上腕腕関節 */
	SetJointInfo( &work->right_arm[ 1 ], 4, body->objs, 120, -120, 40, -40, 80, -50 );
	/* 腕関節 */
	SetJointInfo( &work->right_arm[ 2 ], 5, body->objs, 0, -150, 20, -20, 0, 0 );
	/* ＩＫ起点関節（ダミー用） */
	SetJointInfo( &work->right_arm[ 3 ], 6, body->objs, 0, 0, 0, 0, 0, 0 );
	/* ＩＫ起点座標の設定 */
	InitIkTarget( work, &work->trg_right_arm, (FVECTOR*)body->objs->objs[6].world.m[3] );
	/* ＩＫ状態の初期化 */
	CalcIK( work->right_arm, &work->trg_right_arm.pos, body->objs, 3 );

	work->left_arm[ 0 ].total_joints = 4 ;
	/* ＩＫ終点関節（ダミー用） */
	SetJointInfo( &work->left_arm[ 0 ], 7, body->objs, 0, 0, 0, 0, 0, 0 );
	/* 上腕腕関節 */
	SetJointInfo( &work->left_arm[ 1 ], 8, body->objs, 120, -120, 40, -40, 50, -80 );
	/* 腕関節 */
	SetJointInfo( &work->left_arm[ 2 ], 9, body->objs, 0, -150, 20, -20, 0, 0 );
	/* ＩＫ起点関節（ダミー用） */
	SetJointInfo( &work->left_arm[ 3 ], 10, body->objs, 0, 0, 0, 0, 0, 0 );
	/* ＩＫ起点座標の設定 */
	InitIkTarget( work, &work->trg_left_arm, (FVECTOR*)body->objs->objs[10].world.m[3] );
	/* ＩＫ状態の初期化 */
	CalcIK( work->left_arm, &work->trg_left_arm.pos, body->objs, 3 );

	work->right_leg[ 0 ].total_joints = 4 ;
	/* ＩＫ終点関節（ダミー用） */
	SetJointInfo( &work->right_leg[ 0 ], 0, body->objs, 0, 0, 0, 0, 0, 0 );
	/* 腿関節 */
	SetJointInfo( &work->right_leg[ 1 ], 13, body->objs, 40, -120, 40, -70, 20, -70 );
	/* 膝関節 */
	SetJointInfo( &work->right_leg[ 2 ], 14, body->objs, 150, 0, 0, 0, 0, 0 );
	/* ＩＫ起点関節（ダミー用） */
	SetJointInfo( &work->right_leg[ 3 ], 15, body->objs, 0, 0, 0, 0, 0, 0 );
	/* ＩＫ起点座標の設定 */
	InitIkTarget( work, &work->trg_right_leg, (FVECTOR*)body->objs->objs[15].world.m[3] );
	/* ＩＫ状態の初期化 */
	CalcIK( work->right_leg, &work->trg_right_leg.pos, body->objs, 3 );

	work->left_leg[ 0 ].total_joints = 4 ;
	/* ＩＫ終点関節（ダミー用） */
	SetJointInfo( &work->left_leg[ 0 ], 0, body->objs, 0, 0, 0, 0, 0, 0 );
	/* 腿腕関節 */
	SetJointInfo( &work->left_leg[ 1 ], 17, body->objs, 40, -120, 70, -40, 70, -20 );
	/* 膝関節 */
	SetJointInfo( &work->left_leg[ 2 ], 18, body->objs, 150, 0, 0, 0, 0, 0 );
	/* ＩＫ起点関節（ダミー用） */
	SetJointInfo( &work->left_leg[ 3 ], 19, body->objs, 0, 0, 0, 0, 0, 0 );
	/* ＩＫ起点座標の設定 */
	InitIkTarget( work, &work->trg_left_leg, (FVECTOR*)body->objs->objs[19].world.m[3] );
	/* ＩＫ状態の初期化 */
	CalcIK( work->left_leg, &work->trg_left_leg.pos, body->objs, 3 );

	work->head[ 0 ].total_joints = 5 ;
	/* ＩＫ終点関節（ダミー用） */
	SetJointInfo( &work->head[ 0 ], 0, body->objs, 0, 0, 0, 0, 0, 0 );
	/*  */
	//SetJointInfo( &work->head[ 1 ], 1, body->objs, 40, -40, 0, 0, 30, -30 );
	SetJointInfo( &work->head[ 1 ], 1, body->objs, 140, -140, 0, 0, 30, -30 );
	/*  */
	//SetJointInfo( &work->head[ 2 ], 2, body->objs, 30, -30, 0, 0, 30, -30 );
	SetJointInfo( &work->head[ 2 ], 2, body->objs, 130, -130, 0, 0, 30, -30 );
	/*  */
	SetJointInfo( &work->head[ 3 ], 11, body->objs, 50, -50, 30, -30, 0, 0 );
	/* ＩＫ起点関節（ダミー用） */
	SetJointInfo( &work->head[ 4 ], 12, body->objs, 0, 0, 0, 0, 0, 0 );
	/* ＩＫ起点座標の設定 */
	InitIkTarget( work, &work->trg_head, (FVECTOR*)body->objs->objs[12].world.m[3] );
	/* ＩＫ状態の初期化 */
	CalcIK( work->head, &work->trg_head.pos, body->objs, 3 );


	work->mode = MODE_NO_ACTIVE ;
	work->time = 0 ;
	return (work);
}




/* ---------------------------------------------------------------- */
	/*
		敵兵用全自動ステータス設定ルーチン
	*/
void TAKABE_UtilPuppetIK_AutoConfigOfEnemey( TAKABE_PuppetIK *work, int mode )
{
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = work->control ;
	body = work->object ;

	switch ( mode ){
	  case 0:
		if ( work->mode == MODE_NO_ACTIVE ) TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_INIT_TRG, 0 ) ;
		if ( ctrl->mov.vy < ( ctrl->levels[0] + 300.0f ) ){
			TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_INIT_ROT, 0 ) ;
			//TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_START, 0 ) ;
		}
			TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_INIT_ROT, 0 ) ;
		TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_IDLE, 0 ) ;
		TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_ENABLE_ARM, 1 ) ;
		TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_ENABLE_LEG, 1 ) ;
		TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_ENABLE_BODY, 1 ) ;
		break ;
	  case 1:
		/* モード変更タイミングチェック */
		if ( work->mode != MODE_ACTIVE ){
			/* 腰の高さでモード変更を判断する */
			if ( ctrl->mov.vy > ( ctrl->levels[0] + 500.0f ) ){
				TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_START, 0 ) ;
				TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_INIT_ROT, 0 ) ;
				TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_ENABLE_ARM, 0 ) ;
				TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_ENABLE_LEG, 1 ) ;
				TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_ENABLE_BODY, 0 ) ;
			}
		}
		TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_ENABLE_ARM, 0 ) ;	/* ＩＫを行わない関節はモーションの補完に任せる */
		TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_ENABLE_BODY, 0 ) ;/* ＩＫを行わない関節はモーションの補完に任せる */
		break ;
	  case 2:
		/* モード変更タイミングチェック */
		if ( work->mode != MODE_ACTIVE ){
			/* 腰の高さでモード変更を判断する */
			if ( ctrl->mov.vy > ( ctrl->levels[0] + 500.0f ) ){
				TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_START, 0 ) ;
				TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_INIT_ROT, 0 ) ;
				TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_ENABLE_ARM, 1 ) ;
				TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_ENABLE_LEG, 0 ) ;
				TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_ENABLE_BODY, 1 ) ;
			}
		}
		TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_ENABLE_LEG, 0 ) ;	/* ＩＫを行わない関節はモーションの補完に任せる */
		break ;
	  default:
		/* モード変更タイミングチェック */
		if ( work->mode != MODE_NO_ACTIVE ){
			/* 腰の高さでモード変更を判断する */
			if ( ctrl->mov.vy < ( ctrl->levels[0] + 500.0f ) ){
				//TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_END, 0 ) ;
			}
			
		}
		TAKABE_CmdPuppetIK( work, TAKABE_PUPPETIK_END, 0 ) ;
		break ;
	}
}

/* ---------------------------------------------------------------- */
static void _copy_joint_info( MT_JOINT_INFO2 *src, MT_JOINT_INFO2 *dst )
{
	int			i ;
	for ( i = 0 ; i < dst->total_joints ; i++ ){
		src[i] = dst[i] ;
	}
}
static void _copy_trg_ctrl( TRG_CTRL *src, TRG_CTRL *dst )
{
	*src = *dst ;
}
	/*
		敵兵用全自動ステータス設定ルーチン
	*/
void TAKABE_UtilPuppetIK_CopyStatus( TAKABE_PuppetIK *dst_work, TAKABE_PuppetIK *src_work )
{
	_copy_joint_info( dst_work->head, src_work->head );
	_copy_joint_info( dst_work->right_arm, src_work->right_arm );
	_copy_joint_info( dst_work->left_arm, src_work->left_arm );
	_copy_joint_info( dst_work->right_leg, src_work->right_leg );
	_copy_joint_info( dst_work->left_leg, src_work->left_leg );
	_copy_trg_ctrl( &dst_work->trg_head, &src_work->trg_head );
	_copy_trg_ctrl( &dst_work->trg_right_arm, &src_work->trg_right_arm );
	_copy_trg_ctrl( &dst_work->trg_left_arm, &src_work->trg_left_arm );
	_copy_trg_ctrl( &dst_work->trg_right_leg, &src_work->trg_right_leg );
	_copy_trg_ctrl( &dst_work->trg_left_leg, &src_work->trg_left_leg );
	dst_work->mode = src_work->mode ;
	dst_work->before_mode = src_work->before_mode ;
	dst_work->flag = src_work->flag ;
	dst_work->time = src_work->time ;
	dst_work->static_count = src_work->static_count ;
	dst_work->static_flag = src_work->static_flag ;
	dst_work->last_ctrl_pos = src_work->last_ctrl_pos ;
	dst_work->last_ctrl_rot = src_work->last_ctrl_rot ;

	/*
	FVECTOR		weist_base ;
	FVECTOR		r_leg_pos ;
	FVECTOR		l_leg_pos ;
	int			repulsion_time ;
	float		repulsion_len ;
	FVECTOR		repulsion_vec ;
	FVECTOR		repulsion_pos ;
	*/
}

/* ---------------------------------------------------------------- */
static void PikuTarget( TRG_CTRL *trg )
{
	trg->piku_offset.vx += RAND( 160 ) - 80 ;
	trg->piku_offset.vy += RAND( 100 ) ;
	trg->piku_offset.vz += RAND( 160 ) - 80 ;
}
	/*
		敵兵用全自動ステータス設定ルーチン
	*/
void TAKABE_UtilPuppetIK_Piku( TAKABE_PuppetIK *work, int joint_num )
{
	if ( work->mode == MODE_SLEEP ){
		work->mode = work->before_mode ;
		work->static_count = 0 ;
		printf("ik piku wake up \n");
	}
	switch ( joint_num ){
	  case 3:
	  case 4:
	  case 5:
	  case 6:
		/* 右腕ピク */
		PikuTarget( &work->trg_right_arm );
		break ;
	  case 7:
	  case 8:
	  case 9:
	  case 10:
		/* 左腕ピク */
		PikuTarget( &work->trg_left_arm );
		break ;
	  case 13:
	  case 14:
	  case 15:
	  case 16:
		/* 右足ピク */
		PikuTarget( &work->trg_right_leg );
		break ;
	  case 17:
	  case 18:
	  case 19:
	  case 20:
		/* 左足ピク */
		PikuTarget( &work->trg_left_leg );
		break ;
	  case 11:
	  case 12:
		/* 頭ピク */
		PikuTarget( &work->trg_head );
		break ;
	  default:
		/* 全身ピク */
		PikuTarget( &work->trg_right_arm );
		PikuTarget( &work->trg_left_arm );
		PikuTarget( &work->trg_right_leg );
		PikuTarget( &work->trg_left_leg );
		PikuTarget( &work->trg_head );
		break ;
	}
}
