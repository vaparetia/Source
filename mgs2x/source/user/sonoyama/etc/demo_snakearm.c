//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   demo_snakearm.c
   デモ用蛇手

   2001/06/11	M.Sonoyama
   $Id: demo_snakearm.c,v 1.1.1.3 2002/11/19 11:50:42 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <math.h>

#ifndef KP_XBOX
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif


#include	"gameheader.h"

/*----------------------------------------------------------------*/

static	FVECTOR		HangPos ;

typedef	struct	{
	GV_ACT_EX		actor ;
	OBJECT			*pbody ;
	DG_EVMOBJ		*evmobj ;
	MOTION_CONTROL	*m_ctrl ;
	int				name ;
	int				motion ;
	int				next_motion ;
	int				punit ;
	int				flag ;
	
	void			*flow ;
	int				flow_flag ;

	DG_EVMOBJ		*evmobj2 ;
	void			*flow2 ;
	int				flow_flag2 ;

	DG_OBJS			*shadow_objs ;

	int				proc ;
} Work ;

extern	void *NewSolidusSnakearmFlow(
									 DG_EVMOBJ *evmobj,		// 蛇手モデル 
									 int		*flag			// フラグ 
									 ) ;

/*----------------------------------------------------------------*/

static	void	CheckMessage( Work *work )
{
	GV_MSG		*msg ;
	int			n_msg ;

	n_msg = GV_ReceiveMessage( work->name, &msg ) ;
	while( -- n_msg >= 0 ) {
		if ( msg->message[ 0 ] == 0 ) {	/* motion */
			work->next_motion = msg->message[ 1 ] ;
		} else if( msg->message[ 0 ] == 1 ) {	/* flow */
			if ( msg->message[ 1 ] == 1 ) work->flag = 1 ;
			else						  work->flag = 0 ;			
		}
		msg ++ ;
	}
}

/*----------------------------------------------------------------*/

static	void	UpdateHangPos( Work *work )
{
	FMATRIX		*skel_mats ;
	FVECTOR		to, to1, to2 ;

	skel_mats = work->evmobj->matrix[ work->evmobj->use_buffer ] ;	
	to1.vx = work->evmobj->def->skeleton[ 14 ].rt_tx ;
	to1.vy = work->evmobj->def->skeleton[ 14 ].rt_ty ;
	to1.vz = work->evmobj->def->skeleton[ 14 ].rt_tz ;
	to2.vx = work->evmobj->def->skeleton[ 30 ].rt_tx ;
	to2.vy = work->evmobj->def->skeleton[ 30 ].rt_ty ;
	to2.vz = work->evmobj->def->skeleton[ 30 ].rt_tz ;	

	DG_SetPos( &skel_mats[ 14 ] ) ;
	DG_PutVector( &to1, &to1, 1 ) ;
	DG_SetPos( &skel_mats[ 30 ] ) ;
	DG_PutVector( &to2, &to2, 1 ) ;

	_sceVu0AddVector( &to, &to1, &to2 ) ;
	_sceVu0ScaleVector( &to, &to, 0.50F ) ;

	DG_COPY_VEC( &HangPos, &to ) ;
}

static	void	UpdateShadowModel( Work *work )
{
	int			i ;
	EVM_SKEL	*skel ;
	FMATRIX		*src, *dst ;
	FVECTOR		trans ;

	if ( work->shadow_objs == NULL ) return ;
	src = work->evmobj->matrix[ work->evmobj->use_buffer ] ;
	skel = work->evmobj->def->skeleton ;
	DG_COPY_MAT( &work->shadow_objs->world, &work->evmobj->world ) ;
	for ( i = 0; i < work->evmobj->n_skeleton; i ++, src ++, skel ++ ) {
		dst = &work->shadow_objs->objs[ i ].world ;
		DG_COPY_MAT( dst, src ) ;
		trans.vx = skel->rt_tx ;
		trans.vy = skel->rt_ty ;
		trans.vz = skel->rt_tz ;
		trans.vw = 1.0F ;
		_sceVu0ApplyMatrix( ( FVECTOR * )dst->m[ 3 ], dst, &trans ) ;
	}
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	CheckMessage( work ) ;
	if ( work->motion != work->next_motion ) {
		MT_SetMotionData( work->m_ctrl, 0, work->next_motion, 0, -1 ) ;
		MT_SetMotionInterp( work->m_ctrl, 6, I64(0xfffffffffff) ) ;
		work->motion = work->next_motion ;
	}
	DG_SetPos( &work->pbody->objs->objs[ work->punit ].world ) ;
	DG_GetPos( &work->evmobj->world ) ;

   DG_Arm_SwitchEvmBuffer(work->evmobj);
	MT_ActMotion( work->m_ctrl, NULL, work->evmobj ) ;
	MT_EvmActMotion( work->m_ctrl, work->evmobj ) ;
	MT_ActSequenceSEV( work->m_ctrl->sar_ctrl, work->m_ctrl, NULL, work->evmobj ) ;

	if ( MT_CHECK_LAST1( work->m_ctrl, 0 ) ) {
		if ( work->proc > 0 ) {
			GCL_ARGS		args ;
			int				buf[ 4 ] ;

			args.argv = buf ;
			args.argc = 1 ;
			buf[ 0 ] = work->motion ;
			GM_ExecProc( work->proc, &args ) ;
		}
	}

	work->flow_flag = 0 ;
	if ( work->flag ) work->flow_flag = 1 ;

	UpdateHangPos( work ) ;
	
	/* 影モデル */
	UpdateShadowModel( work ) ;
}

