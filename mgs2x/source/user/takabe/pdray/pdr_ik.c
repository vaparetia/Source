//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	pdr_ik.c
	量産型ＲＡＹ用ＩＫ制御ルーチン

	2001/07/23 K.Takabe
	$Id: pdr_ik.c,v 1.1.1.3 2002/11/19 11:51:24 Yoshizawa1 Exp $

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

#include	"libdg.h"
#include	"libdg.cnf"
#include	"def_dma.h"
#include	"libmt.h"
#include	"gameheader.h"
#include	"debugmenu.h"


#include	"../other/vec_util.h"

#include "r_common.h"
#include "pdray.h"

/* ---------------------------------------------------------------- */
/* 絶対回転モーションデータを相対回転データに変換 */
static void ConvertRotate_Absolute2Relative( FVECTOR *rots, FVECTOR *abs_rots, DG_OBJS *objs, int max )
{
	DG_OBJ		*obj ;
	FVECTOR		inv_quat, *src, *dst, *parent_rot ;
	int			i ;

	obj = objs->objs ;
	src = abs_rots ;
	dst = rots ;
	for ( i = 0 ; i < max ; i++, obj++, src++, dst++ ){
		if ( obj->parent == -1 ){
			*dst = *src ;
		} else {
			parent_rot = abs_rots + obj->parent ;
			inv_quat.vx = - parent_rot->vx ;
			inv_quat.vy = - parent_rot->vy ;
			inv_quat.vz = - parent_rot->vz ;
			inv_quat.vw =   parent_rot->vw ;
			MT_QuatMul( dst, &inv_quat, src );
		}
	}
}
/* 相対回転モーションデータを絶対回転データに変換 */
static void ConvertRotate_Relative2Absolute( FVECTOR *abs_rots, FVECTOR *rots, DG_OBJS *objs, int max )
{
	DG_OBJ		*obj ;
	FVECTOR		*src, *dst, *parent_rot ;
	int			i ;

	obj = objs->objs ;
	dst = abs_rots ;
	src = rots ;
	for ( i = 0 ; i < max ; i++, obj++, src++, dst++ ){
		if ( obj->parent == -1 ){
			*dst = *src ;
		} else {
			parent_rot = abs_rots + obj->parent ;
			MT_QuatMul( dst, parent_rot, src );
		}
	}
}
/* ---------------------------------------------------------------- */
/* オブジェクトのマトリクス計算 */
static void SetMotion( DG_OBJS *objs, FVECTOR *rots )
{
#if 1
	FMATRIX		*matrix, *mats ;
	DG_OBJ		*obj ;
	int			i ;

	matrix = SCRPAD_ADDR ;
	mats = &matrix[ 1 ] ;
	matrix[ 0 ] = objs->world ;
	obj = objs->objs ;
	for ( i = 0 ; i < PDRAY_MAX_JOINTS ; i++ ){
#if 0
		/* 相対回転クォータニオンからマトリクス生成 */
		MT_QuatToMat( mats, rots );
		*(FVECTOR*)mats->m[3] = obj->trans ;
		GTE_MulMatrix( mats, &matrix[ obj->parent + 1 ], mats );
#else
		/* 絶対回転クォータニオンからマトリクス生成 */
		MT_QuatToMat( mats, rots );
		GTE_MulMatrix( mats, matrix, mats );
		GTE_ApplyMatrix( (FVECTOR*)mats->m[3], &matrix[ obj->parent + 1 ], &obj->trans );
#endif
		obj->world = *mats ;
		mats++ ;
		obj++ ;
		rots++ ;
	}
#endif
}

