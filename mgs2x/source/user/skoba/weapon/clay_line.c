//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   clay_line.c
   クレイモアの線

   2001/07/07	M.Sonoyama
   $Id: clay_line.c,v 1.1.1.3 2002/11/19 11:50:27 Yoshizawa1 Exp $
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

#define	TEXTURE		TXT_LSIGHT_LINE
#define	PRIM_FLAG	(DG_PRIM2_LINE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING|DG_PRIM2_SHADE)
#define	N_PRIMS		(1)
#define	N_VERTS		(8)

#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

typedef	struct	{
	GV_ACT_EX			actor ;
	DG_PRIM2			*prim ;
	DG_TEX				*tex ;
	FMATRIX				*world ;
	int					*flag ;
	float				cur_v0 ;
	int					speed ;
} Work ;

/*----------------------------------------------------------------*/

static	FVECTOR		Center = { -15.0F, -200.0F, 0.0F } ;

static	FVECTOR		LineDatas[] = {
	{ -90.0F, -90.0F, 90.0F }, { -1025.0F, -350.0F, 750.0F },
	{ -90.0F, -90.0F, -50.0F }, { -1025.0F, -350.0F, -50.0F },
	{ -90.0F, 90.0F, 90.0F }, { -1025.0F, 350.0F, 750.0F },
	{ -90.0F, 90.0F, -50.0F }, { -1025.0F, 350.0F, -50.0F }
} ;

/*----------------------------------------------------------------*/

static	void	UpdateWorld( Work *work )
{
	FMATRIX		tmpMat, tmpMat2, *world ;
	SVECTOR	rot = { 0, 0, 30 * 1024 / 90 } ;
	
	world = work->world ;
	DG_COPY_MAT( &tmpMat, world ) ;
	GV_VecToMat( &DG_ZeroVector, &tmpMat ) ;
	DG_SetPos2( &DG_ZeroVector, &rot ) ;
	DG_GetPos( &tmpMat2 ) ;
	_sceVu0MulMatrix( &tmpMat, &tmpMat, &tmpMat2 ) ;
	GV_VecToMat( ( FVECTOR * )world->m[ 3 ], &tmpMat ) ;
	DG_SetPos( &tmpMat ) ;
	DG_MovePos( &Center ) ;
	DG_GetPos( &work->prim->as_world ) ;
	GM_GroupPrim2( work->prim, GM_PlayerMap ) ;
}

static	void	UpdateTexV( Work *work )
{
	DG_PRIM2_UVRGB	*uvrgb ;
	DG_TEX			*tex ;
	float			v0, d ;
	int				i ;

	uvrgb = work->prim->uvrgb[ work->prim->buffer_clock ] ;
	tex = work->tex ;
	v0 = work->cur_v0 ;

	d = 1.0F ;
	for ( i = 0; i < N_VERTS / 2; i ++ ) {
		uvrgb[ i * 2 ].v = uvrgb[ i * 2 + 1 ].v = FTOI12( v0 * tex->v_scale + tex->v_offset ) ;
		v0 += 64.0F / 256.0F * d ;
		if ( v0 >= 1.0F ) {
			v0 = 1.0F - ( v0 - 1.0F ) ;
			d = -1.0F ;
		} else if ( v0 < 0.0F ) {
			v0 = -v0 ;
			d = 1.0F ;
		}
	}

	work->cur_v0 += 1.0F / 256.0F * ( float )work->speed ;
	if ( work->cur_v0 >= 1.0F ) {
		work->cur_v0 = 1.0F - ( work->cur_v0 - 1.0F ) ;
		work->speed = BP_PS2_rand() % 2 + 1 * -1 ;
	} else if ( work->cur_v0 < 0.0F ) {
		work->cur_v0 = -work->cur_v0 ;
		work->speed = BP_PS2_rand() % 2 + 1 ;
	}
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	if ( ( *work->flag ) == 0 ) {
		DG_InvisiblePrim2( work->prim ) ;
		return ;
	}
	DG_SwitchBuffPrim2( work->prim ) ;
	DG_VisiblePrim2( work->prim ) ;
	UpdateTexV( work ) ;
	UpdateWorld( work ) ;
}

static	void	Die( Work *work )
{
	GM_FreePrim2( work->prim ) ;
}

/*----------------------------------------------------------------*/

static	int		GetResources( Work *work, FMATRIX *world, int *flag )
{
	DG_PRIM2		*prim ;
	DG_TEX			*tex ;
	DG_PRIM2_UVRGB	*uvrgb ;
	FVECTOR			*pos ;
	int				i, j, k ;

	prim = work->prim = GM_MakePrim2( PRIM_FLAG, N_PRIMS, N_VERTS ) ;
	if ( prim == NULL ) return -1 ;

   work->prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
   
   tex = work->tex = DG_GetTexture( TEXTURE ) ;
	DG_ConfigPrim2Tex( prim, tex ) ;
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) ) ;	
	for ( k = 0; k < 2; k ++ ) {
		for ( i = 0; i < N_PRIMS; i ++ ) {
			uvrgb = prim->uvrgb[ k ] ;
			pos = prim->pos[ k ] ;
			for ( j = 0; j < N_VERTS; j ++ ) {
				if ( j & 1 ) {
					uvrgb[ j ].u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
					uvrgb[ j ].v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
					uvrgb[ j ].f = 0x0fff ;				
				} else {
					uvrgb[ j ].u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
					uvrgb[ j ].v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
					uvrgb[ j ].f = 0x8fff ;
				}
				uvrgb[ j ].r = 32 ;
				uvrgb[ j ].g = 32 ;
				uvrgb[ j ].b = 32 ;
				uvrgb[ j ].a = 32 ;
				uvrgb[ j ].q = 4096 ;
				DG_COPY_VEC( &pos[ j ], &LineDatas[ j ] ) ;
			}
		}
	}
	work->cur_v0 = 0.0F ;
	work->speed = 1 ;

	work->world = world ;
	work->flag = flag ;

	UpdateWorld( work ) ;
	DG_InvisiblePrim2( prim ) ;
	return 0 ;
}

/*----------------------------------------------------------------*/

void	*NewClaymoreLine( FMATRIX *world, int *flag )
{
	Work		*work ;

	work = ( Work * )GV_CreateActor( GV_ACTOR_PLAYER, GV_CLASS_OBJECT, 
									 sizeof( Work ), PLAYER_WEAPON_ACTOR_PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &work->actor, Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, world, flag ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}



