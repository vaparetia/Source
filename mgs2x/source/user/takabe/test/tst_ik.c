/*
	tst_ik.c
	ＩＫ実験プログラム

	2000/10/16 K.Takabe
	$Id: tst_ik.c,v 1.1.1.3 2002/11/19 11:51:31 Yoshizawa1 Exp $

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

#define BODY_FLAG	(DG_FLAG_SHADE|DG_FLAG_FINISHCALC)

typedef	struct	{
	GV_ACT		actor ;
	CONTROL		control ;
	OBJECT		object ;
	FMATRIX		light[2] ;
	MT_JOINT_INFO	left_arm[4+2+6] ;
	MT_JOINT_INFO	right_arm[4+2+6] ;
	MT_JOINT_INFO	left_leg[4+2+6] ;
	MT_JOINT_INFO	right_leg[4+2+6] ;
	FVECTOR		ik_pos ;
} Work ;


/* ---------------------------------------------------------------- */
static void Message( int name, int len, ... )
{
	GV_MSG	msg ;
	int		mes[16], *mes_data ;
	va_list	ap ;

	msg.address = name ;
	msg.message = mes ;
	msg.message_len = len ;

	va_start( ap, len );
	for ( mes_data = mes ; len > 0 ; len-- ){
		*mes_data++ = va_arg( ap, int ) ;
	}
	va_end( ap );
	GV_SendMessage( &msg );
}
/* ---------------------------------------------------------------- */
static void PrintQuat( char *header, FVECTOR *quat )
{
	FVECTOR	tmp ;
	MT_QuatGetValue( &tmp, quat );
	tmp.vw = tmp.vw * 180.0f / 3.14159265f ;
	printf("%s %f %f %f %f\n", header, tmp.vx, tmp.vy, tmp.vz, tmp.vw );
	
}
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
/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	FVECTOR		trg_pos ;
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	{/* ＩＫ目標座標の移動処理 */
		int		dx, dy ;
		dx = GV_PadData[0].right_dx - 128 ;
		dy = GV_PadData[0].right_dy - 128 ;
		if ( dx > 0 ){
			if ( dx < 32 )	dx = 0 ;
			else			dx = ( dx - 32 ) * 256 / ( 128 - 32 );
		} else {
			if ( dx > -32 )	dx = 0 ;
			else			dx = ( dx + 32 ) * 256 / ( 128 - 32 );
		}
		if ( dy > 0 ){
			if ( dy < 32 )	dy = 0 ;
			else			dy = ( dy - 32 ) * 256 / ( 128 - 32 );
		} else {
			if ( dy > -32 )	dy = 0 ;
			else			dy = ( dy + 32 ) * 256 / ( 128 - 32 );
		}
		if ( GV_PadData[0].status & PAD_L1 ){
			work->ik_pos.vx += (float)dx / 16.0f ;
			work->ik_pos.vy -= (float)dy / 16.0f ;
		} else {
			work->ik_pos.vx += (float)dx / 16.0f ;
			work->ik_pos.vz += (float)dy / 16.0f ;
		}
		work->ik_pos.vw = 1.0f ;
		AN_Test_Eye2( &work->ik_pos, 3 );
		//printf("%f %f %f\n", work->ik_pos.vx, work->ik_pos.vy, work->ik_pos.vz );
	}

	GM_ActMotion( body );
	ctrl->height = body->height ;
	GM_ActControl( ctrl );
	GM_ActObject2( body );

	/* ＩＫ情報をセット */
