/*
	puppetik.c
	ＩＫ実験プログラム

	2000/10/16 K.Takabe
	$Id: tst_puppetik.c,v 1.1.1.3 2002/11/19 11:51:32 Yoshizawa1 Exp $

*/

#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#include <stdarg.h>

#include	"libdg.h"
#include	"gameheader.h"

/* ---------------------------------------------------------------- */
#define SET_VEC( _v, _x, _y, _z, _w ) { (_v)->vx = _x ; (_v)->vy = _y ; (_v)->vz = _z ; (_v)->vw = _w ;  }
static void PrintVec( char *header, FVECTOR *vec );

/* ---------------------------------------------------------------- */
#define NEW_IK
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
	FVECTOR		old_pos ;		/* 前回の座標 */
	FVECTOR		v_vec ;			/* 座標移動量 */
	FVECTOR		a_vec ;			/* 座標移動変化量 */
	float		last_h ;		/* 最後にチェックした床の座標 */
	float		max_len ;		/* 関節最大長 */
	int			pad[2] ;
} TRG_CTRL ;


typedef	struct	{
#ifndef NEW_IK
	MT_JOINT_INFO	head[4+2] ;
	MT_JOINT_INFO	left_arm[4+2] ;
	MT_JOINT_INFO	right_arm[4+2] ;
	MT_JOINT_INFO	left_leg[4+2] ;
	MT_JOINT_INFO	right_leg[4+2] ;
#else
	MT_JOINT_INFO2	head[5] ;
	MT_JOINT_INFO2	left_arm[4] ;
	MT_JOINT_INFO2	right_arm[4] ;
	MT_JOINT_INFO2	left_leg[4] ;
	MT_JOINT_INFO2	right_leg[4] ;
#endif
	CONTROL		*control ;
	OBJECT		*object ;
	TRG_CTRL	trg_left_arm ;
	TRG_CTRL	trg_right_arm ;
	TRG_CTRL	trg_left_leg ;
	TRG_CTRL	trg_right_leg ;
	TRG_CTRL	trg_head ;
	int			old_ik_mode ;
} TAKABE_PuppetIK ;

extern void MT_CalcInversKinematic( MT_JOINT_INFO *joints, int n_joints, FVECTOR *target_pos );




/* ---------------------------------------------------------------- */
#if 0
/* 間違えて作ってしまった */
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

	quat->vw = cr * cp * cy - sr * sp * sy ;
	quat->vx = sr * cp * cy + cr * sp * sy ;
	quat->vy = sr * cp * sy + cr * sp * cy ;
	quat->vz = cr * cp * sy - sr * sp * cy ;
}
/* ほかから持ってきたソースを書き直しただけであっているかどうか保証なし */
void MT_QuatToEulerXYZ( FVECTOR *euler, FVECTOR *quat )
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
void MT_QuatToEulerYZX( FVECTOR *euler, FVECTOR *quat )
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

	quat->vw = cr * cp * cy + sr * sp * sy ;
	quat->vx = sr * cp * cy - cr * sp * sy ;
	quat->vy = cr * sp * cy - sr * cp * sy ;
	quat->vz = cr * cp * sy + sr * sp * cy ;
}


