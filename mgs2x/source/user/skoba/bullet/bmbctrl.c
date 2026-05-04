//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   bmbctrl.c
   爆発系コントロール

   2001/06/12	M.Sonoyama
   $Id: bmbctrl.c,v 1.1.1.3 2002/11/19 11:49:59 Yoshizawa1 Exp $
*/

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

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
//#include <libutl.h>

#include	"gameheader.h"

#include "bp_vector.h"

/*----------------------------------------------------------------*/

/* ボムリスト初期化 */
void	GM_InitBombList( GM_BOMB *this, int weapon, FVECTOR *mov, FVECTOR *step )
{
	this->weapon = weapon ;
	this->mov = mov ;
	this->step = step ;
	this->flag = 0 ;
}

/* ボムリスト登録 */
void	GM_AddBombList( GM_BOMB	*this )
{
	/* 先頭に繋ぐ */
	this->next = GM_BombList.next ;
	GM_BombList.next = this ;
}

/* ボムリストから削除 */
int		GM_RemoveBombList( GM_BOMB *this )
{
	GM_BOMB		*cur, *prev ;

	prev = &GM_BombList ;
	cur = GM_BombList.next ;
	while( cur != NULL ) {
		if ( cur == this ) {
			prev->next = cur->next ;
			return 1 ;
		}
		prev = cur ;
		cur = cur->next ;
	}	
	return 0 ;
}

/*----------------------------------------------------------------*/

typedef	struct	{
	GV_ACT			actor ;
	long64			destroy ;
	long64			invisible ;
	long64			noblast ;
	long64			blast ;
	long64			nocontrol ;
} Work ;

#define	MSG_NAME	(2993138)		/* 爆発系管理 */

enum {
	MSG_DESTROY		= 0,
	MSG_INVISIBLE,
	MSG_NOBLAST,
	MSG_BLAST,
	MSG_NOCONTROL,	
} ;

/*----------------------------------------------------------------*/

/*----------------------------------------------------------------*/

static	void	CheckMessage( Work *work )
{
	GV_MSG		*msg ;
	int			n_msg, code ;
	long64		weapon ;

	n_msg = GV_ReceiveMessage( MSG_NAME, &msg ) ;

	while( -- n_msg >= 0 ) {
		code = msg->message[ 0 ] ;
		if ( msg->message_len >= 2 ) weapon = ( I64(1) << msg->message[ 1 ] ) ;
		else						 weapon = I64(0xffffffffffffffff) ;
		switch( code ) {
		case MSG_DESTROY :
			work->destroy |= weapon ;
			break ;
		case MSG_INVISIBLE :
			work->invisible |= weapon ;
			break ;
		case MSG_NOBLAST :
			work->noblast |= weapon ;
			break ;
		case MSG_BLAST :
			work->blast |= weapon ;
			break ;
		case MSG_NOCONTROL :
			work->nocontrol |= weapon ;
		}
		msg ++ ;
	}
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	GM_BOMB		*this, *list ;
	long64		name ;

	work->destroy = I64(0) ;
	work->invisible = I64(0) ;
	work->noblast = I64(0) ;
	work->blast = I64(0) ;
	work->nocontrol = I64(0) ;

	CheckMessage( work ) ;

	list = GM_BombList.next ;
	while( list != NULL ) {
		this = list ;
		list = list->next ;
		name = ( I64(1) << this->weapon ) ;
		if ( name & work->destroy ) this->flag |= GM_BMB_FLAG_DESTROY ;
		if ( name & work->invisible ) this->flag |= GM_BMB_FLAG_INVISIBLE ;
		if ( name & work->noblast ) this->flag |= GM_BMB_FLAG_NOBLAST ;
		if ( name & work->blast ) this->flag |= GM_BMB_FLAG_BLAST ;
		if ( name & work->nocontrol ) this->flag |= GM_BMB_FLAG_NOCONTROL ;
	}
	GV_WaitMessage( work, MSG_NAME ) ;
}

static	void	Die( Work *work )
{

}

/*----------------------------------------------------------------*/

void		*PL_BombListControl( void )
{
	Work		*work ;

	work = ( Work * )GV_NewActor( GV_ACTOR_AFTER2, sizeof( Work ) ) ;
	ASSERT( work != NULL ) ;
	GV_SetActor( &work->actor, Act, Die ) ;
	GM_BombList.next = NULL ;
	return work ;
}

/*----------------------------------------------------------------*/
/*----------------------------------------------------------------*/

/* 水面水飛沫 */
int		PL_CheckBulletSplash( FVECTOR *mov0, FVECTOR *step0, float width, float force )
{
	FVECTOR			mov, step ;
	int				map ;

	if ( PL_BulletSplashFunc != NULL && 
		 mov0->vy+step0->vy <= GM_WaterLevel ) {
		DG_COPY_VEC( &step, step0 ) ;
		DG_COPY_VEC( &mov, mov0 ) ;
		if ( DG_FABS( mov.vy - GM_WaterLevel ) > 1.0F && 
			 DG_FABS( step.vy ) > 0.10F ) {
//			_sceVu0Normalize( &step, &step ) ;
			_sceVu0ScaleVector( &step, &step, DG_FABS(( mov.vy - GM_WaterLevel ) / step.vy )) ;
//			_sceVu0SubVector( &mov, &mov, &step ) ;
			_sceVu0AddVector( &mov, &mov, &step ) ;
		} else {
			mov.vy = GM_WaterLevel ;
		}
//AN_Test_Eye2( &mov, 2 );
		map = GM_GetMapIDfromPos( GM_StageMapAll, mov0 ) ;
		GM_SetNoise( NOISE_MM, mov0, map ) ;

		( *PL_BulletSplashFunc )( &mov, width, force*0.5f, 0 ) ;
        return 1 ;
	}
	return 0 ;
}