/* ---------------------------------------------------------------- */
/* チャフの混乱パラメータアニメーション */
static void ActChaffConfusion( Work *work )
{
	CONTROL		*ctrl ;
	OBJECT		*body ;
	int			i ;

	ctrl = &work->control ;
	body = &work->object ;

	/* ＩＫぶらし値＆補完時間決定 */
	for ( i = 0 ; i < 4 ; i++ ){
		float		t ;
		if ( work->confusion_offset_count[i] <= 0 ){
			/* 新規オフセットと補完時間決定 */
			work->confusion_offset_trg[i].vx = RND( 256 ) - 128 ;
			work->confusion_offset_trg[i].vy = RND( 256 ) - 128 ;
			work->confusion_offset_trg[i].vz = RND( 256 ) - 128 ;
			work->confusion_offset_trg[i].vw = GTE_PS2RAD( RND( 8 ) ) ;
			/* ０番（腰）以外はクォータニオンなので */
			if ( i != 0 ){
				FVECTOR		tmp_vec ;
				tmp_vec = work->confusion_offset_trg[i] ;
				MT_QuatSetValue( &work->confusion_offset_trg[i], &tmp_vec );
			}
			work->confusion_offset_count[i] = RND( 2 ) + 1 ;
		}
		/* オフセットの補間処理 */
		t = 1.0f / (float)work->confusion_offset_count[i] ;
		if ( i == 0 ){
			/* ０番（腰）の場合（移動量の補間） */
			GTE_InterVector( &work->confusion_offset[i],
							&work->confusion_offset[i], &work->confusion_offset_trg[i], t );
		} else {
			/* ０番（腰）以外の場合（クォータニオンの補間） */
			MT_QuatSlerp( &work->confusion_offset[i], &work->confusion_offset[i], &work->confusion_offset_trg[i], t ); 
			MT_QuatNormalize( &work->confusion_offset[i], &work->confusion_offset[i] );
		}
		work->confusion_offset_count[i]-- ;
	}

}
/* ---------------------------------------------------------------- */
	/*
		メイン処理
	*/
void PDRAY_ActIk( Work *work )
{
	CONTROL		*ctrl ;
	OBJECT		*body ;
	int			i, chaff_enable_flag = 0 ;

	ctrl = &work->control ;
	body = &work->object ;

	/* チャフ混乱パラメータ関連 */
	if ( work->active_flag && ( GM_GameStatus & STATE_CHAFF ) ){
		ActChaffConfusion( work );
		chaff_enable_flag = 1 ;
	}

	if ( work->active_flag ){
		if ( !work->chaff_flag ){/* 頭部の注目点更新 */
			//FVECTOR		tmp_vec ;
			if ( work->head_look_flag == 0 ){
				work->head_look_pos = GM_PlayerPosition ;/* デバッグ */
			}
			if ( work->disable_head_ik_flag ){
				FVECTOR		tmp_vec ;
				float		len ;
				GTE_SubVector( &tmp_vec, &work->head_look_pos,
							  (FVECTOR*)body->objs->objs[ PDRAY_JOINT_HEAD ].world.m[3] );
				len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) ) ;
				GTE_SetVector( &tmp_vec, 0, 0, len, 1 );
				GTE_LoadMatrix( &body->objs->objs[ PDRAY_JOINT_HEAD ].world );
				GTE_PutVector1( &work->head_look_pos, &tmp_vec );
			}
			//GTE_AddVector( &tmp_vec, &work->head_look_pos, &work->head_look_old_pos );
			//GTE_ScaleVector( &work->head_look_old_pos, &tmp_vec, 0.5f );
			GTE_InterVector( &work->head_look_old_pos, &work->head_look_old_pos, &work->head_look_pos, 0.5f );
		}

		/* モーション再生で生成されたモーションを相対回転形式へ変換 */
		ConvertRotate_Absolute2Relative( work->rots, body->m_ctrl->abs_rots, body->objs, PDRAY_MAX_JOINTS );

		/* このときの固定指定されていないＩＫ関節先端の座標を基準位置として記録 */
		PDRAY_RecordIKPosition( work );

#if 0
		if ( work->chaff_flag ){/* チャフ混乱時にＩＫ制御点をブラす */
			*(FVECTOR*)body->objs->world.m[3] = work->confusion_pos[0] ;
			*(FVECTOR*)body->objs->objs[PDRAY_JOINT_WAIST].world.m[3] = work->confusion_pos[0] ;
		}
#else
		if ( chaff_enable_flag ){
			body->objs->world.m[3][0] += work->confusion_offset[0].vx ;
			body->objs->world.m[3][1] += work->confusion_offset[0].vy ;
			body->objs->world.m[3][2] += work->confusion_offset[0].vz ;
			body->objs->objs[PDRAY_JOINT_WAIST].world.m[3][0] += work->confusion_offset[0].vx ;
			body->objs->objs[PDRAY_JOINT_WAIST].world.m[3][1] += work->confusion_offset[0].vy ;
			body->objs->objs[PDRAY_JOINT_WAIST].world.m[3][2] += work->confusion_offset[0].vz ;
		}
