//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	skirt.c
	スカート

	2001/3/31 Y.Korekado
	$Id: skirt.c,v 1.1.1.3 2002/11/19 11:44:18 Yoshizawa1 Exp $

*/

/*----------------------------------------------------------------
----------------------------------------------------------------*/

#include <stdlib.h>
#include <stdio.h>
#ifdef PSX2
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libpkt.h>
#include <sifdev.h>
#include <libdev.h>
#include <libvu0.h>
#endif
#include <string.h>

#include	"gameheader.h"
#include	"korekado/conv/korekado.x"

/*----------------------------------------------------------------*/
#define	BODYWORLD(a,b) (a)->objs->objs[(b)].world

#define PRIO	0x60	/* 敵兵より後に処理 */

#define	MAX_PRIM (8)
/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX	actor ;
	OBJECT		*body ;
	DG_PRIM2	*prim ;
	FMATRIX		*lights ;
	FVECTOR		pos[ MAX_PRIM*2 + 2 ] ;

	FVECTOR		sft[ MAX_PRIM*2 + 2 ] ;
	short		base_obj[ MAX_PRIM*2 + 2 ] ;

	u_short		n_prim ;
	u_short		n_pos ;
} Work ;

/*----------------------------------------------------------------*/
static void MakePos ( world, sft, pos )
FMATRIX	*world ;
FVECTOR	*sft ;
FVECTOR	*pos ;
{
	DG_SetPos( world ) ;
	DG_PutVector( sft, pos, 1 ) ;	/* 絶対座標へ変換 */
}

static void SetLights( DG_PRIM2 *prim, FMATRIX *lights )
{
#if 0
	extern	void	Big_CalcRGB( UCVECTOR *rgb, FMATRIX *lights, FVECTOR *norm) ;
	DG_PRIM2_UVRGB	*uvrgb ;
	UCVECTOR rgb ;
	int i, j ;

	Big_CalcRGB( &rgb, lights, NULL ) ;

	uvrgb = prim->uvrgb[ prim->buffer_clock ] ;
	for ( i=0; i<N_PRIMS; i++ ) {
		for ( j=0; j<N_VERTS; j++ ) {
			uvrgb->r = rgb.r ;
			uvrgb->g = rgb.g ;
			uvrgb->b = rgb.b ;
			uvrgb ++ ;
		}
	}
#endif
}

static	void	SetPrimPos( work )
Work	*work ;
{
	int i ;
	
	for ( i=0; i<work->n_pos; i++ ) {
		MakePos( &BODYWORLD( work->body, work->base_obj[i]), &work->sft[i], &work->pos[i] ) ;
	}
	memcpy( work->prim->pos[work->prim->buffer_clock], &work->pos[0],
			sizeof(FVECTOR) * (work->n_pos) ) ;
}

static	void	SetInvisible( work )
Work	*work ;
{
	if ( work->body->objs->flag & DG_FLAG_INVISIBLE ) {
		work->prim->flag |= DG_PRIM2_INVISIBLE ;
	} else {
		work->prim->flag &= ~DG_PRIM2_INVISIBLE ;
		GM_GroupPrim2( work->prim, work->body->map_name ) ;
	}
}

static	void	Clairvoyance( Work *work )
{
	DG_PRIM2_UVRGB		*uvrgb ;
	int value, i ;
	static int power=0 ;
#if 1
	if( GV_PadData[ 0 ].pressure[ PAD_PRESS_R1 ] == 255 ){
		power ++ ;
	} else {
		power -= 8 ;
	}
	
	if ( power > 60 ) {
		if ( power > 210 ) power = 210 ;
		value = 128 - ( (power-60)/4 ) ;
	} else {
		if ( power < 0 ) power = 0 ;
		value = 128 ;
	}

#else
	if( GV_PadData[ 0 ].pressure[ PAD_PRESS_R1 ] >= 240 ){
		value = 128 - ( GV_PadData[ 0 ].pressure[ PAD_PRESS_R1 ] - 240 ) ;
//		value = GV_Time%128 ;
		printf("pressure[%d] alpha=%d\n",GV_PadData[ 0 ].pressure[ PAD_PRESS_R1 ], value );

	} else {
		value = 128 ;
	}
#endif

	uvrgb = work->prim->uvrgb[work->prim->buffer_clock] ;
	for( i=0; i<work->n_pos; i++ ){
		uvrgb->a = value ;
		uvrgb++ ;
	}
}

