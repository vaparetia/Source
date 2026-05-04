/*
	MTNサンプリングコンバートルーチン
 */
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "gte.h"
#include "quat.h"
#include "motion.h"


/* ---------------------------------------------------------------- */
/* 指定したフレームのデータを取得 */
static void SetMoveData( FVECTOR *move, MTN_MOTION *motion, int frame )
{
	if ( motion->move == NULL ) return ;
	if ( frame >= motion->header.motion_length ) return ;
	if ( frame < -1 ) return ;
	motion->move[ frame + 1 ] = *move ;
}

static void SetRotData( FVECTOR *rot, MTN_MOTION *motion, int joint, int frame )
{
	if ( ( motion->rots == NULL ) || ( joint > motion->header.motion_joints ) ) return ;
	if ( frame >= motion->header.motion_length ) return ;
	if ( frame < 0 ) return ;
	motion->rots[ joint * ( motion->header.motion_length + 1 ) + frame + 1 ] = *rot ;
	/* 先頭のダミーフレームのデータ書き込みに関するチェック */
	if ( motion->header.flags & MTN_FLAG_LOOP ){
		if ( frame == ( motion->header.motion_length - 1 ) ){
			motion->rots[ joint * ( motion->header.motion_length + 1 ) + 0 ] = *rot ;
		}
	} else {
		if ( frame == 0 ){
			motion->rots[ joint * ( motion->header.motion_length + 1 ) + 0 ] = *rot ;
		}
	}
}

static void SetTransData( FVECTOR *trans, MTN_MOTION *motion, int joint, int frame )
{
	if ( ( motion->rots == NULL ) || ( joint > motion->header.motion_joints ) ) return ;
	if ( frame >= motion->header.motion_length ) return ;
	if ( frame < 0 ) return ;
	motion->trans[ joint * ( motion->header.motion_length + 1 ) + frame + 1 ] = *trans ;
	/* 先頭のダミーフレームのデータ書き込みに関するチェック */
	if ( motion->header.flags & MTN_FLAG_LOOP ){
		if ( frame == ( motion->header.motion_length - 1 ) ){
			motion->trans[ joint * ( motion->header.motion_length + 1 ) + 0 ] = *trans ;
		}
	} else {
		if ( frame == 0 ){
			motion->trans[ joint * ( motion->header.motion_length + 1 ) + 0 ] = *trans ;
		}
	}
}

/* ---------------------------------------------------------------- */
/* 指定したフレームのデータを取得 */
static void GetMoveData( FVECTOR *move, MTN_MOTION *motion, int frame )
{
	if ( frame >= motion->header.motion_length ) frame = motion->header.motion_length - 1 ;
	if ( frame < 0 ) frame = -1 ;
	if ( motion->move == NULL ){
		move->vx = 0 ; move->vy = 0 ; move->vz = 0 ; move->vw = 1.0f ;
		return ;
	}
	*move = motion->move[ frame + 1 ] ;
}

static void GetRotData( FVECTOR *rot, MTN_MOTION *motion, int joint, int frame )
{
	if ( frame >= motion->header.motion_length ) frame = motion->header.motion_length - 1 ;
	if ( frame < 0 ) {
		if ( motion->header.flags & MTN_FLAG_LOOP ){
			frame += motion->header.motion_length ;
		} else {
			frame = -1 ;
		}
	}
	if ( ( motion->rots == NULL ) || ( joint > motion->header.motion_joints ) ){
		rot->vx = 0 ; rot->vy = 0 ; rot->vz = 0 ; rot->vw = 1.0f ;
		return ;
	}
	*rot = motion->rots[ joint * ( motion->header.motion_length + 1 ) + frame + 1 ] ;
}

static void GetTransData( FVECTOR *trans, MTN_MOTION *motion, int joint, int frame )
{
	if ( frame >= motion->header.motion_length ) frame = motion->header.motion_length - 1 ;
	if ( frame < 0 ) {
		if ( motion->header.flags & MTN_FLAG_LOOP ){
			frame += motion->header.motion_length ;
		} else {
			frame = -1 ;
		}
	}
	if ( ( motion->trans == NULL ) || ( joint > motion->header.motion_joints ) ){
		trans->vx = 0 ; trans->vy = 0 ; trans->vz = 0 ; trans->vw = 1.0f ;
		return ;
	}
	*trans = motion->trans[ joint * ( motion->header.motion_length + 1 ) + frame + 1 ] ;
}

