//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	attacker.c
	攻撃兵
	
	1997/07/07 Y.Korekado
	$Id: attacker.c,v 1.1.1.3 2002/11/19 11:43:59 Yoshizawa1 Exp $
	
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
#include	"korekado/enemy/enemy.h"

#include	"attacker.h"
#include	"atprepro.c"
#include	"atactsub.c"
#include	"ataction.c"
#include	"atthink.c"

static void AttackerMain( Work *work )
{
	/* 前処理 */
	PreProcess( work ) ;
	/* 思考処理 */
	Think( work ) ;
	/* 行動処理 */
	Action( work ) ;

//    work->enethink.zoneadd = HZX_GetAddress( work->control.hzx_id,
//				      &( work->control.mov ), work->enethink.ctrl->addr ) ;
}

static void Act( Work *work )
{
	GM_ActControl( &work->control ) ;
	DG_SetPos2( &work->control.mov, &work->control.rot );
	GM_ActObject( &work->body );
	DG_GetLightMatrix( &work->control.mov, work->lights );

	AttackerMain( work ) ;
}

static void Die( Work *work )
{
}
/*--- ------------------------------------------------------------*/
static	void	SetParam( work )
Work	*work ;
{
	ENE_SetSenseParam( &(work->enethink.sense), 0, DEF_EYE_RANGE, DEF_EYE_SIGHT, DEF_HEARING, DEF_SMELL ) ;
//	ENE_SetEyeInfo( &(work->enethink.pl_eyei), &GM_PlayerPosition, &GM_PlayerAddress, &GM_PlayerMap ) ;
}

static	void	InitControl( ctrl )
CONTROL	*ctrl ;
{
	GM_InitControl( ctrl, 222, 0 ) ;
	ctrl->hzx_height = 750 ;
	ctrl->height = 1049.0F ;
	GM_ConfigControlHazard( ctrl, 1200, 450, 500 ) ;

	ctrl->seg_flag = 0 ;
	ctrl->skip_flag |= CTRL_HZX_SEG_HORIZON_CHECK ;
}

static	void	InitObject( work )
Work	*work ;
{
	/* 関節型モデルの表示 */
	GM_InitObject( &work->body, GV_StrCode( "out_cold" ), DG_FLAG_SHADE|DG_FLAG_FINISHCALC );
	GM_ConfigObjectLight( &work->body, work->lights );
	GM_ConfigObjectMotion( &work->body, 2, GV_StrCode("mg2ene"), MT_FLAG_HUMAN1 );
	GM_ConfigObjectStep( &work->body, &work->control.step );
	GM_ConfigObjectAction( &work->body, 0, 0, 0, 0xfffff, 0 );

	/* モーションオーバーライド実験 */
	GM_ConfigObjectAction( &work->body, 1, 0, 0, 0x0, 0 );
//	GM_ConfigObjectAction( &work->body, 1, GV_StrCode( "stand1_famas" ), 0, 0x003fe, 0 );
//	work->body.m_ctrl->merge_flag = 0x0002 ;

#if 0
	/* モーションアジャスト実験 */
	vec = DG_ZeroVector ;
	vec.vx = 0.5F ;
	MT_EulerToQuatXYZ( &work->body.m_ctrl->adjust[1], &vec );
	work->body.m_ctrl->adjust_flag = 0x0002 ;
#endif

	/* 武器 */
	GM_InitObject( &work->weapon, GV_StrCode( "famas" ), DG_FLAG_SHADE|DG_FLAG_ONEPIECE );
	GM_ConfigObjectRoot( &work->weapon, &work->body, 4 ) ;
}

static void InitAction( work )
Work	*work ;
{
	ACTION	*act ;
	int		act_num ;
	
	act = &work->action ;
	AT_InitAction( act, &work->control, &work->body ) ;
	AT_SetActionMotion( act, NULL, (int *)EnemyMotionArray ) ;
	AT_SetMode( act, ENE_ActStandStill ) ;
	act->CheckPad = CheckPad ;

	act_num = 0 ;
	act->c_motion_num[0] = act_num ;
	GM_ConfigObjectAction( act->body, 0, act->motion_table[act_num], 0, MOTION_MASK_FULL, 0 );
}

static int InitThink( work )
Work	*work ;
{
	int	valu ;

	if ( (valu = GCL_GetOptionValue( 'r', -1 )) < 0  ) {
		return -1 ;
	}

	ENE_InitRouteNavi( &work->routenavi, valu, 0 ) ;

	ENE_InitRoute( &work->cl_route ) ;

	ENE_InitZoneNavi( &work->zonenavi ) ;

	ENE_InitTargPoint( &work->enethink.trgpoint ) ;

	ENE_InitEneThink( &work->enethink, &work->control, &work->routenavi, &work->zonenavi, &work->action ) ;
	work->enethink.rnavi2 = &work->cl_route ;	/* クリアリング用 */

	if ( (valu = GCL_GetOptionValue( 'n', -1 )) < 0  ) {
		valu = 0 ;
	}
	work->control.mov = work->routenavi.nodes[ valu ] ;
	ENE_SetTrgpPoint( &(work->enethink.trgpoint), &(work->control.mov), work->control.hzx_id ) ;

    GM_ConfigControlHzxHeight( &(work->control), 750.0F, work->control.mov.vy + 100.0F ) ;

	ENE_AttackerStartModeSneak( &work->enethink ) ;

	return 0 ;
}

static int GetResources( Work *work )
{
	/* コマンダーシステム登録&ＩＤ取得 */
	ENE_SetIDNumber( &work->enethink ) ;
	/* パラメータセット、シナリオリード */
	SetParam( work ) ;
	/* コントロールの初期化 */
	InitControl( &work->control ) ;
	/* オブジェクトの初期化 */
	InitObject( work ) ;
	/* アクション初期化 */
	InitAction( work ) ;
	/* 思考パラメータ初期化 */
	if ( InitThink( work ) < 0 ) {
		return -1 ;
	}

	return (0);
}

void *NewAttacker( )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

#if 0
void *NewAttackerCall( void )
{
	int	i ;

	for( i=0; i<4; i++ ) {
		NewAttacker( i ) ;
	}
}
#endif
