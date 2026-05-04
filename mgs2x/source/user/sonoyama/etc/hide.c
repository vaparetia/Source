//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   hide.c
   隠し君

   M.Sonoyama 2000/06/07
   $Id: hide.c,v 1.1.1.3 2002/11/19 11:50:43 Yoshizawa1 Exp $
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
#include	"camera.h"

typedef	struct	{
	GV_ACT			actor ;
	TARGET			attack[ 9 ] ;
	TARGET			parent ;
	TARGET			child[ 22 ] ;
	DG_MENU2_PRIM	*prim ;
	int				hide_count ;
} Work ;

/*----------------------------------------------------------------*/

static	void	Hit( off, def, ptr )
TARGET			*off, *def ;
void			*ptr ;
{
	Work		*work ;

	work = ( Work * )ptr ;
//	printf( "[%d] ouch\n", GV_Time ) ;
    DG_VisibleMenu2Prim( work->prim ) ;
	work->hide_count = 16 ;
}

static	void	Hit2( off, def, ptr )
TARGET			*off, *def ;
void			*ptr ;
{

}

/*----------------------------------------------------------------*/

static	void	InitPrim( work )
Work			*work ;
{
	DG_MENU2_PRIM	*prim ;
	DG_MENU2_PACKET	*packs ;
	DG_MENU2_POLY_G	*poly ;
	int				i ;

	prim = work->prim = DG_MakeMenu2Prim( DG_MENU2_PRIM_POLY | DG_PRIM2_SHADE,
										  1, 254 ) ;
	if ( prim == NULL ) return ;
	for ( i = 0; i < 2; i ++ ) {
		packs = ( DG_MENU2_PACKET * )prim->packet[ i ] ;
		poly = ( DG_MENU2_POLY_G * )packs->data ;
		poly->prim.ABE = 0 ;
		DG_MENU2_SET_RGBA1( poly, 0, 0, 0, 128 ) ;
		DG_MENU2_SET_RGBA2( poly, 0, 0, 0, 128 ) ;
		DG_MENU2_SET_RGBA3( poly, 0, 0, 0, 128 ) ;
		DG_MENU2_SET_RGBA1( poly, 0, 0, 0, 128 ) ;
		DG_MENU2_SET_XY1( poly, 0, 0 ) ;
		DG_MENU2_SET_XY2( poly, BUFFER_WIDTH, 0 ) ;
		DG_MENU2_SET_XY3( poly, 0, BUFFER_HEIGHT ) ;
		DG_MENU2_SET_XY4( poly, BUFFER_WIDTH, BUFFER_HEIGHT ) ;
	}
}

/*----------------------------------------------------------------*/

static	void	Act( work )
Work			*work ;
{
	TARGET		*t, *c, *p ;
	FMATRIX		mat ;
	FVECTOR		pos, off, from, to, from2, diff ;
	float		len ;
	GM_CameraSet	*cam ;
	int				i ;

	if ( work->hide_count > 0 &&
		 -- work->hide_count == 0 ) {
		DG_InvisibleMenu2Prim( work->prim ) ;
	}

	p = GM_PlayerTarget ;
	t = &work->parent ;
	c = work->child ;

	GM_SetCurrentMap( GM_PlayerMap ) ;

	GM_MoveTarget3( t, &GM_PlayerBody->objs->world ) ;
	GM_MoveTarget2( c, &GM_PlayerBody->objs->objs[ 0 ].world ) ;

	for ( i = 0; i < 21; i ++ ) {
		GM_MoveTarget2( c + i + 1, &GM_PlayerBody->objs->objs[ i ].world ) ;
	}


//	NewTargetView2( t, 32, 232, 32 ) ;
//	NewTargetView2( c, 232, 232, 32 ) ;
	for ( i = 0; i < 21; i ++ ) {
//		NewTargetView2( c + i + 1, 32, 232, 232 ) ;	
	}

	/* 画面内チェック */
    _sceVu0ApplyMatrix( &off, &c->world, &c->offset ) ;
	_sceVu0AddVector( &pos, &c->center, &off ) ;
	
	cam = GM_GetCurrentCamera( 0 ) ;
	DG_COPY_VEC( &from, &cam->position ) ;
//	_sceVu0SubVector( &diff, &cam->target, &cam->position ) ;
//	len = GV_VecLen3F2( &cam->position, &pos ) ;
//	GV_LenVec3F( &diff, &diff, 0.0F, len ) ;
//	_sceVu0SubVector( &from2, &pos, &diff ) ;
	/* 中心 */
	if ( !( DG_PointCheckOne( &pos, 0x1 ) & 1 ) &&
		 !HZX_OnlineHazardCheck( GM_PlayerControl->hzx_id, &from, &pos,
							     HZX_CHK_ALL, 0, 0 ) ) {
		GM_MoveOnlineTarget( &work->attack[ 0 ], &from, &pos ) ;
		GM_PutTarget( &work->attack[ 0 ] ) ;
	}

	/* ８点チェックなので重々 */
	DG_COPY_MAT( &mat, &c->world ) ;
	GV_VecToMat( &pos, &mat ) ;
	DG_SetPos( &mat ) ;
	for ( i = 0; i < 8; i ++ ) {
		to.vx = ( i & 1 ) ? c->size.vx - 1.0F : - c->size.vx + 1.0F ;
		to.vy = ( i & 2 ) ? c->size.vy - 1.0F : - c->size.vy + 1.0F ;
		to.vz = ( i & 4 ) ? c->size.vz - 1.0F : - c->size.vz + 1.0F ;
		DG_PutVector( &to, &to, 1 ) ;
//		_sceVu0SubVector( &from2, &to, &diff ) ;
		if ( ( DG_PointCheckOne( &to, 0x1 ) & 1 ) || 
			 HZX_OnlineHazardCheck( GM_PlayerControl->hzx_id, &from, &to,
								    HZX_CHK_ALL, HZX_SEG_NO_ENEMY_EYES, 0 ) ) {
			continue ;
		}
		t = &work->attack[ i + 1 ] ;
		GM_MoveOnlineTarget( t, &from, &to ) ;
		GM_PutTarget( t ) ;
//		ViewFromTo( &from, &to, 32, 242, 32 ) ;
	}
}