/* ---------------------------------------------------------------- */
/* ２フレーム間の値を補間したものを取得 */
static void GetMoveDataBetween( FVECTOR *move, MTN_MOTION *motion, int frame0, int frame1, float t )
{
	FVECTOR		tmp1, tmp2, dvec0, dvec1, mov0, mov1 ;

	GetMoveData( &mov0, motion, frame0 );
	GetMoveData( &mov1, motion, frame1 );

	GetMoveData( &tmp1, motion, frame0 - 1 );
	GetMoveData( &tmp2, motion, frame0 + 1 );
	dvec0.vx = ( tmp2.vx - tmp1.vx ) * 0.5f ;
	dvec0.vy = ( tmp2.vy - tmp1.vy ) * 0.5f ;
	dvec0.vz = ( tmp2.vz - tmp1.vz ) * 0.5f ;
	dvec0.vw = ( tmp2.vw - tmp1.vw ) * 0.5f ;

	GetMoveData( &tmp1, motion, frame1 - 1 );
	GetMoveData( &tmp2, motion, frame1 + 1 );
	dvec1.vx = ( tmp2.vx - tmp1.vx ) * 0.5f ;
	dvec1.vy = ( tmp2.vy - tmp1.vy ) * 0.5f ;
	dvec1.vz = ( tmp2.vz - tmp1.vz ) * 0.5f ;
	dvec1.vw = ( tmp2.vw - tmp1.vw ) * 0.5f ;

	GTE_HermiteLerpVec( move, &mov0, &mov1, &dvec0, &dvec1, t );
}

static void GetRotDataBetween( FVECTOR *rot, MTN_MOTION *motion, int joint, int frame0, int frame1, float t )
{
	FVECTOR		tmp0, tmp1, tmp2, crot0, crot1, rot0, rot1 ;

	GetRotData( &rot0, motion, joint, frame0 );
	GetRotData( &rot1, motion, joint, frame1 );

	GetRotData( &tmp0, motion, joint, frame0 - 1 );
	tmp1 = rot0 ;
	GetRotData( &tmp2, motion, joint, frame0 + 1 );
	MT_QuatQCompA( &crot0, &tmp0, &tmp1, &tmp2 );

	GetRotData( &tmp0, motion, joint, frame1 - 1 );
	tmp1 = rot1 ;
	GetRotData( &tmp2, motion, joint, frame1 + 1 );
	MT_QuatQCompA( &crot1, &tmp0, &tmp1, &tmp2 );

	MT_QuatSquad( rot, &rot0, &crot0, &crot1, &rot1, t );
}

static void GetTransDataBetween( FVECTOR *trans, MTN_MOTION *motion, int joint, int frame0, int frame1, float t )
{
	FVECTOR		tmp1, tmp2, dvec0, dvec1, trans0, trans1 ;

	GetTransData( &trans0, motion, joint, frame0 );
	GetTransData( &trans1, motion, joint, frame1 );

	GetTransData( &tmp1, motion, joint, frame0 - 1 );
	GetTransData( &tmp2, motion, joint, frame0 + 1 );
	dvec0.vx = ( tmp2.vx - tmp1.vx ) * 0.5f ;
	dvec0.vy = ( tmp2.vy - tmp1.vy ) * 0.5f ;
	dvec0.vz = ( tmp2.vz - tmp1.vz ) * 0.5f ;
	dvec0.vw = ( tmp2.vw - tmp1.vw ) * 0.5f ;

	GetTransData( &tmp1, motion, joint, frame1 - 1 );
	GetTransData( &tmp2, motion, joint, frame1 + 1 );
	dvec1.vx = ( tmp2.vx - tmp1.vx ) * 0.5f ;
	dvec1.vy = ( tmp2.vy - tmp1.vy ) * 0.5f ;
	dvec1.vz = ( tmp2.vz - tmp1.vz ) * 0.5f ;
	dvec1.vw = ( tmp2.vw - tmp1.vw ) * 0.5f ;

	GTE_HermiteLerpVec( trans, &trans0, &trans1, &dvec0, &dvec1, t );
}




