//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	puppetik.c
	ＩＫ実験プログラム

	2000/10/16 K.Takabe
	$Id: emma_ik.c,v 1.1.1.3 2002/11/19 11:46:03 Yoshizawa1 Exp $

*/

#include <stdio.h>
#include <stdlib.h>

#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"utl_dma.h"

#include	"../include/emma.h"

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


typedef	struct	{
    MT_JOINT_INFO2	left_arm[4] ;
    OBJECT		*object ;
    float               blend ;
    int		mode ;
} EMA_PuppetIK ;

/* ＩＫ計算を行う */
extern void MT_CalcInversKinematic2( MT_JOINT_INFO2 *joints, int n_joints, FVECTOR *target_pos ) ;
/* 間接情報を元にＩＫ計算を行う */

enum
{
    MODE_NO_ACTIVE	= 0,	/* ＩＫルーチン完全停止 */
    MODE_ACTIVE		= 1,	/* ＩＫ発動中 */
};

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

    MT_StartMemToSpr( (u_long128 *)scrpad->joints,
		      (u_long128 *)m_ctrl->abs_rots,
		      m_ctrl->n_joints );
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

    scrpad->root_mat = evmobj->root ? *evmobj->root : evmobj->world ;
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
/* 関節情報設定 */
static void SetJointInfo( MT_JOINT_INFO2 *joint, int joint_num,
			  DG_OBJS *objs,
			  int max_x, int min_x,
			  int max_y, int min_y,
			  int max_z, int min_z )
{
    joint->joint_num = joint_num ;
    joint->joint_parent_num = objs->def->models[joint_num].parent ;

    /* 一応クォータニオンを初期化しておく */
    joint->rotate = DG_ZeroVector ;
    joint->base   = DG_ZeroVector ;

    /* 関節のオフセットを設定 */
    joint->trans.vx = objs->def->models[joint_num].tx ;
    joint->trans.vy = objs->def->models[joint_num].ty ;
    joint->trans.vz = objs->def->models[joint_num].tz ;
    joint->trans.vw = 1.0f ;

    /* 関節長さ設定 */
    joint->joint_len = sceVu0Sqrt( _sceVu0InnerProduct( &joint->trans, &joint->trans ) ) ;

    /* 回転角度の最大・最小角度をラジアンで設定 */
    joint->max.vx =  max_x / 180.0f * 3.14159265f ;
    joint->max.vy =  max_y / 180.0f * 3.14159265f ;
    joint->max.vz =  max_z / 180.0f * 3.14159265f ;
    joint->min.vx =  min_x / 180.0f * 3.14159265f ;
    joint->min.vy =  min_y / 180.0f * 3.14159265f ;
    joint->min.vz =  min_z / 180.0f * 3.14159265f ;
}