static	void	Die( Work *work )
{
	MT_FreeMotion( work->m_ctrl ) ;
	DG_DequeueEvmObj( work->evmobj ) ;
	DG_FreeEvmObj( work->evmobj ) ;
	if ( work->shadow_objs != NULL ) {
		DG_DisconnectObjs( work->pbody->objs, work->shadow_objs ) ;
		DG_DequeueObjs( work->shadow_objs ) ;
		DG_FreeObjs( work->shadow_objs ) ;
	}
}

/*----------------------------------------------------------------*/

static	int		GetResources( Work *work, int name, int where )
{
	EVM_DEF		*def ;
	CONTROL		*pctrl ;
	OBJECT		*pbody ;
	int			model, motion, parent, unit ;
	
	/* 親の検索 */
	parent = GCL_GetOptionValue( 'p', 0 ) ;
	unit = GCL_GetOptionValue( 'u', 0 ) ;
	pctrl = GM_SearchWhere( parent ) ;
	ASSERT( pctrl != NULL ) ;
	pbody = pctrl->object ;
	ASSERT( pbody != NULL ) ;	

	work->name = name ;
	work->pbody = pbody ;
	work->punit = unit ;
	
	model = GCL_GetOptionValue( 'm', 0 ) ;
	def = ( EVM_DEF * )GV_GetCache( GV_CacheID( model, 'e' ) ) ;
	ASSERT( def != NULL ) ;
	work->evmobj = DG_MakeEvmObj( def, 0, 0 ) ;
	ASSERT( work->evmobj != NULL ) ;
	DG_QueueEvmObj( work->evmobj ) ;
	work->evmobj->light = pbody->objs->light ;

	motion = GCL_GetOptionValue( 'O', 0 ) ;
	work->m_ctrl = MT_InitMotion( NULL, 1, motion, MT_FLAG_TRANS ) ;
	work->m_ctrl->sar_ctrl = MT_InitSequence( 1, motion, 0 );
	work->m_ctrl->se_table_id = -1 ;
	MT_SetMotionData( work->m_ctrl, 0, 0, 0, -1 ) ;
	work->motion = work->next_motion = 0 ;

	work->flow = NewSolidusSnakearmFlow( work->evmobj, &work->flow_flag ) ;
	GV_SetActorChild( work, work->flow ) ;

	DG_SetPos( &pbody->objs->objs[ unit ].world ) ;
	DG_GetPos( &work->evmobj->world ) ;

	work->proc = GCL_GetOptionValue( 'R', 0 ) ;

	/* 影モデル */
	if ( GCL_GetOption( 's' ) != NULL ) {
		DG_DEF		*defk ;

		defk = ( DG_DEF * )GV_GetCache( GV_CacheID( GCL_GetNextInt(), 'k' ) ) ;
		if ( defk != NULL ) {
			work->shadow_objs = DG_MakeObjs( defk, DG_FLAG_FINISHCALC|DG_FLAG_SHADOWMAKE, 0 ) ;
			ASSERT( work->shadow_objs != NULL ) ;
			DG_QueueObjs( work->shadow_objs ) ;
			DG_InvisibleObjs( work->shadow_objs ) ;

			DG_ConnectObjs( work->pbody->objs, work->shadow_objs ) ;
		}
	}

	return 0 ;
}

/*----------------------------------------------------------------*/

void		*NewDemoSnakeArm( int name, int where )
{
	Work			*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

/*----------------------------------------------------------------*/

/* つかみ位置取得 */
int			PL_COM_GetDemoSnakeArmHangPos( void )
{
	GCL_VAR_REF		ref ;

	GCL_GetNextVarRef( &ref ) ;
	GCL_SetVarRef( &ref, 0, ( int )HangPos.vx ) ;
	GCL_GetNextVarRef( &ref ) ;
	GCL_SetVarRef( &ref, 0, ( int )HangPos.vy ) ;
	GCL_GetNextVarRef( &ref ) ;
	GCL_SetVarRef( &ref, 0, ( int )HangPos.vz ) ;
	return 0 ;
}