/* ---------------------------------------------------------------- */
	/*
		サンプリングレートを変更したモーションを生成する
	 */
MTN_MOTION *MTN_SamplingRateConvert( MTN_MOTION *org_motion, int new_tick, int flag )
{
	MTN_MOTION	*new_motion ;
	int			total_time, new_frame, old_tick ;
	int			i, j, count, old_frame0, old_frame1, mod ;
	FVECTOR		move, trans, rot ;
	float		t_div ;

	/* 旧レートと新レートが同じ場合は複製するだけ（劣化を押えるため） */
	old_tick = org_motion->header.motion_tick ;
	t_div = 1.0f / (float)old_tick ;
	if ( old_tick == new_tick ){
		new_motion = MTN_DuplicateMotion( org_motion ) ;
		return ( new_motion );
	}

	/* 1/300秒単位でのモーション長を求める */
	total_time = org_motion->header.motion_tick * org_motion->header.motion_length ;

	/* サンプリングレート変更後のモーションを格納するメモリを確保 */
	new_frame = ( total_time + new_tick - 1 ) / new_tick ;
	new_motion = MTN_AllocMotion( org_motion->header.motion_joints, new_frame );
	new_motion->header.flags = org_motion->header.flags ;

	/* 移動量の変更 */
	for ( i = 0 ; i < new_frame ; i++ ){
		count = i * new_tick ;
		mod =  count % old_tick ;
		if ( mod == 0 ){
			/* 端数がない場合には該当フレームのデータをコピーする */
			old_frame0 = count / old_tick ;
			GetMoveData( &move, org_motion, old_frame0 );
			SetMoveData( &move, new_motion, i );
		} else {
			/* 端数がある場合には該当フレーム前後のデータから補間して求める */
			old_frame0 = count / old_tick ;
			old_frame1 = old_frame0 + 1 ;
			GetMoveDataBetween( &move, org_motion, old_frame0, old_frame1, (float)mod * t_div );
			SetMoveData( &move, new_motion, i );
		}
		if ( i == 0 ) SetMoveData( &move, new_motion, -1 );
	}

	/* 関節回転情報の変更 */
	for ( j = 0 ; j < org_motion->header.motion_joints ; j++ ){
		for ( i = 0 ; i < new_frame ; i++ ){
			count = i * new_tick ;
			mod = count % old_tick ;
			if ( mod == 0 ){
				/* 端数がない場合には該当フレームのデータをコピーする */
				old_frame0 = count / old_tick ;
				GetRotData( &move, org_motion, j, old_frame0 );
				SetRotData( &move, new_motion, j, i );
			} else {
				/* 端数がある場合には該当フレーム前後のデータから補間して求める */
				old_frame0 = count / old_tick ;
				old_frame1 = old_frame0 + 1 ;
				GetRotDataBetween( &move, org_motion, j, old_frame0, old_frame1, (float)mod * t_div );
				SetRotData( &move, new_motion, j, i );
			}
		}
	}

	/* 関節移動量情報の変更 */
	for ( j = 0 ; j < org_motion->header.motion_joints ; j++ ){
		for ( i = 0 ; i < new_frame ; i++ ){
			count = i * new_tick ;
			mod = count % old_tick ;
			if ( mod == 0 ){
				/* 端数がない場合には該当フレームのデータをコピーする */
				old_frame0 = count / old_tick ;
				GetTransData( &move, org_motion, j, old_frame0 );
				SetTransData( &move, new_motion, j, i );
			} else {
				/* 端数がある場合には該当フレーム前後のデータから補間して求める */
				old_frame0 = count / old_tick ;
				old_frame1 = old_frame0 + 1 ;
				GetTransDataBetween( &move, org_motion, j, old_frame0, old_frame1, (float)mod * t_div );
				SetTransData( &move, new_motion, j, i );
			}
		}
	}

	return ( new_motion );
}