#if 0
/* ＩＫ関節の初期化 */
static void InitIkJoint( EMA_PuppetIK *work, MT_JOINT_INFO2 *joint )
{
    MOTION_CONTROL	*m_ctrl ;
    FVECTOR		local_quat, parent_quat, inv_quat ;
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
#endif


/* ＩＫ計算ルーチンを呼び出す */
static void CalcIK( MT_JOINT_INFO2 *joint, FVECTOR *trg_pos, DG_OBJS *objs, int loop )
{
    int i ;

    joint[0].matrix = objs->objs[joint[0].joint_num].world ;
    for ( i = 0 ; i < loop ; i++ )
	MT_CalcInversKinematic2( joint, joint[0].total_joints, trg_pos );
}

/* ＩＫ結果をモーションにフィードバックさせる */
static void FeedbackJoint( EMA_PuppetIK *work, MT_JOINT_INFO2 *joint, float blend )
{
    MOTION_CONTROL *m_ctrl ;
    int            i ;
    FVECTOR        rot ;
    FVECTOR       *abs, *pabs ;

    m_ctrl = work->object->m_ctrl ;
    if ( blend == 1.0f )
	for ( i=1 ; i<joint[0].total_joints-1 ; i++ )
	{
	    abs  = &m_ctrl->abs_rots[joint[i].joint_num       ] ;
	    pabs = &m_ctrl->abs_rots[joint[i].joint_parent_num] ;
	    MT_QuatMul( abs, pabs, &joint[i].rotate ) ;
	}
    else
	for ( i=1 ; i<joint[0].total_joints-1 ; i++ )
	{
	    abs  = &m_ctrl->abs_rots[joint[i].joint_num       ] ;
	    pabs = &m_ctrl->abs_rots[joint[i].joint_parent_num] ;
	    MT_QuatMul( &rot, pabs, &joint[i].rotate ) ;
	    MT_QuatSlerp( abs, abs, &rot, blend ) ;
	    MT_QuatNormalize( abs, abs ) ;
	}
}


/*
  リセット
  */
void EMA_ResetPuppetIK( void *ptr )
{
    EMA_PuppetIK *work = ptr ;

    work->blend = 0.0f ;
}


/*
  エマＩＫのメイン処理
  */
int EMA_ActPuppetIK( void *ptr, int flag )
{
    EMA_PuppetIK *work = ptr ;
    OBJECT  *body ;
    FVECTOR  aim  ;

    /* ブレンドの割合を計算 */
    work->blend += flag ? 0.05f : -0.05f ;
    if ( work->blend > 1.0f )
	work->blend = 1.0f ;
    if ( work->blend < 0.0f )
    {
	work->blend = 0.0f ;
	return 1 ;/* これなら処理しない */
    }

#if 0
    /* 基準ポーズの更新要求処理 */
    if ( work->flag & FLAG_REQUEST_INIT_ROT )
    {
	InitIkJoint( work, work->left_arm ) ;
	work->flag &= ~FLAG_REQUEST_INIT_ROT ;
    }
#endif

    body = work->object ;

    /* 各関節のＩＫ計算 */
    _sceVu0ApplyMatrix( &aim,
			&BODYWORLD(GM_PlayerBody, HUMAN21_MIGI_TE),
			&EMA_HandOffset ) ;
    CalcIK( work->left_arm, &aim, body->objs, 2 ) ;

    /* ＩＫ計算結果をモーションに反映 */
    FeedbackJoint( work, work->left_arm, work->blend ) ;

    /* ＩＫで計算されない先端部分を設定 */
    body->m_ctrl->abs_rots[10] = body->m_ctrl->abs_rots[9] ;

    /* モーションをモデルに反映 */
    ActMotion2( body->m_ctrl, body->objs ) ;
    if ( body->evmobj )
	EvmActMotion( body->m_ctrl, body->evmobj ) ;

    return 0 ;
}

void EMA_FreePuppetIK( void *ptr )
{
    EMA_PuppetIK *work = ptr ;

    if ( work )
	GV_Free( work ) ;
}

void *EMA_MakePuppetIK( CONTROL *ctrl, OBJECT *body )
{
    EMA_PuppetIK *work ;
    FVECTOR       aim  ;

    work = GV_Malloc( sizeof(EMA_PuppetIK) ) ;
    if ( work )
    {
	GV_ZeroMemory( work, sizeof(EMA_PuppetIK) );

	work->object = body ;
	work->mode = MODE_NO_ACTIVE ;
	work->blend = 0.0f ;

	work->left_arm[ 0 ].total_joints = 4 ;
	/* 最初と末端はダミー */
	SetJointInfo( &work->left_arm[0], HUMAN21_HIDARI_KATA, body->objs,
		      0,   0,  0,  0,   0,   0 ) ;
	SetJointInfo( &work->left_arm[1], HUMAN21_HIDARI_UDE1, body->objs,
		      0, -80, 40,-40,  50,   0 ) ;
	SetJointInfo( &work->left_arm[2], HUMAN21_HIDARI_UDE2, body->objs,
		      0,-150, 20,-20, 120, -80 ) ;
	SetJointInfo( &work->left_arm[3], HUMAN21_HIDARI_TE  , body->objs,
		      0,   0,  0,  0,   0,   0 ) ;
	/* ＩＫ状態の初期化 */
	_sceVu0ApplyMatrix( &aim, &BODYWORLD(GM_PlayerBody, HUMAN21_MIGI_TE), &EMA_HandOffset ) ;
	CalcIK( work->left_arm, &aim, body->objs, 3 ) ;
    }
    return work ;
}