/* 間接情報を元にＩＫ計算を行う */
void MT_CalcInversKinematic2( MT_JOINT_INFO2 *joints, int n_joints, FVECTOR *target_pos )
{
	static FVECTOR	zero_vec = {0,0,0,1};
	int		i, j, k ;
	FVECTOR		dir1, dir2, quat, from, to, euler, tmp_vec ;
	FMATRIX		mat, tmp_mat, inv_mat ;
	float		angle ;

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
			MT_QuatToEulerYZX( &euler, &tmp_vec );
			euler.vx = DG_MIN( euler.vx, joints[j].max.vx );
			euler.vy = DG_MIN( euler.vy, joints[j].max.vy );
			euler.vz = DG_MIN( euler.vz, joints[j].max.vz );
			euler.vx = DG_MAX( euler.vx, joints[j].min.vx );
			euler.vy = DG_MAX( euler.vy, joints[j].min.vy );
			euler.vz = DG_MAX( euler.vz, joints[j].min.vz );

			/* オイラー角からクォータニオンに戻す */
			MT_EulerToQuatYZX( &tmp_vec, &euler );

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
/* デバッグ用 */
static void PrintVec( char *header, FVECTOR *vec )
{
	float	len ;
	len = sqrtf( vec->vx * vec->vx + vec->vy * vec->vy + vec->vz * vec->vz );
	printf("%s %f %f %f %f (%f)\n", header, vec->vx, vec->vy, vec->vz, vec->vw, len );
	
}
/* デバッグ用 */
static void PrintQuat( char *header, FVECTOR *quat )
{
	FVECTOR	tmp ;
	MT_QuatGetValue( &tmp, quat );
	tmp.vw = tmp.vw * 180.0f / 3.14159265f ;
	printf("%s %f %f %f %f\n", header, tmp.vx, tmp.vy, tmp.vz, tmp.vw );
	MT_QuatToEulerYZX( &tmp, quat );
	printf("     (euler: %f %f %f)\n",
		   tmp.vx * 180.0f / (float)M_PI, tmp.vy * 180.0f / (float)M_PI, tmp.vz * 180.0f / (float)M_PI );
#if 0
	{/* quat to euler 動作チェック用 */
		FVECTOR	x, y, z, v ;

		MT_QuatToEulerYZX( &tmp, quat );
		printf("euler: %f %f %f\n",
			   tmp.vx * 180.0f / (float)M_PI, tmp.vy * 180.0f / (float)M_PI, tmp.vz * 180.0f / (float)M_PI );

		MT_EulerToQuatYZX( &x, &tmp );
		MT_QuatGetValue( &y, &x );
		y.vw = y.vw * 180.0f / 3.14159265f ;
		printf("&&%s %f %f %f %f\n", header, y.vx, y.vy, y.vz, y.vw );

		MT_QuatToEulerXYZ( &tmp, quat );
		printf("euler: %f %f %f\n",
			   tmp.vx * 180.0f / (float)M_PI, tmp.vy * 180.0f / (float)M_PI, tmp.vz * 180.0f / (float)M_PI );
		MT_EulerToQuatXYZ( &x, &tmp );
		MT_QuatGetValue( &y, &x );
		y.vw = y.vw * 180.0f / 3.14159265f ;
		printf("&&%s %f %f %f %f\n", header, y.vx, y.vy, y.vz, y.vw );


	}
#endif
}
static void PrintMat( char *header, FMATRIX *mat )
{
	printf("%s\n", header );
	printf("%f %f %f %f \n", mat->m[0][0], mat->m[1][0], mat->m[2][0], mat->m[3][0] );
	printf("%f %f %f %f \n", mat->m[0][1], mat->m[1][1], mat->m[2][1], mat->m[3][1] );
	printf("%f %f %f %f \n", mat->m[0][2], mat->m[1][2], mat->m[2][2], mat->m[3][2] );
	printf("%f %f %f %f \n", mat->m[0][3], mat->m[1][3], mat->m[2][3], mat->m[3][3] );
	
}
/* ---------------------------------------------------------------- */
static void ScaleVec( FVECTOR *vec, float len )
{
	float	l ;
	l = vec->vx * vec->vx + vec->vy * vec->vy + vec->vz * vec->vz ;
	l = len * DG_RSQRT( l );
	vec->vx *= l ;
	vec->vy *= l ;
	vec->vz *= l ;
}
static void MiniVec( FVECTOR *vec, float len )
{
	float	l ;
	l = vec->vx * vec->vx + vec->vy * vec->vy + vec->vz * vec->vz ;
	if ( l < len * len ) return ;
	l = len * DG_RSQRT( l );
	vec->vx *= l ;
	vec->vy *= l ;
	vec->vz *= l ;
}

/* ---------------------------------------------------------------- */
/* 仮想座標と実座標が指定した距離以上離れている場合には仮想座標位置を補正する */
static void PositionCorrect( FVECTOR *v_pos, FVECTOR *r_pos, float range )
{
	FVECTOR	vec ;
	float	len ;
	_sceVu0SubVector( &vec, v_pos, r_pos );
	len = vec.vx * vec.vx + vec.vy * vec.vy + vec.vz * vec.vz ;
	if ( len > ( range * range ) ){
		*v_pos = *r_pos ;
	}
}
/* ベースからの距離が遠いほうで補正 */
static void PositionCorrect2( FVECTOR *v_pos, FVECTOR *r_pos, FVECTOR *base )
{
	FVECTOR	vec ;
	float	len1, len2 ;
	_sceVu0SubVector( &vec, v_pos, base );
	len1 = vec.vx * vec.vx + vec.vy * vec.vy + vec.vz * vec.vz ;
	_sceVu0SubVector( &vec, r_pos, base );
	len2 = vec.vx * vec.vx + vec.vy * vec.vy + vec.vz * vec.vz ;
	if ( len1 < len2 ){
		*v_pos = *r_pos ;
	}
}
/* ---------------------------------------------------------------- */
/* 関節情報設定 */
#ifndef NEW_IK
static void SetJointInfo( MT_JOINT_INFO *joint, int type, FVECTOR *trans, int max, int min )
{
	joint->type = type ;
	joint->rotate = DG_ZeroVector ;
	joint->trans.vx = trans->vx ;
	joint->trans.vy = trans->vy ;
	joint->trans.vz = trans->vz ;
	joint->trans.vw = 1.0f ;
	joint->max =  cosf( ( max / 180.0f * 3.14159265f ) / 2.0f ) ;
	joint->min = -cosf( ( min / 180.0f * 3.14159265f ) / 2.0f );
}
#else
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
#endif
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
							   , HZX_CHK_ALL, HZX_SEG_ALL, HZX_FLOOR_ALL ) ){
		HZX_GetOnlinePoint( &trg->pos );
		PrintVec( " from", &ctrl_pos );
		PrintVec( " to", pos );
		PrintVec( " pos", &trg->pos );
	}

	trg->old_pos = trg->pos ;
	/* 加速度の初期化（適当） */
	trg->a_vec.vy = -2.0f ;
	/* 最終タッチフロアの高さを設定 */
	trg->last_h = trg->pos.vy + 1.0f ;
}
#ifndef NEW_IK
/* ＩＫ関節の初期化 */
static void InitIkJoint( TAKABE_PuppetIK *work, MT_JOINT_INFO *joint )
{
}
#else
/* ＩＫ関節の初期化 */
static void InitIkJoint( TAKABE_PuppetIK *work, MT_JOINT_INFO2 *joint )
{
	MOTION_CONTROL	*m_ctrl ;
	FVECTOR			local_quat, parent_quat, inv_quat ;
	int		i ;

	m_ctrl = work->object->m_ctrl ;
	for ( i = 1 ; i < joint[0].total_joints - 1 ; i++ ){
		/* 絶対回転クォータニオンからローカルクォータニオンを生成 */
		parent_quat = m_ctrl->abs_rots[ joint[i].joint_parent_num ] ;
		MT_QuatInverse( &inv_quat, &parent_quat );
		MT_QuatMul( &local_quat, &inv_quat, &m_ctrl->abs_rots[ joint[i].joint_num ] );
		joint[i].rotate = local_quat ;
		joint[i].base = local_quat ;
	}
}
#endif

