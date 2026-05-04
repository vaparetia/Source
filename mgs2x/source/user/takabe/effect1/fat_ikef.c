//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	fat_ikef.c
	ファットマン用死体ＩＫ補助キャラ

	2001/06/10 K.Takabe
	$Id: fat_ikef.c,v 1.1.1.3 2002/11/19 11:51:10 Yoshizawa1 Exp $

*/
/*

シナリオ呼び出しインターフェイス

chara ファットマン死体ＩＫ補正[NewFatmanPuppetIKSupportSet] $s:name \
	-chara $s:ファットマンのキャラ名



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

#define FAT_JOITNS		(58)
#define FAT_JOINT_WAIST	(0)
#define FAT_JOINT_STOMACH	(1)
#define FAT_JOINT_BUST	(2)
#define FAT_JOINT_R_LEG	(13)
#define FAT_JOINT_L_LEG	(17)
#define FAT_JOINT_COAT_RF	(53)
#define FAT_JOINT_COAT_RB	(54)
#define FAT_JOINT_COAT_LF	(55)
#define FAT_JOINT_COAT_LB	(56)
#define FAT_JOINT_COAT_CF	(57)

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
	CONTROL		*ctrl ;
	OBJECT		*body ;
	DG_EVMOBJ	*evmobj ;			/* ＥＶＭオブジェクト */
} Work ;

/* ---------------------------------------------------------------- */
	/*
		ローカル関数群
	*/