static	void	Die( work )
Work			*work ;
{
	GM_FreeTarget( &work->parent ) ;
	DG_FreeMenu2Prim( work->prim ) ;
}

/*----------------------------------------------------------------*/

static	int	GetResources( work )
Work		*work ;
{
	TARGET		*t, *c, *p, *ct ;
	FVECTOR		size, off ;
	int			i ;

	p = GM_PlayerTarget ;
	t = &work->parent ;
	c = work->child ;
	size.vx = size.vy = size.vz = 1000.0F ;
	GM_SetTarget( t, TARGET_DEFENSE | TARGET_CHILD | TARGET_SEEK, 
				  0, BOTH_SIDE, &size, &DG_ZeroVector ) ;
	GM_PutTarget( t ) ;
	GM_MoveTarget3( t, &GM_PlayerBody->objs->world ) ;

	size.vx = size.vy = 100.0F ; size.vz = 20.0F ;
	off.vx = 0.0F ;
	off.vy = -200.0F ; 
	off.vz = 200.0F ;
	GM_SetTarget( c, TARGET_DEFENSE | TARGET_SEEK | TARGET_ROTATE, 
				  0, BOTH_SIDE, &size, &off ) ;
	GM_MoveTarget2( c, &GM_PlayerBody->objs->objs[ 0 ].world ) ;

	for ( i = 0; i < 21; i ++ ) {
		ct = c + i + 1 ;
		off.vx = ( GM_PlayerBody->objs->def->models[ i ].lx +
				   GM_PlayerBody->objs->def->models[ i ].ux ) / 2.0F ;
		off.vy = ( GM_PlayerBody->objs->def->models[ i ].ly +
				   GM_PlayerBody->objs->def->models[ i ].uy ) / 2.0F ;
		off.vz = ( GM_PlayerBody->objs->def->models[ i ].lz +
				   GM_PlayerBody->objs->def->models[ i ].uz ) / 2.0F ;
		size.vx = GM_PlayerBody->objs->def->models[ i ].ux - off.vx ;
		size.vy = GM_PlayerBody->objs->def->models[ i ].uy - off.vy ;
		size.vz = GM_PlayerBody->objs->def->models[ i ].uz - off.vz ;
		GM_SetTarget( ct, TARGET_DEFENSE | TARGET_SEEK | TARGET_ROTATE, 
					  0, BOTH_SIDE, &size, &off ) ;		
		GM_MoveTarget2( ct, &GM_PlayerBody->objs->objs[ i ].world ) ;
	}
	GM_TargetConnectChild( t, c, 22, 0 ) ;
	GM_SetTargetCallBack( c, Hit, work ) ;

	t = work->attack ;
	for ( i = 0; i < 9; i ++ ) {
		GM_SetTarget( t, TARGET_OFFENSE | TARGET_SEEK | TARGET_ONLINE | 
					  TARGET_ONLINE_MIN | TARGET_CHILD,
					  0, BOTH_SIDE, &DG_ZeroVector, &DG_ZeroVector ) ;	
		t ++ ;
	}

	InitPrim( work ) ;
	DG_InvisibleMenu2Prim( work->prim ) ;
	
	return 0 ;
}

void	*NewHideMan( name, where )
int		name, where ;
{
	Work		*work ;
	
	work = ( Work * )GV_CreateActor( GV_ACTOR_AFTER, GV_CLASS_OBJECT,
									 sizeof( Work ), 254 ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}