#ifndef NEW_IK
/* ＩＫ計算実行 */
static void CalcIK( MT_JOINT_INFO *joint, FVECTOR *trg_pos, DG_OBJS *objs, int root_joint, int loop )
{
	FMATRIX		inv_mat ;
	FVECTOR		trg ;
	int			i ;

#if 0
	/* ルート関節からの相対座標に変換してから計算を行う（誤差低減のため） */
	//joint[0].matrix = objs->objs[root_joint].world ;
	joint[0].matrix = DG_UnitMatrix ;
	_sceVu0InversMatrix( &inv_mat, &objs->objs[root_joint].world );
	_sceVu0ApplyMatrix( &trg, &inv_mat, trg_pos );
	for ( i = 0 ; i < loop ; i++ ){
		MT_CalcInversKinematic( joint, 6, &trg );
	}
#else
	joint[0].matrix = objs->objs[root_joint].world ;
	for ( i = 0 ; i < loop ; i++ ){
		MT_CalcInversKinematic( joint, 6, trg_pos );
	}
#endif
}
#else
static void CalcIK( MT_JOINT_INFO2 *joint, FVECTOR *trg_pos, DG_OBJS *objs, int loop )
{
	FMATRIX		inv_mat ;
	FVECTOR		trg ;
	int			i ;

	joint[0].matrix = objs->objs[joint[0].joint_num].world ;
	for ( i = 0 ; i < loop ; i++ ){
		MT_CalcInversKinematic2( joint, joint[0].total_joints, trg_pos );
	}
}
#endif

#ifndef NEW_IK
/* ＩＫ結果をモーションにフィードバックさせる */
static void FreedbackJoint( TAKABE_PuppetIK *work, MT_JOINT_INFO *joint )
{
}
#else
/* ＩＫ結果をモーションにフィードバックさせる */
static void FreedbackJoint( TAKABE_PuppetIK *work, MT_JOINT_INFO2 *joint )
{
	MOTION_CONTROL	*m_ctrl ;
	int		i ;

	m_ctrl = work->object->m_ctrl ;
	for ( i = 1 ; i < joint[0].total_joints - 1 ; i++ ){
#if 1
		MT_QuatMul( &m_ctrl->abs_rots[joint[i].joint_num],
				   &m_ctrl->abs_rots[joint[i].joint_parent_num], &joint[i].rotate );
#else
		{/* 滑らかに変化させるために補完を行う */
			FVECTOR		tmp_quat ;
			MT_QuatMul( &tmp_quat, &m_ctrl->abs_rots[joint[i].joint_parent_num], &joint[i].rotate );
			MT_QuatSlerp( &m_ctrl->abs_rots[joint[i].joint_num],
						 &m_ctrl->abs_rots[joint[i].joint_num], &tmp_quat, 0.3f );
			MT_QuatNormalize( &m_ctrl->abs_rots[joint[i].joint_num], &m_ctrl->abs_rots[joint[i].joint_num] );
		}
#endif
	}
}
#endif