#if 1
	//work->right_arm[ 0 ].matrix = body->objs->objs[3].world ;
	work->right_arm[ 0 ].matrix = DG_UnitMatrix ;
	{
		FMATRIX	inv_mat ;
		_sceVu0InversMatrix( &inv_mat, &body->objs->objs[3].world );
		trg_pos = work->ik_pos ;
		trg_pos.vw = 1.0f ;
		_sceVu0ApplyMatrix( &trg_pos, &inv_mat, &trg_pos );
		//printf("%f %f %f %f\n", trg_pos.vx, trg_pos.vy, trg_pos.vz, trg_pos.vw );
	}
	{/**/
		FVECTOR	quat, tmp ;
		extern void MT_CalcInversKinematic( MT_JOINT_INFO *joints, int n_joints, FVECTOR *target_pos );
		MT_CalcInversKinematic( work->right_arm, 6, &trg_pos );

		quat = work->right_arm[1].rotate ;
		MT_QuatMul( &quat, &quat, &work->right_arm[2].rotate );
		MT_QuatMul( &quat, &quat, &work->right_arm[3].rotate );
		//body->m_ctrl->abs_rots[4] = quat ;
		//body->m_ctrl->abs_rots[5] = work->right_arm[4].rotate ;
		MT_QuatMul( &body->m_ctrl->abs_rots[4], &body->m_ctrl->abs_rots[3], &quat );

		MT_QuatMul( &body->m_ctrl->abs_rots[5], &body->m_ctrl->abs_rots[4], &work->right_arm[4].rotate );

		//MT_QuatMul( &body->m_ctrl->abs_rots[6], &body->m_ctrl->abs_rots[5], &body->m_ctrl->rots[6] );
		body->m_ctrl->abs_rots[6] = body->m_ctrl->abs_rots[5] ;
#if 0
		MT_QuatGetValue( &tmp, &work->right_arm[1].rotate );
		printf("1) %f %f %f %f\n", tmp.vx, tmp.vy, tmp.vz, tmp.vw );
		MT_QuatGetValue( &tmp, &work->right_arm[2].rotate );
		printf("2) %f %f %f %f\n", tmp.vx, tmp.vy, tmp.vz, tmp.vw );
		MT_QuatGetValue( &tmp, &work->right_arm[3].rotate );
		printf("3) %f %f %f %f\n", tmp.vx, tmp.vy, tmp.vz, tmp.vw );
		MT_QuatGetValue( &tmp, &work->right_arm[4].rotate );
		printf("4) %f %f %f %f\n", tmp.vx, tmp.vy, tmp.vz, tmp.vw );
#endif
	}
#endif

#if 0
	work->right_leg[ 0 ].matrix = DG_UnitMatrix ;
	{
		FMATRIX	inv_mat ;
		_sceVu0InversMatrix( &inv_mat, &body->objs->objs[0].world );
		trg_pos = work->ik_pos ;
		trg_pos.vw = 1.0f ;
		_sceVu0ApplyMatrix( &trg_pos, &inv_mat, &trg_pos );
	}
	{/**/
		FVECTOR	quat, tmp ;
		extern void MT_CalcInversKinematic( MT_JOINT_INFO *joints, int n_joints, FVECTOR *target_pos );
		MT_CalcInversKinematic( work->right_leg, 6, &trg_pos );

		quat = work->right_leg[1].rotate ;
		MT_QuatMul( &quat, &quat, &work->right_leg[2].rotate );
		MT_QuatMul( &quat, &quat, &work->right_leg[3].rotate );
		MT_QuatMul( &body->m_ctrl->abs_rots[13], &body->m_ctrl->abs_rots[0], &quat );

		MT_QuatMul( &body->m_ctrl->abs_rots[14], &body->m_ctrl->abs_rots[13], &work->right_leg[4].rotate );

		//MT_QuatMul( &body->m_ctrl->abs_rots[15], &body->m_ctrl->abs_rots[14], &body->m_ctrl->rots[6] );
		body->m_ctrl->abs_rots[15] = body->m_ctrl->abs_rots[14] ;
	}
#endif

	GM_ActObject2( body );
	work->ik_pos = *(FVECTOR*)body->objs->objs[6].world.m[3] ;
	//work->ik_pos = *(FVECTOR*)body->objs->objs[15].world.m[3] ;

	DG_GetLightMatrix( &ctrl->mov, work->light );
}

static void Die( Work *work )
{
	GM_FreeControl( &work->control );
	GM_FreeObject( &work->object );
}