#endif

		/* 現在のモーションポーズをＩＫワークにコピーする */
		for ( i = 0 ; i < 5 ; i++ ){
			work->ik_right_leg->joints[ i ].rotate = work->rots[ work->ik_right_leg->joints[ i ].joint_num ] ;
		}
		for ( i = 0 ; i < 5 ; i++ ){
			work->ik_left_leg->joints[ i ].rotate = work->rots[ work->ik_left_leg->joints[ i ].joint_num ] ;
		}
		for ( i = 0 ; i < 2 ; i++ ){
			work->ik_head->joints[ i ].rotate = work->rots[ work->ik_head->joints[ i ].joint_num ] ;
		}
		/* ＩＫ計算用データセット */
		work->ik_right_leg->base_world = body->objs->objs[ 0 ].world ;
		work->ik_left_leg->base_world = body->objs->objs[ 0 ].world ;
		work->ik_right_leg->target_pos = work->right_leg_target_pos ;
		work->ik_left_leg->target_pos = work->left_leg_target_pos ;
		/* ＩＫ処理 */
		if ( work->on_stage_flag ){
			/* ステージ上に乗っている場合にはＩＫ計算精度を上げる */
			work->ik_right_leg->n_loop = 6 ;
			work->ik_left_leg->n_loop = 6 ;
		} else {
			/* 通常時はあまりＩＫ計算精度を上げない */
			work->ik_right_leg->n_loop = 3 ;
			work->ik_left_leg->n_loop = 3 ;
		}
		if ( work->right_leg_ik_mode != IK_MODE_FREE ) MT_CalcInversKinematic( work->ik_right_leg );
		if ( work->left_leg_ik_mode != IK_MODE_FREE ) MT_CalcInversKinematic( work->ik_left_leg );
		/* ＩＫ結果をモデルに反映させる */
		for ( i = 0 ; i < 5 ; i++ ){
			work->rots[ work->ik_right_leg->joints[ i ].joint_num ] = work->ik_right_leg->joints[ i ].rotate ;
		}
		for ( i = 0 ; i < 5 ; i++ ){
			work->rots[ work->ik_left_leg->joints[ i ].joint_num ] = work->ik_left_leg->joints[ i ].rotate ;
		}
		/* 首の角度を調整する */
		if ( work->disable_head_ik_flag == 0 || 1 ){
			/* ＩＫ計算用データセット */
			work->ik_head->base_world = body->objs->objs[ PDRAY_JOINT_BUST ].world ;
			work->ik_head->target_pos = work->head_look_old_pos ;
			GTE_ZeroVector( &work->ik_head->joints[ 0 ].rotate );/* モーションの角度影響を受けないように首の角度を初期化 */
			MT_CalcInversKinematic( work->ik_head );
			if ( work->disable_head_ik_flag == 0 ){
				/* ＩＫ結果をモデルに反映させる */
				for ( i = 0 ; i < 1 ; i++ ){
					work->rots[ work->ik_head->joints[ i ].joint_num ] = work->ik_head->joints[ i ].rotate ;
				}
				work->rots[ PDRAY_JOINT_HEAD ] = DG_ZeroVector ;
			}
		}
		{/* 腕ＩＫ処理 */
			//static FVECTOR	x_plus = {100,0,0,1}, x_minus = {-100,0,0,1};
			FVECTOR	tmp_vec, old_pos ;
			//float	len ;
			/* 右腕 */
			old_pos = work->right_arm_target_pos ;
			switch ( work->right_arm_ik_mode ){
			  case 0:
				work->right_arm_target_pos = *(FVECTOR*)body->objs->objs[ PDRAY_JOINT_RIGHT_HAND ].world.m[3] ;
				break ;
			  case 1:/* プレイヤー方向に補正 */
				//GTE_InterVector( &tmp_vec, &work->right_arm_target_pos, &GM_PlayerPosition, 1.0f/16.0f );
				GTE_InterVector( &tmp_vec, &work->right_arm_target_pos, &work->attack_pos, 0.25f );
				work->right_arm_target_pos = tmp_vec ;
				break ;
			  case 2:/* 通常方向に補正 */
#if 0
				GTE_LoadMatrix( &body->objs->objs[ PDRAY_JOINT_RIGHT_HAND ].world );
				GTE_PutVector1( &tmp_vec, &x_minus );
				GTE_InterVector( &tmp_vec, &work->right_arm_target_pos, &tmp_vec, 0.25f );
				work->right_arm_target_pos = tmp_vec ;
				/* 差分を計算する */
				GTE_SubVector( &tmp_vec, &old_pos, &tmp_vec );
				len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
				if ( len < 100.0f ) work->right_arm_ik_mode = 0 ;
#else
				work->right_arm_ik_mode = 0 ;
#endif
				break ;
			}
			if ( work->chaff_flag ) work->right_arm_target_pos = old_pos ;
			if ( work->right_arm_ik_mode ){
				/* 現在のモーションポーズをＩＫワークにコピーする */
				for ( i = 0 ; i < 3 ; i++ ){
					work->ik_right_arm->joints[ i ].rotate = work->rots[ work->ik_right_arm->joints[ i ].joint_num ] ;
				}
				/* ＩＫ計算用データセット */
				work->ik_right_arm->base_world = body->objs->objs[ 2 ].world ;
				work->ik_right_arm->target_pos = work->right_arm_target_pos ;
				/* ＩＫ計算 */
				MT_CalcInversKinematic( work->ik_right_arm );
				/* ＩＫ結果をモデルに反映させる */
				for ( i = 0 ; i < 3 ; i++ ){
					work->rots[ work->ik_right_arm->joints[ i ].joint_num ] = work->ik_right_arm->joints[ i ].rotate ;
				}
			}
			/* 左腕 */
			old_pos = work->left_arm_target_pos ;
			switch ( work->left_arm_ik_mode ){
			  case 0:
				work->left_arm_target_pos = *(FVECTOR*)body->objs->objs[ PDRAY_JOINT_LEFT_HAND ].world.m[3] ;
				break ;
			  case 1:/* プレイヤー方向に補正 */
				//GTE_InterVector( &tmp_vec, &work->left_arm_target_pos, &GM_PlayerPosition, 1.0f/16.0f );
				//GTE_InterVector( &tmp_vec, &work->left_arm_target_pos, &GM_PlayerPosition, 0.125f );
				GTE_InterVector( &tmp_vec, &work->left_arm_target_pos, &work->attack_pos, 0.25f );
				work->left_arm_target_pos = tmp_vec ;
				break ;
			  case 2:/* 通常方向に補正 */
#if 0
				GTE_LoadMatrix( &body->objs->objs[ PDRAY_JOINT_LEFT_HAND ].world );
				GTE_PutVector1( &tmp_vec, &x_plus );
				GTE_InterVector( &tmp_vec, &work->left_arm_target_pos, &tmp_vec, 0.25f );
				work->left_arm_target_pos = tmp_vec ;
				/* 差分を計算する */
				GTE_SubVector( &tmp_vec, &old_pos, &tmp_vec );
				len = DG_SQRT( GTE_InnerProduct( &tmp_vec, &tmp_vec ) );
				if ( len < 100.0f ) work->left_arm_ik_mode = 0 ;
#else
				work->left_arm_ik_mode = 0 ;
#endif
				break ;
			}
			if ( work->chaff_flag ) work->left_arm_target_pos = old_pos ;
			if ( work->left_arm_ik_mode ){
				/* 現在のモーションポーズをＩＫワークにコピーする */
				for ( i = 0 ; i < 3 ; i++ ){
					work->ik_left_arm->joints[ i ].rotate = work->rots[ work->ik_left_arm->joints[ i ].joint_num ] ;
				}
				/* ＩＫ計算用データセット */
				work->ik_left_arm->base_world = body->objs->objs[ 2 ].world ;
				work->ik_left_arm->target_pos = work->left_arm_target_pos ;
				/* ＩＫ計算 */
				MT_CalcInversKinematic( work->ik_left_arm );
				/* ＩＫ結果をモデルに反映させる */
				for ( i = 0 ; i < 3 ; i++ ){
					work->rots[ work->ik_left_arm->joints[ i ].joint_num ] = work->ik_left_arm->joints[ i ].rotate ;
				}
			}
		}

		/* 混乱時の各関節調整 */
		if ( chaff_enable_flag ){
			MT_QuatMul( &work->rots[ PDRAY_JOINT_NECK ], &work->rots[ PDRAY_JOINT_NECK ], &work->confusion_offset[1] );
			MT_QuatMul( &work->rots[ 3 ], &work->rots[ 3 ], &work->confusion_offset[2] );
			MT_QuatMul( &work->rots[ 8 ], &work->rots[ 8 ], &work->confusion_offset[3] );
		}

		{/* つま先だけ角度を合わせる */
			//FVECTOR		*tmp_abs_rots = SCRPAD_ADDR ;
			FVECTOR		*tmp_abs_rots = work->abs_rots ;
			ConvertRotate_Relative2Absolute( tmp_abs_rots, work->rots, body->objs, PDRAY_MAX_JOINTS );
			tmp_abs_rots[ PDRAY_JOINT_RIGHT_TOE ] = body->m_ctrl->abs_rots[ PDRAY_JOINT_RIGHT_TOE ] ;
			tmp_abs_rots[ PDRAY_JOINT_LEFT_TOE ] = body->m_ctrl->abs_rots[ PDRAY_JOINT_LEFT_TOE ] ;
			tmp_abs_rots[ PDRAY_JOINT_RIGHT_ANKLE ] = body->m_ctrl->abs_rots[ PDRAY_JOINT_RIGHT_ANKLE ] ;
			tmp_abs_rots[ PDRAY_JOINT_LEFT_ANKLE ] = body->m_ctrl->abs_rots[ PDRAY_JOINT_LEFT_ANKLE ] ;
			//ConvertRotate_Absolute2Relative( work->rots, tmp_abs_rots, body->objs, PDRAY_MAX_JOINTS );

			/* モーション設定 */
			SetMotion( body->objs, tmp_abs_rots );
		}
	}

}
/* ---------------------------------------------------------------- */
	/*
		終了
	*/
