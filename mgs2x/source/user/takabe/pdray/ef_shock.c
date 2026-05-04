//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	ef_shock.c
	ÎÌ»º·¿£Ò£Á£ÙÍÑ¥À¥ß¡¼¥¨¥Õ¥§¥¯¥È¡Ê¿¶Æ°¾×·âÇÈ¡Ë

	2001/05/21 K.Takabe
	$Id: ef_shock.c,v 1.1.1.3 2002/11/19 11:51:21 Yoshizawa1 Exp $

*/
/*



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
#include	"def_dma.h"
#include	"libmt.h"
#include	"gameheader.h"
#include	"debugmenu.h"


#include	"../other/vec_util.h"

/* ---------------------------------------------------------------- */
/* ÈÆÍÑ¥Þ¥¯¥í */
#define FTOI12(_f)	( DG_FTOI( ( (float)(_f)*4096.0f) ) )
#define FTOI(_f)	( DG_FTOI(_f) )
#define RND(_n)		( (int)( ( (u_long64)BP_PS2_rand()*(_n) ) / ((u_long64)BP_PS2_RAND_MAX+1) ) )
/* ---------------------------------------------------------------- */
#define TOTAL_DIV	(64)
#define PARTS_DIV	(8)
/* ---------------------------------------------------------------- */
#define PRIM_FLAG	(DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA)
#define N_PRIMS		(TOTAL_DIV/PARTS_DIV)
#define N_VERTS		(PARTS_DIV+2)
//#define TEXTURE		(1084957)		/* "smoke_lp1_alp" */
//#define TEXTURE		(2133533)		/* "smoke_lp2_alp" */
//#define TEXTURE		(3182109)		/* "smoke_lp3_alp" */
#define TEXTURE		(13145893)			/* "wind01" */

#define FADE_TIME	(300)
/* ---------------------------------------------------------------- */
#define	SET_UVRGB_UV( _p, _u, _v )	{ (_p)->u = (_u) ; (_p)->v = (_v) ; (_p)->q = 4096 ; }
#define	SET_UVRGB_COL( _p, _r, _g, _b, _a )	{ (_p)->r = (_r) ; (_p)->g = (_g) ; (_p)->b = (_b) ; (_p)->a = (_a) ; }

/* ---------------------------------------------------------------- */
typedef struct {
	GV_ACT_EX	actor ;
	int			map ;
	int			hzx_id ;

	FMATRIX		pos_mat ;
	DG_PRIM2	*prim ;

	int				count ;
} Work ;