/* ＩＫ起点ターゲットの制御処理 */
static void CheckIkTarget( TAKABE_PuppetIK *work, TRG_CTRL *trg )
{
	float	near_size, len, h ;

	/* 壁突き抜けチェック */

	/* 加速度調整（適当） */
	/* 速度調整（適当） */
	//_sceVu0AddVector( &trg->v_vec, &trg->v_vec, &trg->a_vec );
	//if ( trg->v_vec.vy < -50.0f ) trg->v_vec.vy = -50.0f ;
	//len = trg->v_vec.vx * trg->v_vec.vx + trg->v_vec.vy * trg->v_vec.vy + trg->v_vec.vz * trg->v_vec.vz ;
	//near_size = DG_SQRT( len ) + 1.0f ;
	/* 座標を移動させる */
	//_sceVu0AddVector( &trg->pos, &trg->pos, &trg->v_vec );

#if 0
	/* 突き抜けチェック */
	if ( HZX_OnlineHazardCheck( work->control->hzx_id, &trg->old_pos, &trg->pos
							   , HZX_CHK_ALL, HZX_SEG_ALL, HZX_FLOOR_ALL ) ){
		HZX_GetOnlinePoint( &trg->pos );
	}
	trg->old_pos = trg->pos ;
#endif

	/* 等速落下のみ対応 */
	trg->pos.vy -= 50.0f ;

	/* 隣接壁チェック */
	if ( HZX_NearHazardCheck( work->control->hzx_id, &trg->pos ,
							  200, /* 検出半径 */
							  HZX_CHK_F_SEGMENT | HZX_CHK_D_SEGMENT,0,
							  200 /* 反発半径 */) ){
		FVECTOR v;
		HZX_GetReactVector(&v);
		_sceVu0AddVector( &trg->pos, &trg->pos, &v );
	}

	/* 床チェック */
	HZX_LevelHazardCheck( work->control->hzx_id, &trg->pos,
						 HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR,0);
	h = HZX_GetFloorLevel();
	if ( trg->pos.vy < ( h + 100.0f ) ){
		trg->pos.vy = h + 100.0f ;	/* 自由落下による突き抜けを防止するために少し浮かせる */
		//trg->v_vec.vy = 0.0f ;	/* 接地したので速度を０に */
	}
	trg->last_h = h ;

	
}
/* ＩＫ根元とＩＫ基準点との距離から補正を行う */
static void CheckPosition2( TAKABE_PuppetIK *work, MT_JOINT_INFO2 *joint, TRG_CTRL *trg )
{
	FVECTOR		vec, *root_pos, *tip_pos ;
	int		i, total_joints ;
	float	h ;

	total_joints = joint[0].total_joints ;
	root_pos = (FVECTOR*)work->object->objs->objs[joint[0].joint_num].world.m[3] ;				/* 根元 */
	tip_pos = (FVECTOR*)work->object->objs->objs[joint[total_joints-1].joint_num].world.m[3] ;	/* 先端 */

	/* 突き抜けチェック */
	/* 最終検出フロアとのチェック */
	vec = *tip_pos ;
	if ( vec.vy < ( trg->last_h + 100.0f ) ) vec.vy = trg->last_h + 100.0f ;
	/* 壁抜けのチェック */
	if ( HZX_OnlineHazardCheck( work->control->hzx_id, (FVECTOR*)work->object->objs->world.m[3], &vec, 
							   HZX_CHK_ALL, HZX_SEG_ALL, HZX_FLOOR_ALL ) ){
		HZX_GetOnlinePoint( &vec );
		/* 床チェック */
		HZX_LevelHazardCheck( work->control->hzx_id, &trg->pos,
							 HZX_CHK_F_FLOOR | HZX_CHK_D_FLOOR,0);
		h = HZX_GetFloorLevel();
		if ( vec.vy < ( h + 50.0f ) ) vec.vy += 50.0f ;
		trg->pos.vy = vec.vy ;
		PositionCorrect( &trg->pos, &vec, 50.0f );
	} else {
		/* 不要なブレを抑えるため一定距離以内であればＩＫ基準点の補正を行わない */
		PositionCorrect( &trg->pos, &vec, 50.0f );
	}

}
/* ---------------------------------------------------------------- */
/* ＩＫターゲット座標などの初期化 */
void TAKABE_ResetPosition( TAKABE_PuppetIK *work )
{
	InitIkTarget( work, &work->trg_right_arm, (FVECTOR*)work->object->objs->objs[6].world.m[3] );
	InitIkTarget( work, &work->trg_left_arm, (FVECTOR*)work->object->objs->objs[10].world.m[3] );
	InitIkTarget( work, &work->trg_right_leg, (FVECTOR*)work->object->objs->objs[15].world.m[3] );
	InitIkTarget( work, &work->trg_left_leg, (FVECTOR*)work->object->objs->objs[19].world.m[3] );
	InitIkTarget( work, &work->trg_head, (FVECTOR*)work->object->objs->objs[12].world.m[3] );

	InitIkJoint( work, work->right_arm );
	InitIkJoint( work, work->right_leg );
	InitIkJoint( work, work->left_arm );
	InitIkJoint( work, work->left_leg );
	InitIkJoint( work, work->head );
}