/*----------------------------------------------------------------*/
static	void	Act( work )
Work		*work ;
{
	SetInvisible( work ) ;

	DG_SwitchBuffPrim2( work->prim );
	SetPrimPos( work ) ;
	SetLights( work->prim, work->lights ) ;
	Clairvoyance( work ) ;
}

static	void	Die( work )
Work		*work ;
{
	GM_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
static	int	GetResources( work, tex_name, n_prim, uv_pix, body, sft, base_obj, lights )
Work	*work ;
int			tex_name ;
int			n_prim ;
short		*uv_pix ;
OBJECT		*body ;
FVECTOR		*sft ;
short		*base_obj ;
FMATRIX		*lights ;
{
	extern DG_PRIM2 *InitStripPrim( int tex_code, int n_strip, short *uv_pix, FVECTOR *pos ) ;
	DG_PRIM2	*prim ;
	int		i ;

	if ( (work->n_prim = n_prim) >= MAX_PRIM ) {
		printf(" skirt.c: Err Max Prim Over \n");
		return -1 ;
	}
	work->n_pos = (work->n_prim * 2) + 2 ;
	work->body = body ;
	work->lights = lights ;
	

	for ( i = 0 ; i < work->n_pos ; i++ ){
		work->sft[i] = *sft ;
		work->base_obj[i] = *base_obj ;
		sft++ ;
		base_obj++ ;
	}

	for ( i=0; i<work->n_pos; i++ ) {
		MakePos( &BODYWORLD( work->body, work->base_obj[i]), &work->sft[i], &work->pos[i] ) ;
	}

	prim = work->prim = InitStripPrim( tex_name, n_prim, uv_pix, work->pos );
	prim->flag |= DG_PRIM2_FOG ;

	return 0 ;
}

void		*New_KR_StripPrim( tex_name, n_prim, uv_pix, body, sft, base_obj, lights )
int			tex_name ;
int			n_prim ;
short		*uv_pix ;
OBJECT		*body ;
FVECTOR		*sft ;
short		*base_obj ;
FMATRIX		*lights ;
{
	Work		*work ;

	OPERATOR() ;
    work = (Work *)GV_CreateActor( GV_ACTOR_AFTER, GV_CLASS_CHARA, 
				   sizeof( Work ), PRIO ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor ) ;
		if ( GetResources( work, tex_name, n_prim, uv_pix, body, sft, base_obj, lights ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return work ;
}

//#define TEX_NAME_A	(12500430)

#define TEX_NAME_A	GV_StrCode("gbs_ak_sling2")

void	NewHostageSkirtA( body, lights )
OBJECT		*body ;
FMATRIX		*lights ;
{
	static FVECTOR sft[] = {
		{ -63.0f, -131.0f, 113.0f },
		{ 63.0f, -131.0f, 113.0f },
		{ 60.0f, -15.0f, 78.0f },
		{ -60.0f, -15.0f, 78.0f },
		{ 60.0f, -185.0f, 78.0f },
		{ -60.0f, -185.0f, 78.0f },
	} ;

	static u_short base_obj[] = {
		HUMAN21_KOSHI,
		HUMAN21_KOSHI,
		HUMAN21_MIGI_ASHI1,
		HUMAN21_HIDARI_ASHI1,
		HUMAN21_MIGI_ASHI1,
		HUMAN21_HIDARI_ASHI1,
	} ;

	static u_short uv_pix[] = {
		0, 0,
		7, 0,
		0, 3,
		7, 3,
		0, 7,
		7, 7
	} ;

	New_KR_StripPrim( TEX_NAME_A, 2, uv_pix, body, sft, base_obj, lights ) ;
}

