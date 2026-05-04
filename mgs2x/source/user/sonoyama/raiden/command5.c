//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   command5.c
   プレイヤー／シナリオリンク関数（その５）
   
   2001/06/28 M.Sonoyama
   $Id: command5.c,v 1.1.1.3 2002/11/19 11:50:54 Yoshizawa1 Exp $
*/
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#ifndef KP_XBOX
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include	"gameheader.h"
#include	"camera.h"
#include	"pl_define.h"
#include	"pl_work.h"
#include	"pl_inline.h"

/* ダンボール瞬時破壊 */
extern void *PutCBoxBreakBody( FMATRIX *world, int map ) ;

int			PL_COM_DestroyBox( void )
{
	int		item ;

	ASSERT( GM_PlayerWork != NULL && GM_PlayerBody != NULL && GM_PlayerControl != NULL ) ;
	item = PL_GetPlayerItem() ;
	if ( !( GM_ItemTypes[ item ] & IT_TYPE_CBBOX ) ) {
		printf( "destroy box : not cb_box\n" ) ;
		return 0 ;
	}
	GM_SetItemNum( item, 0 ) ;
	PutCBoxBreakBody( &GM_PlayerBody->objs->world, GM_PlayerControl->map ) ;
	PL_NoEquip( GM_PlayerWork ) ;
	return 0 ;
}

/* 主観左右回転範囲設定 */
typedef	struct	{ 
	GV_ACT_EX		actor ;
	short			center ;
	short			range ;
} Work1 ;

static	void	Act1( Work1 *work )
{
	int			dir, c ;
	if ( GM_PlayerControl == NULL || !GM_CheckPlayerStatus( PLAYER_WATCH ) ) return ;
	c = 0 ;
	dir = GM_CameraDir.vy ;
	dir = GV_NearPhase( dir, work->center ) ;
	if ( dir > work->center + work->range ) {
		dir = work->center + work->range ; c = 1 ;
	} else if ( dir < work->center - work->range ) {
		dir = work->center - work->range ; c = 1 ;
	}
	if ( c ) GM_PlayerControl->turn.vy = dir ;
	GM_CameraDir.vy = dir ;
}

void	*PL_CHARA_SetSubjectLimit( int name, int where )
{
	Work1		*work ;
	int			max, min ;
	work = GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT, 
						   sizeof( Work1 ), PLAYER_BODY_ACTOR_PRIO + 1 ) ;
	if ( work == NULL ) return NULL ;
	GV_SetActor( &work->actor, Act1, NULL ) ;
	GCL_GetOption( 'r' ) ;
	min = GCL_GetNextInt() ;
	max = GCL_GetNextInt() ;
	if ( min > max ) max += 4096 ;
	work->center = ( max + min ) / 2 ;
	work->range = ( max - min ) / 2 ;
	if ( work->range < 0 ) work->range *= -1 ;
	return work ;
}