/* ---------------------------------------------------------------- */
void TAKABE_ActPuppetIK( TAKABE_PuppetIK *work, int ik_mode )
{
	FVECTOR		vec ;
	CONTROL	*ctrl ;
	OBJECT	*body ;

	if ( ik_mode == -1 ){
		work->old_ik_mode = -1 ;
		return ;
	}
	if ( work->old_ik_mode == -1 ){
		TAKABE_ResetPosition( work );
		work->old_ik_mode = ik_mode ;
	}

	ctrl = work->control ;
	body = work->object ;

	/* ＩＫターゲット位置決定 */
	CheckIkTarget( work, &work->trg_right_arm );
	CheckIkTarget( work, &work->trg_right_leg );
	CheckIkTarget( work, &work->trg_left_arm );
	CheckIkTarget( work, &work->trg_left_leg );
	CheckIkTarget( work, &work->trg_head );

	/* ＩＫによる関節角度計算 */
#ifndef NEW_IK
	if ( !( ik_mode & 1 ) ){
		CalcIK( work->right_leg, &work->trg_right_leg.pos, body->objs, 0, 1 );
		CalcIK( work->left_leg, &work->trg_left_leg.pos, body->objs, 0, 1 );
	} else {
		CalcIK( work->head, &work->trg_head.pos, body->objs, 0, 1 );
	}
#else
	if ( !( ik_mode & 1 ) ){
		CalcIK( work->right_leg, &work->trg_right_leg.pos, body->objs, 1 );
		CalcIK( work->left_leg, &work->trg_left_leg.pos, body->objs, 1 );
	} else {
		CalcIK( work->head, &work->trg_head.pos, body->objs, 1 );
	}
#endif

	/* ＩＫ結果をモデルにフィードバックさせる */
#ifndef NEW_IK
	if ( !( ik_mode & 1 ) ){
		{/**/
			FVECTOR		quat ;
			quat = work->right_leg[1].rotate ;
			MT_QuatMul( &quat, &quat, &work->right_leg[2].rotate );
			MT_QuatMul( &quat, &quat, &work->right_leg[3].rotate );
			MT_QuatMul( &body->m_ctrl->abs_rots[13], &body->m_ctrl->abs_rots[0], &quat );
			MT_QuatMul( &body->m_ctrl->abs_rots[14], &body->m_ctrl->abs_rots[13], &work->right_leg[4].rotate );
			body->m_ctrl->abs_rots[15] = body->m_ctrl->abs_rots[14] ;
			body->m_ctrl->abs_rots[16] = body->m_ctrl->abs_rots[14] ;
		}
		{/**/
			FVECTOR		quat ;
			quat = work->left_leg[1].rotate ;
			MT_QuatMul( &quat, &quat, &work->left_leg[2].rotate );
			MT_QuatMul( &quat, &quat, &work->left_leg[3].rotate );
			MT_QuatMul( &body->m_ctrl->abs_rots[17], &body->m_ctrl->abs_rots[0], &quat );
			MT_QuatMul( &body->m_ctrl->abs_rots[18], &body->m_ctrl->abs_rots[17], &work->left_leg[4].rotate );
			body->m_ctrl->abs_rots[19] = body->m_ctrl->abs_rots[18] ;
			body->m_ctrl->abs_rots[20] = body->m_ctrl->abs_rots[18] ;
		}
	} else {
#if 1
		{/**/
			FVECTOR		quat ;
			quat = work->head[1].rotate ;
			MT_QuatMul( &quat, &quat, &work->head[2].rotate );
			MT_QuatMul( &body->m_ctrl->abs_rots[1], &body->m_ctrl->abs_rots[0], &quat );
			MT_QuatMul( &body->m_ctrl->abs_rots[2], &body->m_ctrl->abs_rots[1], &work->head[3].rotate );
			MT_QuatMul( &body->m_ctrl->abs_rots[11], &body->m_ctrl->abs_rots[1], &work->head[4].rotate );
			body->m_ctrl->abs_rots[12] = body->m_ctrl->abs_rots[11] ;
		}
		GM_ActObject2( body );
#endif
	}
#else
	if ( !( ik_mode & 1 ) ){
#if 0
		MT_QuatMul( &body->m_ctrl->abs_rots[13], &body->m_ctrl->abs_rots[0], &work->right_leg[1].rotate );
		MT_QuatMul( &body->m_ctrl->abs_rots[14], &body->m_ctrl->abs_rots[13], &work->right_leg[2].rotate );
		body->m_ctrl->abs_rots[15] = body->m_ctrl->abs_rots[14] ;
		body->m_ctrl->abs_rots[16] = body->m_ctrl->abs_rots[14] ;

		MT_QuatMul( &body->m_ctrl->abs_rots[17], &body->m_ctrl->abs_rots[0], &work->left_leg[1].rotate );
		MT_QuatMul( &body->m_ctrl->abs_rots[18], &body->m_ctrl->abs_rots[17], &work->left_leg[2].rotate );
		body->m_ctrl->abs_rots[19] = body->m_ctrl->abs_rots[18] ;
		body->m_ctrl->abs_rots[20] = body->m_ctrl->abs_rots[18] ;
#else
		FreedbackJoint( work, work->right_leg );
		FreedbackJoint( work, work->left_leg );
		/* ＩＫで計算されない先端部分を設定 */
		body->m_ctrl->abs_rots[15] = body->m_ctrl->abs_rots[14] ;
		body->m_ctrl->abs_rots[16] = body->m_ctrl->abs_rots[14] ;
		body->m_ctrl->abs_rots[19] = body->m_ctrl->abs_rots[18] ;
		body->m_ctrl->abs_rots[20] = body->m_ctrl->abs_rots[18] ;
#endif
	} else {
		FreedbackJoint( work, work->head );
		/* ＩＫで計算されない先端部分を設定 */
		//body->m_ctrl->abs_rots[11] = body->m_ctrl->abs_rots[2] ;
		body->m_ctrl->abs_rots[12] = body->m_ctrl->abs_rots[11] ;
	}
#endif

#ifndef NEW_IK
	/* ＩＫによる関節角度計算 */
	CalcIK( work->right_arm, &work->trg_right_arm.pos, body->objs, 3, 1 );
	CalcIK( work->left_arm, &work->trg_left_arm.pos, body->objs, 7, 1 );
#else
	/* ＩＫによる関節角度計算 */
	CalcIK( work->right_arm, &work->trg_right_arm.pos, body->objs, 1 );
	CalcIK( work->left_arm, &work->trg_left_arm.pos, body->objs, 1 );
#endif
	/* ＩＫ結果をモデルにフィードバックさせる */
#ifndef NEW_IK
	{/**/
		FVECTOR		quat ;
		quat = work->right_arm[1].rotate ;
		MT_QuatMul( &quat, &quat, &work->right_arm[2].rotate );
		MT_QuatMul( &quat, &quat, &work->right_arm[3].rotate );
		MT_QuatMul( &body->m_ctrl->abs_rots[4], &body->m_ctrl->abs_rots[3], &quat );
		MT_QuatMul( &body->m_ctrl->abs_rots[5], &body->m_ctrl->abs_rots[4], &work->right_arm[4].rotate );
		body->m_ctrl->abs_rots[6] = body->m_ctrl->abs_rots[5] ;
		//PrintQuat( "1: ", &work->right_arm[1].rotate );
		//PrintQuat( "2: ", &work->right_arm[2].rotate );
		//PrintQuat( "3: ", &work->right_arm[3].rotate );
		//PrintQuat( "quat: ", &quat );
		//PrintQuat( "4: ", &work->right_arm[4].rotate );
	}
	{/**/
		FVECTOR		quat ;
		quat = work->left_arm[1].rotate ;
		MT_QuatMul( &quat, &quat, &work->left_arm[2].rotate );
		MT_QuatMul( &quat, &quat, &work->left_arm[3].rotate );
		MT_QuatMul( &body->m_ctrl->abs_rots[8], &body->m_ctrl->abs_rots[7], &quat );
		MT_QuatMul( &body->m_ctrl->abs_rots[9], &body->m_ctrl->abs_rots[8], &work->left_arm[4].rotate );
		body->m_ctrl->abs_rots[10] = body->m_ctrl->abs_rots[9] ;
	}
#else
	FreedbackJoint( work, work->right_arm );
	FreedbackJoint( work, work->left_arm );
	/* ＩＫで計算されない先端部分を設定 */
	body->m_ctrl->abs_rots[6] = body->m_ctrl->abs_rots[5] ;
	body->m_ctrl->abs_rots[10] = body->m_ctrl->abs_rots[9] ;
#endif

	GM_ActObject2( body );

	/* ＩＫ基準点の補正処理 */
	CheckPosition2( work, work->head, &work->trg_head );
	CheckPosition2( work, work->right_arm, &work->trg_right_arm );
	CheckPosition2( work, work->left_arm, &work->trg_left_arm );
	CheckPosition2( work, work->right_leg, &work->trg_right_leg );
	CheckPosition2( work, work->left_leg, &work->trg_left_leg );

	//AN_Test_Eye2( &work->trg_right_arm.pos, 3 );
	//AN_Test_Eye2( &work->trg_right_leg.pos, 3 );
	//AN_Test_Eye2( &work->trg_left_arm.pos, 3 );
	//AN_Test_Eye2( &work->trg_left_leg.pos, 3 );
	//AN_Test_Eye2( &work->trg_head.pos, 3 );

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

#ifndef NEW_IK
	/* ＩＫ関連初期化 */
	/* ＩＫ終点関節（ダミー用） */
	SetJointInfo( &work->right_arm[ 0 ], 0, &body->objs->objs[3].trans, 0, 0 );
	/* 上腕腕関節 */
	//SetJointInfo( &work->right_arm[ 1 ], 0, &body->objs->objs[4].trans, 60, -170 );
	SetJointInfo( &work->right_arm[ 1 ], 0, &body->objs->objs[4].trans, 180, -180 );
	SetJointInfo( &work->right_arm[ 2 ], 2, &DG_ZeroVector, 50, -90 );
	SetJointInfo( &work->right_arm[ 3 ], 1, &DG_ZeroVector, 100, -45 );
	/* 腕関節 */
	SetJointInfo( &work->right_arm[ 4 ], 0, &body->objs->objs[5].trans, 0, -150 );
	/* ＩＫ起点関節（ダミー用） */
	SetJointInfo( &work->right_arm[ 5 ], 0, &body->objs->objs[6].trans, 0, 0 );
	/* ＩＫ起点座標の設定 */
	InitIkTarget( work, &work->trg_right_arm, (FVECTOR*)body->objs->objs[6].world.m[3] );
	/* ＩＫ状態の初期化 */
	CalcIK( work->right_arm, &work->trg_right_arm.pos, body->objs, 3, 3 );

	/* ＩＫ終点関節（ダミー用） */
	SetJointInfo( &work->left_arm[ 0 ], 0, &body->objs->objs[7].trans, 0, 0 );
	/* 上腕腕関節 */
	//SetJointInfo( &work->left_arm[ 1 ], 0, &body->objs->objs[8].trans, 60, -170 );
	SetJointInfo( &work->left_arm[ 1 ], 0, &body->objs->objs[8].trans, 180, -180 );
	SetJointInfo( &work->left_arm[ 2 ], 2, &DG_ZeroVector, 90, -50 );
	SetJointInfo( &work->left_arm[ 3 ], 1, &DG_ZeroVector, 45, -100 );
	/* 腕関節 */
	SetJointInfo( &work->left_arm[ 4 ], 0, &body->objs->objs[9].trans, 0, -150 );
	/* ＩＫ起点関節（ダミー用） */
	SetJointInfo( &work->left_arm[ 5 ], 0, &body->objs->objs[10].trans, 0, 0 );
	/* ＩＫ起点座標の設定 */
	InitIkTarget( work, &work->trg_left_arm, (FVECTOR*)body->objs->objs[10].world.m[3] );
	/* ＩＫ状態の初期化 */
	CalcIK( work->left_arm, &work->trg_left_arm.pos, body->objs, 7, 3 );

	/* ＩＫ終点関節（ダミー用） */
	SetJointInfo( &work->right_leg[ 0 ], 0, &body->objs->objs[0].trans, 0, 0 );
	/* 腿関節 */
	SetJointInfo( &work->right_leg[ 1 ], 0, &body->objs->objs[13].trans, 40, -120 );
	SetJointInfo( &work->right_leg[ 2 ], 2, &DG_ZeroVector, 20, -45 );
	SetJointInfo( &work->right_leg[ 3 ], 1, &DG_ZeroVector, 40, -70 );
	/* 膝関節 */
	SetJointInfo( &work->right_leg[ 4 ], 0, &body->objs->objs[14].trans, 150, 0 );
	/* ＩＫ起点関節（ダミー用） */
	SetJointInfo( &work->right_leg[ 5 ], 0, &body->objs->objs[15].trans, 0, 0 );
	/* ＩＫ起点座標の設定 */
	InitIkTarget( work, &work->trg_right_leg, (FVECTOR*)body->objs->objs[15].world.m[3] );
	/* ＩＫ状態の初期化 */
	CalcIK( work->right_leg, &work->trg_right_leg.pos, body->objs, 0, 3 );

	/* ＩＫ終点関節（ダミー用） */
	SetJointInfo( &work->left_leg[ 0 ], 0, &body->objs->objs[0].trans, 0, 0 );
	/* 腿腕関節 */
	SetJointInfo( &work->left_leg[ 1 ], 0, &body->objs->objs[17].trans, 40, -120 );
	SetJointInfo( &work->left_leg[ 2 ], 2, &DG_ZeroVector, 45, -20 );
	SetJointInfo( &work->left_leg[ 3 ], 1, &DG_ZeroVector, 70, -40 );
	/* 膝関節 */
	SetJointInfo( &work->left_leg[ 4 ], 0, &body->objs->objs[18].trans, 150, 0 );
	/* ＩＫ起点関節（ダミー用） */
	SetJointInfo( &work->left_leg[ 5 ], 0, &body->objs->objs[19].trans, 0, 0 );
	/* ＩＫ起点座標の設定 */
	InitIkTarget( work, &work->trg_left_leg, (FVECTOR*)body->objs->objs[19].world.m[3] );
	/* ＩＫ状態の初期化 */
	CalcIK( work->left_leg, &work->trg_left_leg.pos, body->objs, 0, 3 );

	/* ＩＫ終点関節（ダミー用） */
	SetJointInfo( &work->head[ 0 ], 0, &body->objs->objs[0].trans, 0, 0 );
	/*  */
	SetJointInfo( &work->head[ 1 ], 0, &body->objs->objs[1].trans, 60, -40 );
	SetJointInfo( &work->head[ 2 ], 2, &DG_ZeroVector, 30, -30 );
	/*  */
	SetJointInfo( &work->head[ 3 ], 0, &body->objs->objs[2].trans, 60, -30 );
	/*  */
	SetJointInfo( &work->head[ 4 ], 0, &body->objs->objs[11].trans, 50, -40 );
	/* ＩＫ起点関節（ダミー用） */
	SetJointInfo( &work->head[ 5 ], 0, &body->objs->objs[12].trans, 0, 0 );
	/* ＩＫ起点座標の設定 */
	InitIkTarget( work, &work->trg_head, (FVECTOR*)body->objs->objs[12].world.m[3] );
	/* ＩＫ状態の初期化 */
	CalcIK( work->head, &work->trg_head.pos, body->objs, 0, 3 );
#else
	work->right_arm[ 0 ].total_joints = 4 ;
	/* ＩＫ関連初期化 */
	/* ＩＫ終点関節（ダミー用） */
	SetJointInfo( &work->right_arm[ 0 ], 3, body->objs, 0, 0, 0, 0, 0, 0 );
	/* 上腕腕関節 */
	SetJointInfo( &work->right_arm[ 1 ], 4, body->objs, 120, -120, 0, 0, 80, -50 );
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
	SetJointInfo( &work->left_arm[ 1 ], 8, body->objs, 120, -120, 0, 0, 50, -80 );
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
	SetJointInfo( &work->right_leg[ 1 ], 12, body->objs, 40, -120, 40, -70, 20, -45 );
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
	SetJointInfo( &work->left_leg[ 1 ], 17, body->objs, 40, -120, 70, -40, 45, -20 );
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
	SetJointInfo( &work->head[ 1 ], 1, body->objs, 30, -40, 0, 0, 30, -30 );
	/*  */
	SetJointInfo( &work->head[ 2 ], 2, body->objs, 30, -30, 0, 0, 30, -30 );
	/*  */
	SetJointInfo( &work->head[ 3 ], 11, body->objs, 30, -30, 30, -30, 0, 0 );
	/* ＩＫ起点関節（ダミー用） */
	SetJointInfo( &work->head[ 4 ], 12, body->objs, 0, 0, 0, 0, 0, 0 );
	/* ＩＫ起点座標の設定 */
	InitIkTarget( work, &work->trg_head, (FVECTOR*)body->objs->objs[12].world.m[3] );
	/* ＩＫ状態の初期化 */
	CalcIK( work->head, &work->trg_head.pos, body->objs, 3 );
#endif


	work->old_ik_mode = -1 ;
	return (work);
}