/* ---------------------------------------------------------------- */
void PDRAY_CLOUD_DeleteDensity( FVECTOR *pos, int size );
/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	DG_PRIM2	*prim ;
	FVECTOR		*pos ;
	DG_PRIM2_UVRGB	*uvrgb ;
	float		t ;
	int			i, j ;

	GM_CurrentMap = work->map ;
	work->count++ ;

	prim = work->prim ;
	prim->flag &= ~DG_PRIM2_INVISIBLE ;
	DG_SwitchBuffPrim2( prim );
	pos = prim->pos[ prim->buffer_clock ] ;
	uvrgb = prim->uvrgb[ prim->buffer_clock ] ;

	t = (float)work->count / (float)FADE_TIME ;
	{
		FMATRIX	scale_mat ;
		float	scale_xz, scale_y ;
		GTE_UnitMatrix( &scale_mat );
		scale_xz = 1.0f + t ;
		scale_y = 1.0f - t ;
		scale_mat.m[0][0] *= scale_xz ;
		scale_mat.m[1][1] *= scale_y ;
		scale_mat.m[2][2] *= scale_xz ;
		GTE_MulMatrix( &prim->as_world, &work->pos_mat, &scale_mat );
	}

	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( j = 0 ; j < N_VERTS ; j+=2, pos+=2, uvrgb+=2 ){
			uvrgb[1].a = ( 1.0f - t ) * 128 ;
		}
	}

	work->count += TIME_BASE * 3 ;

	if ( work->count > FADE_TIME ){
		GV_DestroyActor( work );
	}
}
/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
	DG_DequeuePrim2( work->prim );
	DG_FreePrim2( work->prim );
}
/* ---------------------------------------------------------------- */
static int GetResources( Work *work, FMATRIX *parent, float in_size, float out_size, float height )
{
	DG_PRIM2	*prim ;
	DG_PRIM2_UVRGB	*uvrgb ;
	FVECTOR		*pos ;
	DG_TEX		*tex ;
	int			i, j, k ;
	int			u0, v0, u1, v1 ;

	work->map = GM_CurrentMap ;
	work->pos_mat = *parent ;
	{
		FMATRIX	mat ;
		GTE_MakeRotate( &mat, 0, 1, 0, GTE_PS2RAD( RND( 4096 ) ) );
		GTE_MulMatrix( &work->pos_mat, &work->pos_mat, &mat );
	}

	tex = DG_GetTexture( TEXTURE );
	u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset ) ;
	v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset ) ;
	u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset ) ;
	v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset ) ;

	prim = work->prim = GM_MakePrim2( PRIM_FLAG, N_PRIMS, N_VERTS );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 128 ) );
	prim->flag |= DG_PRIM2_INVISIBLE ;
   prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
	prim->as_world = *parent ;

	for ( k = 0 ; k < 2 ; k++ ){
		pos = prim->pos[ k ];
		uvrgb = prim->uvrgb[ k ];
		for ( i = 0 ; i < N_PRIMS ; i++ ){
			for ( j = 0 ; j < N_VERTS ; j+=2, pos+=2, uvrgb+=2 ){
				FVECTOR		sincos, base_vec ;
				float angle ;
				int		iangle ;

				iangle = ( i * (PARTS_DIV) + j ) * 2048 / (TOTAL_DIV/2) ;
				angle = GTE_PS2RAD( iangle );
				GTE_SinCos( &sincos, angle );
				base_vec.vx = sincos.vy ;
				base_vec.vy = 0.0f ;
				base_vec.vz = sincos.vx ;
				base_vec.vw = 0.0f ;
				GTE_ScaleVector( &pos[0], &base_vec, out_size );
				GTE_ScaleVector( &pos[1], &base_vec, in_size );
				pos[0].vy = height ;

				uvrgb[0].r = 48 ;
				uvrgb[0].g = 48 ;
				uvrgb[0].b = 48 ;
				uvrgb[0].a = 0 ;
				uvrgb[1] = uvrgb[0] ;
				pos[1].vy = 0.0f ;
				uvrgb[1].a = 128 ;

				if ( j & 8 ){
					int		rate = ( j >> 1 ) & 3 ;
					uvrgb[0].u = ( u1 - u0 ) * rate / 4 + u0 ;
					uvrgb[0].v = v0 ;
					uvrgb[1].u = ( u1 - u0 ) * rate / 4 + u0 ;
					uvrgb[1].v = v1 ;
				} else {
					int		rate = ( j >> 1 ) & 3 ;
					uvrgb[0].u = ( u0 - u1 ) * rate / 4 + u1 ;
					uvrgb[0].v = v0 ;
					uvrgb[1].u = ( u0 - u1 ) * rate / 4 + u1 ;
					uvrgb[1].v = v1 ;
				}
				uvrgb[0].q = 4096 ;
				uvrgb[1].q = 4096 ;

				uvrgb[0].f = 0x0fff ;
				uvrgb[1].f = 0x0fff ;
				if ( j == 0 ){
					uvrgb[0].f = 0x8fff ;
					uvrgb[1].f = 0x8fff ;
				}
			}
		}
	}
	return ( 0 );
}
/* ---------------------------------------------------------------- */
void *_NewDummyEarthquakeShock( FMATRIX *parent, int in_size, int out_size, int height )
{
	Work		*work ;

	OPERATOR() ;
	work = (Work *)GV_NewActor( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );
		if ( GetResources( work, parent, in_size, out_size, height ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewDummyEarthquakeShock( FMATRIX *parent, int in_size, int out_size, int height )
{
	void *dummy_work ;
	height *= 3 ;
	in_size /= 2 ;
	out_size /= 2 ;
	dummy_work = _NewDummyEarthquakeShock( parent, in_size, out_size, height );
	_NewDummyEarthquakeShock( parent, in_size, out_size + ( out_size - in_size ) * 1, height/2 );
	_NewDummyEarthquakeShock( parent, in_size, out_size + ( out_size - in_size ) * 2, height/3 );
	_NewDummyEarthquakeShock( parent, in_size, out_size + ( out_size - in_size ) * 3, height/4 );

	if ( parent->m[3][1] > 3900.0f && parent->m[3][1] < 4200.0f ){
		PDRAY_CLOUD_DeleteDensity( (FVECTOR*)parent->m[3], out_size + ( out_size - in_size ) * 3 );
	}
	return ( dummy_work );
}

