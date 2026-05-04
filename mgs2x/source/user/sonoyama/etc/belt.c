//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
   belt.c
   ベルトコンベアのベルトモデル表示

   2001/05/25	M.Sonoyama
   $Id: belt.c,v 1.1.1.3 2002/11/19 11:50:40 Yoshizawa1 Exp $
*/

#include	<stdio.h>
#include	<stdlib.h>
#include	<string.h>
#include	<sys/types.h>

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

#include "BP_Misc.h"

/*----------------------------------------------------------------*/

/* 反対まわしはできません */

typedef struct {
	short	vx;
	short	vy;
	short	vz;
	short	u;
	short	v;
	short	flags;
	short	count;
	short	nx;
	short	ny;
	short	nz;
} VERTS_DATA;

#define	WIDE	(999)

static 	VERTS_DATA	Verts[] = {
	{    0,    0,	0,    			0, 		4096, 	0x8fff, 567,   0, -4096,    0, },
	{    0,    0,	-WIDE * 2 / 3,  0,    	4096 / 3, 		0x8fff, 1567,  0, -4096,    0, },
	{ WIDE,    0,   0, 				4096, 	4096, 	0x0fff, 567,   0, -4096,    0, },
	{ WIDE,    0, 	-WIDE * 2 / 3, 	4096,   4096 / 3, 		0x0fff, 1567,  0, -4096,    0, }
} ;

extern	void TS_PolyVertsLight( FVECTOR *ppos, DG_PRIM2_UVRGB *puv, int n_verts, 
							    int alpha, VERTS_DATA *vdata, FMATRIX *light, FMATRIX *root ) ;

#define	PRIM_FLAG	(DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_FOG)
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )

#define	V_SHIFT(_s)	(_s*(4096)/(WIDE))

static	inline	void	SetTexV( DG_PRIM2_UVRGB *uvrgb, DG_TEX *tex, float v )
{
	uvrgb->v = FTOI12( v / 4096.0F * tex->v_scale + tex->v_offset ) ;
}

typedef	struct	{
	GV_ACT			actor ;
	FVECTOR			pos[ 2 ] ;
	DG_TEX			*tex ;
	DG_PRIM2		*prims ; 
	int				n_prims ;
	int				name ;
	int				status ;
	float			speed ;
	float			v_shift ;
	VERTS_DATA		appVerts[ 4 ] ;
} Work ;

#define	AppVerts	(work->appVerts)

enum {
	BELT_STOP	= 0,
	BELT_MOVE 	= 1,
} ;

extern	float	BeltSpeedScale ;

/*----------------------------------------------------------------*/

#if 0
static	void	CheckMessage( Work *work )
{
	GV_MSG		*msg ;
	int			n_msg ;

	n_msg = GV_ReceiveMessage( work->name, &msg ) ;
	while( -- n_msg >= 0 ) {
		if ( msg->message[ 0 ] == 0 ) {	/* start */
			work->status = BELT_MOVE ;
		} else if ( msg->message[ 0 ] == 1 ) {	/* stop */
			work->status = BELT_STOP ;
		}
		msg ++ ;
	}
}
#endif

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	DG_PRIM2		*prims ;
	DG_PRIM2_UVRGB	*uvrgb ;
	float			shift ;
	int				i, j ;

	//CheckMessage( work ) ;

	prims = work->prims ;

	switch( work->status ) {
	case BELT_STOP :
		break ;
	case BELT_MOVE :
		if ( BeltSpeedScale == 0.0F ) break ;
		DG_SwitchBuffPrim2( prims ) ;

      if ( BP_IsPAL()==TRUE )
		   work->v_shift += work->speed * BeltSpeedScale * 60.0F / 50.0F ;
      else
		   work->v_shift += work->speed * BeltSpeedScale ;

      if ( work->v_shift > ( float )WIDE / 3.0F ) {
			work->v_shift -= ( float )WIDE / 3.0F ;
		}
		shift = V_SHIFT( work->v_shift ) ;
		uvrgb = ( DG_PRIM2_UVRGB * )prims->uvrgb[ prims->buffer_clock ] ;		
		for ( i = 0; i < work->n_prims - 1; i ++ ) {
			for ( j = 0; j < 4; j ++ ) {
				SetTexV( uvrgb, work->tex, ( float )Verts[ j ].v - shift ) ;
				uvrgb ++ ;
			}
			//NewSquareView( prims->pos[ prims->buffer_clock ] + i * 4, 1, 32, 232, 32 ) ;
		}
		/* 最後の一枚 */
		for ( j = 0; j < 4; j ++ ) {
			SetTexV( uvrgb, work->tex, ( float )AppVerts[ j ].v - shift ) ;
			uvrgb ++ ;
		}
		GM_SetCurrentMap( GM_CurrentStageMap ) ;
		//NewSquareView( prims->pos[ prims->buffer_clock ] + i * 4, 1, 32, 232, 32 ) ;
	}
}

static	void	Die( Work *work )
{
	GM_FreePrim2( work->prims ) ;
}

/*----------------------------------------------------------------*/