static int GetResources( Work *work, int name, int where )
{
	FVECTOR		pos ;
	SVECTOR		rot ;
	CONTROL	*ctrl ;
	OBJECT	*body ;

	ctrl = &work->control ;
	body = &work->object ;

	/* 設置座標読み込み */
	if ( GCL_GetOption( 'p' ) != NULL ) {
		pos.vx = (float)GCL_GetNextInt() ;
		pos.vy = (float)GCL_GetNextInt() ;
		pos.vz = (float)GCL_GetNextInt() ;
	} else {
		pos.vx = pos.vy = pos.vz = 0 ;
	}
	pos.vw = 1.0f ;
	if ( GCL_GetOption( 'r' ) != NULL ){
		rot.vx = GCL_GetNextInt() ;
		rot.vy = GCL_GetNextInt() ;
		rot.vz = GCL_GetNextInt() ;
	} else {
		rot.vx = rot.vy = rot.vz = 0 ;
	}

	/* オブジェクトの初期化 */
	GM_InitObject( body, GV_StrCode("sna_def"), BODY_FLAG );
	GM_ConfigObjectMotion( body, 1, GV_StrCode("snake"), MT_FLAG_HUMAN2 );
	GM_ConfigObjectLight( body, work->light );
	GM_ConfigObjectStep( body, &ctrl->step );
	GM_ConfigObjectAction( body, 0, 0, 0, 0xffffffff, 0 );
	/* コントロールの初期化 */
	GM_InitControl( ctrl, name, where );
	GM_ConfigControlHzxHeight( ctrl, 1500, 1500 );
	GM_ConfigControlHzxCheckFlag( ctrl, 0, 0 );
	GM_ConfigControlPosition( ctrl, &pos, &rot );
	GM_ConfigControlObject( ctrl, body );

	GM_ActMotion( body );
	ctrl->height = body->height ;
	GM_ActControl( ctrl );
	GM_ActObject2( body );

	/* ＩＫ関連初期化 */
	/* ＩＫ終点関節（ダミー用） */
	SetJointInfo( &work->right_arm[ 0 ], 0, &body->objs->objs[3].trans, 0, 0 );
	/* 上腕腕関節（Ａ） */
	SetJointInfo( &work->right_arm[ 1 ], 0, &body->objs->objs[4].trans, 60, -170 );
	/* 上腕腕関節（Ｂ） */
	SetJointInfo( &work->right_arm[ 2 ], 2, &DG_ZeroVector, 50, -90 );
	/* 上腕腕関節（Ｃ） */
	SetJointInfo( &work->right_arm[ 3 ], 1, &DG_ZeroVector, 100, -45 );
	/* 腕関節 */
	SetJointInfo( &work->right_arm[ 4 ], 0, &body->objs->objs[5].trans, 0, -150 );
	/* ＩＫ起点関節（ダミー用） */
	SetJointInfo( &work->right_arm[ 5 ], 0, &body->objs->objs[6].trans, 0, 0 );

	/* ＩＫ終点関節（ダミー用） */
	SetJointInfo( &work->right_leg[ 0 ], 0, &body->objs->objs[0].trans, 0, 0 );
	/* 上腕腕関節（Ａ） */
	SetJointInfo( &work->right_leg[ 1 ], 0, &body->objs->objs[13].trans, 40, -120 );
	/* 上腕腕関節（Ｂ） */
	SetJointInfo( &work->right_leg[ 2 ], 2, &DG_ZeroVector, 20, -45 );
	/* 上腕腕関節（Ｃ） */
	SetJointInfo( &work->right_leg[ 3 ], 1, &DG_ZeroVector, 40, -70 );
	/* 腕関節 */
	SetJointInfo( &work->right_leg[ 4 ], 0, &body->objs->objs[14].trans, 150, 0 );
	/* ＩＫ起点関節（ダミー用） */
	SetJointInfo( &work->right_leg[ 5 ], 0, &body->objs->objs[15].trans, 0, 0 );

	work->ik_pos = pos ;

	return (0);
}


void *NewInversKinematicTest( int name, int where )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