void PDRAY_EndIk( Work *work )
{
	MT_FreeIKControl( work->ik_right_leg );
	MT_FreeIKControl( work->ik_left_leg );
	MT_FreeIKControl( work->ik_head );
	MT_FreeIKControl( work->ik_right_arm );
	MT_FreeIKControl( work->ik_left_arm );
}
/* ---------------------------------------------------------------- */
	/*
		初期化処理
	*/
void PDRAY_InitIk( Work *work )
{
	CONTROL		*ctrl ;
	OBJECT		*body ;
	int			i ;

	ctrl = &work->control ;
	body = &work->object ;

	/* ＩＫデバッグ */
	body->objs->rots = work->rots ;
	for ( i = 0 ; i < PDRAY_MAX_JOINTS ; i++ ){
		work->rots[ i ] = DG_ZeroVector ;
	}

#if 0
	/* ＩＫ関連初期化 */
	work->ik_right_leg = MT_MakeIKControl( 0, 5, body->objs );
	work->ik_left_leg = MT_MakeIKControl( 0, 5, body->objs );
	work->ik_head = MT_MakeIKControl( 0, 1, body->objs );
	{/* ＩＫ関節情報の設定 */
		extern void MT_ConfigIKJointParamX( MT_IK_CONTROL *ik_ctrl, int n_joint, float max, float min );
		extern void MT_ConfigIKJointParamY( MT_IK_CONTROL *ik_ctrl, int n_joint, float max, float min );
		extern void MT_ConfigIKJointParamZ( MT_IK_CONTROL *ik_ctrl, int n_joint, float max, float min );
		MT_ConfigIKJointObj( work->ik_right_leg, 0, 21 );
		MT_ConfigIKJointObj( work->ik_right_leg, 1, 22 );
		MT_ConfigIKJointObj( work->ik_right_leg, 2, 23 );
		MT_ConfigIKJointObj( work->ik_right_leg, 3, 24 );
		MT_ConfigIKJointObj( work->ik_right_leg, 4, 25 );
		MT_ConfigIKJointObj( work->ik_right_leg, 5, 26 );
		MT_ConfigIKJointParamX( work->ik_right_leg, 0, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamY( work->ik_right_leg, 0, DEG2RAD(30), DEG2RAD(-30) );
		MT_ConfigIKJointParamZ( work->ik_right_leg, 0, DEG2RAD(30), DEG2RAD(-30) );
		MT_ConfigIKJointParamX( work->ik_right_leg, 1, DEG2RAD(70), DEG2RAD(-70) );
		MT_ConfigIKJointParamY( work->ik_right_leg, 1, DEG2RAD(30), DEG2RAD(-30) );
		MT_ConfigIKJointParamZ( work->ik_right_leg, 1, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamX( work->ik_right_leg, 2, DEG2RAD(90), DEG2RAD(-70) );
		MT_ConfigIKJointParamY( work->ik_right_leg, 2, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamZ( work->ik_right_leg, 2, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamX( work->ik_right_leg, 3, DEG2RAD(70), DEG2RAD(-90) );
		MT_ConfigIKJointParamY( work->ik_right_leg, 3, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamZ( work->ik_right_leg, 3, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamX( work->ik_right_leg, 4, DEG2RAD(20), DEG2RAD(-20) );
		MT_ConfigIKJointParamY( work->ik_right_leg, 4, DEG2RAD(0), DEG2RAD(0) );
		//MT_ConfigIKJointParamZ( work->ik_right_leg, 4, DEG2RAD(30), DEG2RAD(-30) );
		MT_ConfigIKJointParamZ( work->ik_right_leg, 4, DEG2RAD(0), DEG2RAD(0) );

		MT_ConfigIKJointObj( work->ik_left_leg, 0, 27 );
		MT_ConfigIKJointObj( work->ik_left_leg, 1, 28 );
		MT_ConfigIKJointObj( work->ik_left_leg, 2, 29 );
		MT_ConfigIKJointObj( work->ik_left_leg, 3, 30 );
		MT_ConfigIKJointObj( work->ik_left_leg, 4, 31 );
		MT_ConfigIKJointObj( work->ik_left_leg, 5, 32 );
		MT_ConfigIKJointParamX( work->ik_left_leg, 0, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamY( work->ik_left_leg, 0, DEG2RAD(30), DEG2RAD(-30) );
		MT_ConfigIKJointParamZ( work->ik_left_leg, 0, DEG2RAD(30), DEG2RAD(-30) );
		MT_ConfigIKJointParamX( work->ik_left_leg, 1, DEG2RAD(70), DEG2RAD(-70) );
		MT_ConfigIKJointParamY( work->ik_left_leg, 1, DEG2RAD(30), DEG2RAD(-30) );
		MT_ConfigIKJointParamZ( work->ik_left_leg, 1, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamX( work->ik_left_leg, 2, DEG2RAD(90), DEG2RAD(-70) );
		MT_ConfigIKJointParamY( work->ik_left_leg, 2, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamZ( work->ik_left_leg, 2, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamX( work->ik_left_leg, 3, DEG2RAD(70), DEG2RAD(-90) );
		MT_ConfigIKJointParamY( work->ik_left_leg, 3, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamZ( work->ik_left_leg, 3, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamX( work->ik_left_leg, 4, DEG2RAD(20), DEG2RAD(-20) );
		MT_ConfigIKJointParamY( work->ik_left_leg, 4, DEG2RAD(0), DEG2RAD(0) );
		//MT_ConfigIKJointParamZ( work->ik_left_leg, 4, DEG2RAD(30), DEG2RAD(-30) );
		MT_ConfigIKJointParamZ( work->ik_left_leg, 4, DEG2RAD(0), DEG2RAD(0) );

		MT_ConfigIKJointObj( work->ik_head, 0, PDRAY_JOINT_NECK );
		MT_ConfigIKJointObj( work->ik_head, 1, PDRAY_JOINT_NECK+1 );
		/* 強制的に最終関節のオフセットを設定 */
		work->ik_head->joints[ 1 ].trans.vx = 0.0f ;
		work->ik_head->joints[ 1 ].trans.vy = 0.0f ;
		work->ik_head->joints[ 1 ].trans.vz = 1.0f ;
		work->ik_head->joints[ 1 ].trans.vw = 1.0f ;
		MT_ConfigIKJointParamX( work->ik_head, 0, DEG2RAD(40), DEG2RAD(-40) );
		MT_ConfigIKJointParamY( work->ik_head, 0, DEG2RAD(40), DEG2RAD(-40) );
		MT_ConfigIKJointParamZ( work->ik_head, 0, DEG2RAD(40), DEG2RAD(-40) );
	}
#else
	/* ＩＫ関連初期化 */
	work->ik_right_leg = MT_MakeIKControl( 0, 4, body->objs );
	work->ik_left_leg = MT_MakeIKControl( 0, 4, body->objs );
	work->ik_head = MT_MakeIKControl( 0, 1, body->objs );
	work->ik_right_arm = MT_MakeIKControl( 0, 3, body->objs );
	work->ik_left_arm = MT_MakeIKControl( 0, 3, body->objs );
	work->ik_right_leg->n_loop = 3 ;
	work->ik_left_leg->n_loop = 3 ;
	work->ik_head->n_loop = 2 ;
	work->ik_right_arm->n_loop = 3 ;
	work->ik_left_arm->n_loop = 3 ;
	{/* ＩＫ関節情報の設定 */
		extern void MT_ConfigIKJointParamX( MT_IK_CONTROL *ik_ctrl, int n_joint, float max, float min );
		extern void MT_ConfigIKJointParamY( MT_IK_CONTROL *ik_ctrl, int n_joint, float max, float min );
		extern void MT_ConfigIKJointParamZ( MT_IK_CONTROL *ik_ctrl, int n_joint, float max, float min );
		MT_ConfigIKJointObj( work->ik_right_leg, 0, 21 );
		MT_ConfigIKJointObj( work->ik_right_leg, 1, 22 );
		MT_ConfigIKJointObj( work->ik_right_leg, 2, 23 );
		MT_ConfigIKJointObj( work->ik_right_leg, 3, 24 );
		MT_ConfigIKJointObj( work->ik_right_leg, 4, 25 );
		MT_ConfigIKJointParamX( work->ik_right_leg, 0, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamY( work->ik_right_leg, 0, DEG2RAD(30), DEG2RAD(-30) );
		MT_ConfigIKJointParamZ( work->ik_right_leg, 0, DEG2RAD(30), DEG2RAD(-30) );
		MT_ConfigIKJointParamX( work->ik_right_leg, 1, DEG2RAD(90), DEG2RAD(-90) );
		MT_ConfigIKJointParamY( work->ik_right_leg, 1, DEG2RAD(30), DEG2RAD(-30) );
		MT_ConfigIKJointParamZ( work->ik_right_leg, 1, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamX( work->ik_right_leg, 2, DEG2RAD(90), DEG2RAD(-70) );
		MT_ConfigIKJointParamY( work->ik_right_leg, 2, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamZ( work->ik_right_leg, 2, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamX( work->ik_right_leg, 3, DEG2RAD(70), DEG2RAD(-90) );
		MT_ConfigIKJointParamY( work->ik_right_leg, 3, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamZ( work->ik_right_leg, 3, DEG2RAD(0), DEG2RAD(0) );

		MT_ConfigIKJointObj( work->ik_left_leg, 0, 27 );
		MT_ConfigIKJointObj( work->ik_left_leg, 1, 28 );
		MT_ConfigIKJointObj( work->ik_left_leg, 2, 29 );
		MT_ConfigIKJointObj( work->ik_left_leg, 3, 30 );
		MT_ConfigIKJointObj( work->ik_left_leg, 4, 31 );
		MT_ConfigIKJointParamX( work->ik_left_leg, 0, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamY( work->ik_left_leg, 0, DEG2RAD(30), DEG2RAD(-30) );
		MT_ConfigIKJointParamZ( work->ik_left_leg, 0, DEG2RAD(30), DEG2RAD(-30) );
		MT_ConfigIKJointParamX( work->ik_left_leg, 1, DEG2RAD(90), DEG2RAD(-90) );
		MT_ConfigIKJointParamY( work->ik_left_leg, 1, DEG2RAD(30), DEG2RAD(-30) );
		MT_ConfigIKJointParamZ( work->ik_left_leg, 1, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamX( work->ik_left_leg, 2, DEG2RAD(90), DEG2RAD(-70) );
		MT_ConfigIKJointParamY( work->ik_left_leg, 2, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamZ( work->ik_left_leg, 2, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamX( work->ik_left_leg, 3, DEG2RAD(70), DEG2RAD(-90) );
		MT_ConfigIKJointParamY( work->ik_left_leg, 3, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamZ( work->ik_left_leg, 3, DEG2RAD(0), DEG2RAD(0) );

		MT_ConfigIKJointObj( work->ik_head, 0, PDRAY_JOINT_NECK );
		MT_ConfigIKJointObj( work->ik_head, 1, PDRAY_JOINT_NECK+1 );
		/* 強制的に最終関節のオフセットを設定 */
		work->ik_head->joints[ 1 ].trans.vx = 0.0f ;
		work->ik_head->joints[ 1 ].trans.vy = 0.0f ;
		work->ik_head->joints[ 1 ].trans.vz = 1.0f ;
		work->ik_head->joints[ 1 ].trans.vw = 1.0f ;
		MT_ConfigIKJointParamX( work->ik_head, 0, DEG2RAD(50), DEG2RAD(-50) );
		MT_ConfigIKJointParamY( work->ik_head, 0, DEG2RAD(40), DEG2RAD(-40) );
		MT_ConfigIKJointParamZ( work->ik_head, 0, DEG2RAD(60), DEG2RAD(-60) );

		/* 右腕ＩＫの初期化 */
		MT_ConfigIKJointObj( work->ik_right_arm, 0, 3 );
		MT_ConfigIKJointObj( work->ik_right_arm, 1, 5 );
		MT_ConfigIKJointObj( work->ik_right_arm, 2, 7 );
		/* 強制的に最終関節のオフセットを設定 */
		MT_ConfigIKJointObj( work->ik_right_arm, 3, 7 );
		work->ik_right_arm->joints[ 3 ].trans.vx = -100.0f ;
		work->ik_right_arm->joints[ 3 ].trans.vy = 0.0f ;
		work->ik_right_arm->joints[ 3 ].trans.vz = 0.0f ;
		work->ik_right_arm->joints[ 3 ].trans.vw = 1.0f ;
		MT_ConfigIKJointParamX( work->ik_right_arm, 0, DEG2RAD(30), DEG2RAD(-30) );
		MT_ConfigIKJointParamY( work->ik_right_arm, 0, DEG2RAD(60), DEG2RAD(-60) );
		MT_ConfigIKJointParamZ( work->ik_right_arm, 0, DEG2RAD(60), DEG2RAD(-60) );
		MT_ConfigIKJointParamX( work->ik_right_arm, 1, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamY( work->ik_right_arm, 1, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamZ( work->ik_right_arm, 1, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamX( work->ik_right_arm, 2, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamY( work->ik_right_arm, 2, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamZ( work->ik_right_arm, 2, DEG2RAD(20), DEG2RAD(-10) );

		/* 左腕ＩＫの初期化 */
		MT_ConfigIKJointObj( work->ik_left_arm, 0, 8 );
		MT_ConfigIKJointObj( work->ik_left_arm, 1, 10 );
		MT_ConfigIKJointObj( work->ik_left_arm, 2, 12 );
		/* 強制的に最終関節のオフセットを設定 */
		MT_ConfigIKJointObj( work->ik_left_arm, 3, 12 );
		work->ik_left_arm->joints[ 3 ].trans.vx = 100.0f ;
		work->ik_left_arm->joints[ 3 ].trans.vy = 0.0f ;
		work->ik_left_arm->joints[ 3 ].trans.vz = 0.0f ;
		work->ik_left_arm->joints[ 3 ].trans.vw = 1.0f ;
		MT_ConfigIKJointParamX( work->ik_left_arm, 0, DEG2RAD(30), DEG2RAD(-30) );
		MT_ConfigIKJointParamY( work->ik_left_arm, 0, DEG2RAD(60), DEG2RAD(-60) );
		MT_ConfigIKJointParamZ( work->ik_left_arm, 0, DEG2RAD(60), DEG2RAD(-60) );
		MT_ConfigIKJointParamX( work->ik_left_arm, 1, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamY( work->ik_left_arm, 1, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamZ( work->ik_left_arm, 1, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamX( work->ik_left_arm, 2, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamY( work->ik_left_arm, 2, DEG2RAD(0), DEG2RAD(0) );
		MT_ConfigIKJointParamZ( work->ik_left_arm, 2, DEG2RAD(10), DEG2RAD(-20) );

	}
#endif

	/* アイドル時の足のオフセットを求めるために一度モーションを設定する */
	GM_ConfigObjectAction( body, 0, MOTION_WALK_READY, 0, ~0, 0 );
	GM_ActMotion( body );
	ctrl->height = body->height ;
	GM_ActControl( ctrl );
	GM_ActObject2( body );

	SET_LEG_IK_MODE( IK_MODE_FREE, IK_MODE_FREE );
	PDRAY_RecordIKPosition( work );
	{/* 現在設定されている基本モーションから足のベース位置オフセットを求める */
#if 0
		FMATRIX	inv_mat ;
		FVECTOR	tmp_vec ;
		_sceVu0InversMatrix( &inv_mat, &body->objs->world );
		tmp_vec = work->right_leg_target_pos ;
		_sceVu0ApplyMatrix( &work->ik_base_right_leg_offset, &inv_mat, &tmp_vec );
		tmp_vec = work->left_leg_target_pos ;
		_sceVu0ApplyMatrix( &work->ik_base_left_leg_offset, &inv_mat, &tmp_vec );
#endif
		work->ik_base_right_leg_offset.vx = RIGHT_LEG_OFFSET_X ;
		work->ik_base_right_leg_offset.vy = RIGHT_LEG_OFFSET_Y ;
		work->ik_base_right_leg_offset.vz = RIGHT_LEG_OFFSET_Z ;
		work->ik_base_left_leg_offset.vx = LEFT_LEG_OFFSET_X ;
		work->ik_base_left_leg_offset.vy = LEFT_LEG_OFFSET_Y ;
		work->ik_base_left_leg_offset.vz = LEFT_LEG_OFFSET_Z ;
	}

	/* 変更したモーションを元に戻す */
	GM_ConfigObjectAction( body, 0, MOTION_STANDBY, 0, ~0, 0 );
	GM_ActMotion( body );
	ctrl->height = body->height ;
	GM_ActControl( ctrl );
	GM_ActObject2( body );

}