static	int	GetResources( Work *work, int name, int where )
{
	DG_DEF		*def ;
	FVECTOR		diff, step, mov ;
	FMATRIX		world ;
	SVECTOR		rot ; 
	int			i ;
	int			model, n_prims, tex_name ;
	int			onelen, fulllen ;
	DG_TEX		*tex ;
	DG_PRIM2	*prims ;
	FVECTOR		*ppos ;
	float		wx, wz ;
	DG_PRIM2_UVRGB	*uvrgb ;
	VERTS_DATA		*verts ;

	work->name = name ;

	GCL_GetOption( 'p' ) ;
	PL_GetNextFV( &work->pos[ 0 ] ) ;
	PL_GetNextFV( &work->pos[ 1 ] ) ;
	_sceVu0SubVector( &diff, &work->pos[ 1 ], &work->pos[ 0 ] ) ;

	model = GCL_GetOptionValue( 'm', GV_StrCode( "w20a_catp" ) ) ;
	def = ( DG_DEF * )GV_GetCache( GV_CacheID( model, 'k' ) ) ;
	tex_name = GCL_GetOptionValue( 't', GV_StrCode( "w20a_bel_b02" ) ) ;
	tex = work->tex = DG_GetTexture( tex_name ) ;
	ASSERT( tex != NULL ) ;

	/* 何枚張れば良いか調べる */
	//onelen = ( int )( def->models[ 0 ].uz - def->models[ 0 ].lz * 2.0F / 3.0F ) ;
	onelen = ( int )( ( float )WIDE * 2.0F / 3.0F ) ;
	fulllen = ( int )GV_VecLen3F( &diff ) ;
	n_prims = fulllen / onelen ;
	if ( n_prims == 0 ||
		( ( fulllen / onelen ) > 0 && ( fulllen % onelen ) ) ) n_prims ++ ;

	work->n_prims = n_prims ;

	/* プリミティブ生成 */
	prims = work->prims = GM_MakePrim2( PRIM_FLAG, n_prims, 4 ) ;
	GM_GroupPrim2( prims, GM_CurrentMap ) ;
	ASSERT( prims != NULL ) ;
	DG_ConfigPrim2Tex( prims, tex ) ;
	//DG_SetPrim2Alpha( prims, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0 ) ) ;

	GV_LenVec3F( &diff, &step, 0.0F, ( float )onelen ) ;
	rot.vx = rot.vz = 0 ;
	rot.vy = GV_VecDir2( &diff ) ;

	DG_COPY_VEC( &mov, &work->pos[ 0 ] ) ;
	DG_SetPos2( &mov, &rot ) ;
	//wx = ( def->models[ 0 ].ux - def->models[ 0 ].lx ) / 2.0F ;
	//wz = ( def->models[ 0 ].uz - def->models[ 0 ].lz ) * 2.0F / 3.0F ;
	wx = ( float )WIDE / 2.0F ;
	wz = ( float )WIDE * 2.0F / 3.0F ;

	mov.vx = - wx ;
	mov.vy = 0.0F ;
	mov.vz = wz ;
	DG_MovePos( &mov ) ;
	DG_GetPos( &world ) ;
	GV_MatToVec( &world, &mov ) ;

	memcpy( AppVerts, Verts, sizeof( VERTS_DATA ) * 4 ) ;

	for ( i = 0; i < n_prims; i ++ ) {
		int				j, k ;

		verts = Verts ;
		if ( i == n_prims - 1 ) {
			int			applen ;

			applen = fulllen % onelen ;	
			if ( DG_FABS( ( float )applen ) > 0.10F ) {
				AppVerts[ 0 ].vz = AppVerts[ 2 ].vz = -WIDE*2/3 + applen ;
				AppVerts[ 0 ].v = AppVerts[ 2 ].v
					= 4096/3 + ( int )((4096.0F*2.0F/3.0F)*(float)applen/((float)WIDE*2.0F/3.0F)) ;
				verts = AppVerts ;
			}
		}

		for ( k = 0; k < 2; k ++ ) {
			ppos = prims->pos[ k ] ;
			uvrgb = ( DG_PRIM2_UVRGB * )prims->uvrgb[ k ] ;

			for ( j = 0; j < 4; j ++ ) {
				uvrgb[ i * 4 + j ].u = FTOI12( ( float )verts[ j ].u / 4096.0F 
											  * tex->u_scale + tex->u_offset ) ;
				uvrgb[ i * 4 + j ].v = FTOI12( ( float )verts[ j ].v / 4096.0F 
											  * tex->v_scale + tex->v_offset ) ;
				uvrgb[ i * 4 + j ].a = 128 ;

				ppos[ i * 4 + j ].vw = 1.0F ;

				uvrgb[ i * 4 + j ].f = ( j < 2 ) ? 0x8fff : 0x0fff ;
				uvrgb[ i * 4 + j ].q = 4096 ;

				{
					FVECTOR		p ;
					FMATRIX		pl[ 2 ] ;

					p.vx = ( float )verts[ j ].vx ;
					p.vy = ( float )verts[ j ].vy ;
					p.vz = ( float )verts[ j ].vz ;
					DG_SetPos( &world ) ;
					DG_PutVector( &p, &p, 1 ) ;
					DG_GetLightMatrix( &p, pl ) ;
					TS_PolyVertsLight( ppos + i * 4 + j, uvrgb + i * 4 + j, 1, 128, 
									   &verts[ j ], pl, &world ) ;					
				}
			}
		}
		_sceVu0AddVector( &mov, &mov, &step ) ;
		GV_VecToMat( &mov, &world ) ;
	}

	work->speed = ( float )GCL_GetOptionValue( 's', 0 ) ;
	//work->status = GCL_GetOptionValue( 'A', BELT_MOVE ) ;
	work->status = BELT_MOVE ;

	return 0 ;
}

/*----------------------------------------------------------------*/

void	*NewBeltConveyerBelt( int name, int where )
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