/* ＥＶＭオブジェクトの一部に直接モーションを設定 */
static void EvmActMotionJoint( DG_EVMOBJ *evmobj, int n_joints, FVECTOR *skel_rot, FVECTOR *skel_trans )
{
	FMATRIX		*skel_mats ;
	FVECTOR		vec ;
	FMATRIX		mat ;
	EVM_SKEL	*skel ;
	FMATRIX	*parent ;

	skel_mats = evmobj->matrix[ evmobj->use_buffer ] ;

	/* オブジェクトにマトリクスを設定する */
	skel = &evmobj->def->skeleton[ n_joints ] ;
	/* モデル情報から親子関係を取得して求める */
	MT_QuatToMat( &mat, skel_rot );
	if ( skel_trans == NULL ){
		mat.m[3][0] = skel->rt_tx ;
		mat.m[3][1] = skel->rt_ty ;
		mat.m[3][2] = skel->rt_tz ;
	} else {
		mat.m[3][0] = skel->rt_tx + skel_trans->vx ;
		mat.m[3][1] = skel->rt_ty + skel_trans->vy ;
		mat.m[3][2] = skel->rt_tz + skel_trans->vz ;
	}
	if ( skel->parent != -1 ){
		parent = &skel_mats[ skel->parent ] ;
	} else {
		parent = &evmobj->world ;
	}
	_sceVu0MulMatrix( &mat, parent, &mat ) ;
	vec.vx = -skel->rt_tx ;
	vec.vy = -skel->rt_ty ;
	vec.vz = -skel->rt_tz ;
	vec.vw = 1.0F ;
	_sceVu0ApplyMatrix( &mat.m[3][0], &mat, &vec );
	skel_mats[n_joints] = mat ;

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
	DG_EVMOBJ	*evmobj ;
	FMATRIX		*skel_mats ;
	EVM_SKEL	*skel ;
	FVECTOR		r_leg_quat, l_leg_quat, c_quat, tmp_vec, tmp_quat ;
	FVECTOR		rf_trans, rb_trans, lf_trans, lb_trans, cf_trans ;
	FVECTOR		*abs_rots ;
	FMATRIX		mat, mat2 ;

	evmobj = work->evmobj ;
	skel = evmobj->def->skeleton ;
	skel_mats = evmobj->matrix[ evmobj->use_buffer ] ;

	/* 足の胸関節から見た相対回転クォータニオンを求める */
	abs_rots = work->body->m_ctrl->abs_rots ;
	MT_QuatInverse( &tmp_quat, &abs_rots[ FAT_JOINT_BUST ] );
	MT_QuatMul( &r_leg_quat, &tmp_quat, &abs_rots[ FAT_JOINT_R_LEG ] );
	MT_QuatMul( &l_leg_quat, &tmp_quat, &abs_rots[ FAT_JOINT_L_LEG ] );

	MT_QuatSlerp( &c_quat, &r_leg_quat, &l_leg_quat, 0.5f );
	MT_QuatNormalize( &c_quat, &c_quat );

	if ( r_leg_quat.vw < 0.0f ){
		r_leg_quat.vx = -r_leg_quat.vx ;
		r_leg_quat.vy = -r_leg_quat.vy ;
		r_leg_quat.vz = -r_leg_quat.vz ;
		r_leg_quat.vw = -r_leg_quat.vw ;
	}
	if ( l_leg_quat.vw < 0.0f ){
		l_leg_quat.vx = -l_leg_quat.vx ;
		l_leg_quat.vy = -l_leg_quat.vy ;
		l_leg_quat.vz = -l_leg_quat.vz ;
		l_leg_quat.vw = -l_leg_quat.vw ;
	}
	if ( c_quat.vw < 0.0f ){
		c_quat.vx = -c_quat.vx ;
		c_quat.vy = -c_quat.vy ;
		c_quat.vz = -c_quat.vz ;
		c_quat.vw = -c_quat.vw ;
	}

	/*
		まだまだ改良の余地あり
	*/

	/*
		前面部分は胸を中心に回転して補正
	*/
	MT_QuatScaleAngle( &tmp_quat, &c_quat, 0.5f );
	MT_QuatToMat( &mat, &tmp_quat );
	_sceVu0MulMatrix( &mat, &skel_mats[ FAT_JOINT_BUST ], &mat );
	//mat = skel_mats[ FAT_JOINT_WAIST ] ;
	mat2 = skel_mats[ FAT_JOINT_BUST ] ;
	_sceVu0InversMatrix( &mat2, &mat2 );

	tmp_vec.vx = skel[ FAT_JOINT_COAT_RF ].rt_tx ;
	tmp_vec.vy = skel[ FAT_JOINT_COAT_RF ].rt_ty ;
	tmp_vec.vz = skel[ FAT_JOINT_COAT_RF ].rt_tz ;
	tmp_vec.vw = 1.0f ;
	_sceVu0ApplyMatrix( &rf_trans, &mat, &tmp_vec );
	_sceVu0ApplyMatrix( &rf_trans, &mat2, &rf_trans );
	_sceVu0SubVector( &rf_trans, &rf_trans, &tmp_vec );

	tmp_vec.vx = skel[ FAT_JOINT_COAT_LF ].rt_tx ;
	tmp_vec.vy = skel[ FAT_JOINT_COAT_LF ].rt_ty ;
	tmp_vec.vz = skel[ FAT_JOINT_COAT_LF ].rt_tz ;
	tmp_vec.vw = 1.0f ;
	_sceVu0ApplyMatrix( &lf_trans, &mat, &tmp_vec );
	_sceVu0ApplyMatrix( &lf_trans, &mat2, &lf_trans );
	_sceVu0SubVector( &lf_trans, &lf_trans, &tmp_vec );

	tmp_vec.vx = skel[ FAT_JOINT_COAT_CF ].rt_tx ;
	tmp_vec.vy = skel[ FAT_JOINT_COAT_CF ].rt_ty ;
	tmp_vec.vz = skel[ FAT_JOINT_COAT_CF ].rt_tz ;
	tmp_vec.vw = 1.0f ;
	_sceVu0ApplyMatrix( &cf_trans, &mat, &tmp_vec );
	_sceVu0ApplyMatrix( &cf_trans, &mat2, &cf_trans );
	_sceVu0SubVector( &cf_trans, &cf_trans, &tmp_vec );

	/*
		後面部分は胸を中心に回転して補正
	*/
	MT_QuatScaleAngle( &tmp_quat, &c_quat, 0.5f );
	MT_QuatToMat( &mat, &tmp_quat );
	_sceVu0MulMatrix( &mat, &skel_mats[ FAT_JOINT_WAIST ], &mat );
	//mat = skel_mats[ FAT_JOINT_WAIST ] ;
	mat2 = skel_mats[ FAT_JOINT_BUST ] ;
	_sceVu0InversMatrix( &mat2, &mat2 );

	tmp_vec.vx = skel[ FAT_JOINT_COAT_RB ].rt_tx ;
	tmp_vec.vy = skel[ FAT_JOINT_COAT_RB ].rt_ty ;
	tmp_vec.vz = skel[ FAT_JOINT_COAT_RB ].rt_tz ;
	tmp_vec.vw = 1.0f ;
	_sceVu0ApplyMatrix( &rb_trans, &mat, &tmp_vec );
	_sceVu0ApplyMatrix( &rb_trans, &mat2, &rb_trans );
	_sceVu0SubVector( &rb_trans, &rb_trans, &tmp_vec );

	tmp_vec.vx = skel[ FAT_JOINT_COAT_LB ].rt_tx ;
	tmp_vec.vy = skel[ FAT_JOINT_COAT_LB ].rt_ty ;
	tmp_vec.vz = skel[ FAT_JOINT_COAT_LB ].rt_tz ;
	tmp_vec.vw = 1.0f ;
	_sceVu0ApplyMatrix( &lb_trans, &mat, &tmp_vec );
	_sceVu0ApplyMatrix( &lb_trans, &mat2, &lb_trans );
	_sceVu0SubVector( &lb_trans, &lb_trans, &tmp_vec );



	/* 補正角度はちょっと控えめ */
	MT_QuatScaleAngle( &r_leg_quat, &r_leg_quat, 0.8f );
	MT_QuatScaleAngle( &l_leg_quat, &l_leg_quat, 0.8f );
	MT_QuatScaleAngle( &c_quat, &c_quat, 0.8f );



	EvmActMotionJoint( evmobj, FAT_JOINT_COAT_RF, &r_leg_quat, &rf_trans );
	EvmActMotionJoint( evmobj, FAT_JOINT_COAT_RB, &r_leg_quat, &rb_trans );

	EvmActMotionJoint( evmobj, FAT_JOINT_COAT_LF, &l_leg_quat, &lf_trans );
	EvmActMotionJoint( evmobj, FAT_JOINT_COAT_LB, &l_leg_quat, &lb_trans );

	EvmActMotionJoint( evmobj, FAT_JOINT_COAT_CF, &c_quat, &cf_trans );
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
static int GetResources( Work *work, int name, int where )
{
	int		chara_id ;
	CONTROL	*ctrl ;
	OBJECT	*body ;

	work->name = name ;

	/* キャラクタ取得 */
	if ( GCL_GetOption( 'c' ) == NULL ) return ( -1 );
	chara_id = GCL_GetNextInt() ;
	work->ctrl = ctrl = SearchControl( chara_id );
	if ( ctrl == NULL ){
		printf("%s: search error control\n", __FILE__);
		return ( -1 );
	}
	if ( ctrl->object == NULL ){
		printf("%s: control initialize miss error\n", __FILE__);
		return ( -1 );
	}
	work->body = body = ctrl->object ;
	if ( body->evmobj == NULL ){
		printf("%s: error: no support chara\n", __FILE__);
		return ( -1 );
	}

	work->evmobj = body->evmobj ;


	return ( 0 );
}

/* ---------------------------------------------------------------- */
	/*
		シナリオインターフェイス
	*/
void *NewFatmanPuppetIKSupportSet( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/* ---------------------------------------------------------------- */

