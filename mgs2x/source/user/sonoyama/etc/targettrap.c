//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   targettrap.c	
   トラップにターゲット発生

   2001/05/11	M.Sonoyama
   $Id: targettrap.c,v 1.1.1.3 2002/11/19 11:50:46 Yoshizawa1 Exp $
*/

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <sys/types.h>

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

typedef	struct	{
	GV_ACT			actor ;
	TARGET			defense ;
	int				trap_name ;
	int				name ;
	int				proc ;
	int				where ;
	int				flag ;
} Work ;

enum {
	IS_KICK = 0,	/* キック */
	IS_M9,
	IS_PSG1,
	IS_NIKITA,
	IS_STINGER,
} ;

enum {
	MSG_KILL = 0,
} ;

/*----------------------------------------------------------------*/

static	void	Hit( TARGET *off, TARGET *def, void *ptr )
{
	Work		*work ;
	long64		wt ;
	GCL_ARGS	args ;
	int			buf[ 2 ] = { 5, 0 } ;
	void		( *func )( TARGET *, TARGET *, void * );

	work = ( Work * )ptr ;

	if ( work->proc <= 0 ) return ;

	wt = off->weapon_type ;
	args.argc = 2 ;
	args.argv = buf ;

	/* プレイヤーキック */
	if ( ( wt & WP_KICK ) && ( wt & WP_PLAYER ) ) {
		/* 強引にコールバックを呼ぶ */
		off->damaged |= ( off->class & TARGET_CHECK_CLASS ) ;
		func = off->callback ;
		if ( func != NULL ) {
			( *func )( off, def, off->work ) ;
		}
		/* プロック実行 */
		buf[ 0 ] = IS_KICK ;
		GM_ExecProc( work->proc, &args ) ;
		return ;
	}
	/* Ｍ９、ＰＳＧ１－Ｔ */
	if ( wt & WP_M92 ) {
		buf[ 0 ] = IS_M9 ;
		GM_ExecProc( work->proc, &args ) ;		
		return ;
	}
	/* ＰＳＧ１ */
	if ( wt & WP_PSG1 ) {
		buf[ 0 ] = IS_PSG1 ;
		GM_ExecProc( work->proc, &args ) ;		
		return ;
	}
	/* ニキータ、スティンガー */
	if ( ( wt & WP_WEAPONCORE ) ) {
		if ( off->name == WP_Nikita ) {
			buf[ 0 ] = IS_NIKITA ;			
		} else if ( off->name == WP_Stinger ) {
			buf[ 0 ] = IS_STINGER ;
		}
		GM_ExecProc( work->proc, &args ) ;		
		return ;
	}
	if ( wt & WP_NIKITA ) {
		buf[ 0 ] = IS_NIKITA ;			
	} else if ( wt & WP_STINGER ) {
		buf[ 0 ] = IS_STINGER ;
	}
	GM_ExecProc( work->proc, &args ) ;		
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	GV_MSG		*msg ;
	int			n_msg, code ;

	//NewTargetView2( &work->defense, 32, 232, 32 ) ;

	n_msg = GV_ReceiveMessage( work->name, &msg ) ;
	while( -- n_msg >= 0 ) {
		code = msg->message[ 0 ] ;	
		if ( code == MSG_KILL ) {
			GM_FreeTarget( &work->defense ) ;
			GV_DestroyActor( work ) ;
			work->flag |= 1 ;
			break ;
		}
		msg ++ ;
	}
	/* メッセージがくるまで停止 */
	GV_WaitMessage( &work->actor, work->name ) ;
}

static	void	Die( Work *work )
{
	if ( !( work->flag & 1 ) ) {
		GM_FreeTarget( &work->defense ) ;
	}
}

/*----------------------------------------------------------------*/

static	void	SetTarget( Work *work, HZX_BLOCK *blk, HZX_TRP *trap, int where )
{
	TARGET		*t ;
	FVECTOR		size, center ;

	t = &work->defense ;
	
	center.vx = ( float )blk->tx + ( float )( trap->b1.vx + trap->b2.vx ) / 2.0F ;
	center.vy = ( float )blk->ty + ( float )( trap->b1.vy + trap->b2.vy ) / 2.0F ;
	center.vz = ( float )blk->tz + ( float )( trap->b1.vz + trap->b2.vz ) / 2.0F ;
	size.vx = ( float )( trap->b2.vx - trap->b1.vx ) / 2.0F ;
	size.vy = ( float )( trap->b2.vy - trap->b1.vy ) / 2.0F ;
	size.vz = ( float )( trap->b2.vz - trap->b1.vz ) / 2.0F ;
	GM_SetTarget( t, TARGET_DEFENSE | TARGET_POWER | TARGET_SEEK | 
				  TARGET_THROUGH | TARGET_HAZARD,
				  where, BOTH_SIDE, &size, &DG_ZeroVector ) ;
	GM_MoveTarget( t, &center ) ;
	GM_SetTargetCallBack( t, Hit, ( void * )work ) ;
	GM_PutTarget( t ) ;
}

static	int	GetResources( Work *work, int name, int where )
{
	int				trap_name ;
	HZX_GROUP_ID	hzx_id ;
	HZX_BLOCK		*blk ;
	HZX_TRP			*trap ;

	if ( GCL_GetOption( 'm' ) != NULL ) {
		where = GM_GetMapID( GCL_GetNextInt() ) ;
	}	
	trap_name = GCL_GetOptionValue( 't', 0 ) ;

	work->trap_name = trap_name ;
	work->name = name ;
	work->where = where ;

	hzx_id = GM_GetHzxGroupID( where ) ;
	if ( !HZX_FindTrap( hzx_id, trap_name, &blk, &trap ) ) return -1 ;

	SetTarget( work, blk, trap, where ) ;

	work->proc = GCL_GetOptionValue( 'p', 0 ) ;

	/* メッセージがくるまで停止 */
	GV_WaitMessage( &work->actor, work->name ) ;

	return 0 ;
}

/*----------------------------------------------------------------*/

void	*NewTargetTrap( int name, int where )
{
	Work			*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		if ( GetResources( work, name, where ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	} 
	return work ;
}






