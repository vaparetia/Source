//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	window_light.c
	差し込む光
	2000/12/18 S.Okajima
	$Id: window_light.c,v 1.1.1.3 2002/11/19 11:47:27 Yoshizawa1 Exp $
*/
#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"utl_dma.h"
#include	"../etc/ok_util.h"


/*----------------------------------------------------------------*/
#define	N_PRIMS		(16)
#define	N_POLYS		(16)
#define	N_VERTS		(N_POLYS * 4)
//上記固定

#define	N_PRIMS_GAS	(1)
#define	N_POLYS_GAS	(5)
#define	N_VERTS_GAS	(N_POLYS_GAS * 4)


#define	SCR_POS00	(SCRPAD_ADDR + 0x00)
#define	SCR_POS01	(SCRPAD_ADDR + 0x10)
#define	SCR_POS02	(SCRPAD_ADDR + 0x20)
#define	SCR_POS03	(SCRPAD_ADDR + 0x30)
#define	SCR_POS04	(SCRPAD_ADDR + 0x40)
#define	SCR_POS05	(SCRPAD_ADDR + 0x50)
#define	SCR_POS10	(SCRPAD_ADDR + 0x60)
#define	SCR_POS11	(SCRPAD_ADDR + 0x70)
#define	SCR_POS12	(SCRPAD_ADDR + 0x80)
#define	SCR_POS13	(SCRPAD_ADDR + 0x90)
#define	SCR_POS14	(SCRPAD_ADDR + 0xa0)
#define	SCR_POS15	(SCRPAD_ADDR + 0xb0)

#define	SCR_DATA	(SCRPAD_ADDR)

#define	ROTATE_RANGE	(0.05f)

typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;
	int			map ;

	DG_PRIM2	*prim;
	DG_PRIM2	*prim_gas;
	FVECTOR		source;
	FVECTOR		center;
	FVECTOR		pos0[4];
	FVECTOR		pos1[4];
	SVECTOR		rot;
	SVECTOR		rgba;
	float		height;
	float		width;
	float		height_diff;
	float		width_diff;

	float		size_0;
	float		size_1;
	float		length;

} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	clock;
	int	i,j;
	DG_PRIM2	*prim ;
	float		ftemp;
	float		size_0;
	float		size_1;
	FVECTOR	*pos0;
	FVECTOR	*pos1;
	FVECTOR	*pos2;
	FVECTOR	*pos3;
	FVECTOR	fvtemp;
	FVECTOR	cam;

	size_0 = work->size_0 / (float)N_POLYS;
	size_1 = work->size_1 / (float)N_POLYS;

	/* ライン状 */
	prim = work->prim;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	fvtemp.vx = sinf( (float)GV_Time * 0.011f )  *  ROTATE_RANGE;
	fvtemp.vy = sinf( (float)GV_Time * 0.015f )  *  ROTATE_RANGE;
	fvtemp.vz = 1.0f;
	fvtemp.vw = 0.0f;
	DG_SetPos( &DG_Chanls->eye );
	DG_RotVector( &fvtemp, &fvtemp, 1 );
	_sceVu0Normalize( &fvtemp, &fvtemp );

	DG_COPY_VEC( &cam, (FVECTOR *)DG_Chanls->eye.m[3] );

	OK_Mem_Scr( SCR_DATA, prim->pos[clock], sizeof(FVECTOR), N_PRIMS * N_VERTS);

	pos0 = SCR_POS00;
	pos1 = SCR_POS01;
	pos2 = SCR_POS02;
	pos3 = SCR_POS03;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( j = 0 ; j < N_POLYS ; j++ ){
			_sceVu0SubVector( pos3, &cam, pos0 ) ;
			_sceVu0Normalize( pos3, pos3 );
			ftemp = _sceVu0InnerProduct( pos3, &fvtemp );
			ftemp = ( ftemp > 0.0f )? ftemp: -ftemp;


			_sceVu0SubVector( pos3, pos2, pos0 ) ;
			_sceVu0OuterProduct( pos3, pos3, &fvtemp ) ;
			_sceVu0Normalize( pos3, pos3 );
			_sceVu0ScaleVector( pos1, pos3, size_0 );
			_sceVu0SubVector( pos1, pos0, pos1 ) ;
			_sceVu0ScaleVector( pos3, pos3, size_1 );
			_sceVu0SubVector( pos3, pos2, pos3 ) ;
			pos0 += 4;
			pos1 += 4;
			pos2 += 4;
			pos3 += 4;
		}
	}

	OK_Scr_Mem( prim->pos[clock], SCR_DATA, sizeof(FVECTOR), N_PRIMS * N_VERTS);



	//------------------------------------------------------------------------------
	/* スプライト：ガス状 */
	prim = work->prim_gas;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

/*
	fvtemp.vx = sinf( (float)GV_Time * 0.011f )  *  ROTATE_RANGE;
	fvtemp.vy = sinf( (float)GV_Time * 0.015f )  *  ROTATE_RANGE;
	fvtemp.vz = 1.0f;
	fvtemp.vw = 0.0f;
	DG_SetPos( &DG_Chanls->eye );
	DG_RotVector( &fvtemp, &fvtemp, 1 );
	_sceVu0Normalize( &fvtemp, &fvtemp );

	DG_COPY_VEC( &cam, (FVECTOR *)DG_Chanls->eye.m[3] );

	OK_Mem_Scr( SCR_DATA, prim->pos[clock], sizeof(FVECTOR), N_PRIMS_GAS * N_VERTS_GAS);

	pos0 = SCR_POS00;
	pos1 = SCR_POS01;
	pos2 = SCR_POS02;
	pos3 = SCR_POS03;
	for ( i = 0 ; i < N_PRIMS_GAS ; i++ ){
		for ( j = 0 ; j < N_VERTS_GAS ; j++ ){
		}
	}

	OK_Scr_Mem( prim->pos[clock], SCR_DATA, sizeof(FVECTOR), N_PRIMS_GAS * N_VERTS_GAS);
*/
}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR	*pos0 ;
	FVECTOR	*pos1 ;
	DG_PRIM2_UVRGB	*uvrgb0 ;
	DG_PRIM2_UVRGB	*uvrgb1 ;
	int		i, j ;
	u_char	col_r;
	u_char	col_g;
	u_char	col_b;
	u_char	col_a;
	float		rand_x;
	float		rand_y;
	FVECTOR	*fvtemp00 = SCR_POS00;
	FVECTOR	*fvtemp01 = SCR_POS01;
	FVECTOR	*fvtemp02 = SCR_POS02;
	FVECTOR	*fvtemp03 = SCR_POS03;
	FVECTOR	*fvtemp04 = SCR_POS04;
	FVECTOR	*fvtemp10 = SCR_POS10;
	FVECTOR	*fvtemp11 = SCR_POS11;
	FVECTOR	*fvtemp12 = SCR_POS12;
	FVECTOR	*fvtemp13 = SCR_POS13;
	FVECTOR	*fvtemp14 = SCR_POS14;

	col_r = work->rgba.vx;
	col_g = work->rgba.vy;
	col_b = work->rgba.vz;
	col_a = work->rgba.pad;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	GM_GroupPrim2( work->prim, work->map ) ;

	DG_COPY_VEC( fvtemp00, &work->pos0[0] );
	DG_COPY_VEC( fvtemp03, fvtemp00 );
	_sceVu0SubVector( fvtemp01, &work->pos0[1], fvtemp00 ) ;
//	_sceVu0ScaleVector( fvtemp01, fvtemp01, 1.0f/(float)N_POLYS );
	_sceVu0SubVector( fvtemp02, &work->pos0[2], fvtemp00 ) ;
//	_sceVu0ScaleVector( fvtemp02, fvtemp02, 1.0f/(float)N_PRIMS );

	DG_COPY_VEC( fvtemp10, &work->pos1[0] );
	DG_COPY_VEC( fvtemp13, fvtemp10 );
	_sceVu0SubVector( fvtemp11, &work->pos1[1], fvtemp10 ) ;
//	_sceVu0ScaleVector( fvtemp11, fvtemp11, 1.0f/(float)N_POLYS );
	_sceVu0SubVector( fvtemp12, &work->pos1[2], fvtemp10 ) ;
//	_sceVu0ScaleVector( fvtemp12, fvtemp12, 1.0f/(float)N_PRIMS );


	pos0   = prim->pos[ 0 ] ;;
	pos1   = prim->pos[ 1 ] ;;
	uvrgb0 = prim->uvrgb[ 0 ] ;
	uvrgb1 = prim->uvrgb[ 1 ] ;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( j = 0 ; j < N_POLYS ; j++ ){
			rand_x = rnd();
			rand_y = rnd();

			fvtemp04->vx = fvtemp03->vx + fvtemp02->vx*rand_x + fvtemp01->vx*rand_y;
			fvtemp04->vy = fvtemp03->vy + fvtemp02->vy*rand_x + fvtemp01->vy*rand_y;
			fvtemp04->vz = fvtemp03->vz + fvtemp02->vz*rand_x + fvtemp01->vz*rand_y;
			DG_COPY_VEC( pos0++, fvtemp04 );
			DG_COPY_VEC( pos1++, fvtemp04 );
			DG_COPY_VEC( pos0++, fvtemp04 );
			DG_COPY_VEC( pos1++, fvtemp04 );
			fvtemp14->vx = fvtemp13->vx + fvtemp12->vx*rand_x + fvtemp11->vx*rand_y;
			fvtemp14->vy = fvtemp13->vy + fvtemp12->vy*rand_x + fvtemp11->vy*rand_y;
			fvtemp14->vz = fvtemp13->vz + fvtemp12->vz*rand_x + fvtemp11->vz*rand_y;
			DG_COPY_VEC( pos0++, fvtemp14 );
			DG_COPY_VEC( pos1++, fvtemp14 );
			DG_COPY_VEC( pos0++, fvtemp14 );
			DG_COPY_VEC( pos1++, fvtemp14 );


			uvrgb1->u = uvrgb0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x8fff ;
			uvrgb1->r = uvrgb0->r = col_r ;
			uvrgb1->g = uvrgb0->g = col_g ;
			uvrgb1->b = uvrgb0->b = col_b ;
			uvrgb1->a = uvrgb0->a = col_a ;
			uvrgb0++;
			uvrgb1++;

			uvrgb1->u = uvrgb0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x8fff ;
			uvrgb1->r = uvrgb0->r = col_r ;
			uvrgb1->g = uvrgb0->g = col_g ;
			uvrgb1->b = uvrgb0->b = col_b ;
			uvrgb1->a = uvrgb0->a = 0 ;
			uvrgb0++;
			uvrgb1++;

			uvrgb1->u = uvrgb0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x0fff ;
			uvrgb1->r = uvrgb0->r = col_r ;
			uvrgb1->g = uvrgb0->g = col_g ;
			uvrgb1->b = uvrgb0->b = col_b ;
			uvrgb1->a = uvrgb0->a = col_a ;
			uvrgb0++;
			uvrgb1++;

			uvrgb1->u = uvrgb0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x0fff ;
			uvrgb1->r = uvrgb0->r = col_r ;
			uvrgb1->g = uvrgb0->g = col_g ;
			uvrgb1->b = uvrgb0->b = col_b ;
			uvrgb1->a = uvrgb0->a = 0 ;
			uvrgb0++;
			uvrgb1++;
		}
	}

	return 1;
}

/*----------------------------------------------------------------*/
static int InitPacket2_Gas( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR	*pos0;
	FVECTOR	*pos1;
	DG_PRIM2_UVRGB	*uvrgb0;
	DG_PRIM2_UVRGB	*uvrgb1;
	int		i, j ;
	u_char	col_r;
	u_char	col_g;
	u_char	col_b;
	u_char	col_a;


	col_r = work->rgba.vx;
	col_g = work->rgba.vy;
	col_b = work->rgba.vz;
	col_a = work->rgba.pad;



	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos0 = prim->pos[0];
	pos1 = prim->pos[1];
	uvrgb0 = prim->uvrgb[ 0 ] ;
	uvrgb1 = prim->uvrgb[ 1 ] ;

	DG_COPY_VEC( pos0++, &work->pos0[0] );
	DG_COPY_VEC( pos0++, &work->pos0[1] );
	DG_COPY_VEC( pos0++, &work->pos1[0] );
	DG_COPY_VEC( pos0++, &work->pos1[1] );

	DG_COPY_VEC( pos0++, &work->pos0[1] );
	DG_COPY_VEC( pos0++, &work->pos0[3] );
	DG_COPY_VEC( pos0++, &work->pos1[1] );
	DG_COPY_VEC( pos0++, &work->pos1[3] );

	DG_COPY_VEC( pos0++, &work->pos0[3] );
	DG_COPY_VEC( pos0++, &work->pos0[2] );
	DG_COPY_VEC( pos0++, &work->pos1[3] );
	DG_COPY_VEC( pos0++, &work->pos1[2] );

	DG_COPY_VEC( pos0++, &work->pos0[2] );
	DG_COPY_VEC( pos0++, &work->pos0[0] );
	DG_COPY_VEC( pos0++, &work->pos1[2] );
	DG_COPY_VEC( pos0++, &work->pos1[0] );

	DG_COPY_VEC( pos0++, &work->pos0[0] );
	DG_COPY_VEC( pos0++, &work->pos0[1] );
	DG_COPY_VEC( pos0++, &work->pos0[2] );
	DG_COPY_VEC( pos0++, &work->pos0[3] );


	DG_COPY_VEC( pos1++, &work->pos0[0] );
	DG_COPY_VEC( pos1++, &work->pos0[1] );
	DG_COPY_VEC( pos1++, &work->pos1[0] );
	DG_COPY_VEC( pos1++, &work->pos1[1] );

	DG_COPY_VEC( pos1++, &work->pos0[1] );
	DG_COPY_VEC( pos1++, &work->pos0[3] );
	DG_COPY_VEC( pos1++, &work->pos1[1] );
	DG_COPY_VEC( pos1++, &work->pos1[3] );

	DG_COPY_VEC( pos1++, &work->pos0[3] );
	DG_COPY_VEC( pos1++, &work->pos0[2] );
	DG_COPY_VEC( pos1++, &work->pos1[3] );
	DG_COPY_VEC( pos1++, &work->pos1[2] );

	DG_COPY_VEC( pos1++, &work->pos0[2] );
	DG_COPY_VEC( pos1++, &work->pos0[0] );
	DG_COPY_VEC( pos1++, &work->pos1[2] );
	DG_COPY_VEC( pos1++, &work->pos1[0] );

	DG_COPY_VEC( pos1++, &work->pos0[0] );
	DG_COPY_VEC( pos1++, &work->pos0[1] );
	DG_COPY_VEC( pos1++, &work->pos0[2] );
	DG_COPY_VEC( pos1++, &work->pos0[3] );


	for ( i = 0 ; i < N_PRIMS_GAS ; i++ ){
		for ( j = 0 ; j < N_POLYS_GAS ; j++ ){
			uvrgb1->u = uvrgb0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x8fff ;
			uvrgb1->r = uvrgb0->r = col_r ;
			uvrgb1->g = uvrgb0->g = col_g ;
			uvrgb1->b = uvrgb0->b = col_b ;
			uvrgb1->a = uvrgb0->a = col_a ;
			uvrgb0++;
			uvrgb1++;

			uvrgb1->u = uvrgb0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x8fff ;
			uvrgb1->r = uvrgb0->r = col_r ;
			uvrgb1->g = uvrgb0->g = col_g ;
			uvrgb1->b = uvrgb0->b = col_b ;
			uvrgb1->a = uvrgb0->a = col_a ;
			uvrgb0++;
			uvrgb1++;

			uvrgb1->u = uvrgb0->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x0fff ;
			uvrgb1->r = uvrgb0->r = col_r ;
			uvrgb1->g = uvrgb0->g = col_g ;
			uvrgb1->b = uvrgb0->b = col_b ;
			if( j!=0 ){
				uvrgb1->a = uvrgb0->a = 0 ;
			}else{
				uvrgb1->a = uvrgb0->a = col_a ;
			}
			uvrgb0++;
			uvrgb1++;

			uvrgb1->u = uvrgb0->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb1->v = uvrgb0->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb1->q = uvrgb0->q = 4096 ;
			uvrgb1->f = uvrgb0->f = 0x0fff ;
			uvrgb1->r = uvrgb0->r = col_r ;
			uvrgb1->g = uvrgb0->g = col_g ;
			uvrgb1->b = uvrgb0->b = col_b ;
			if( j!=0 ){
				uvrgb1->a = uvrgb0->a = 0 ;
			}else{
				uvrgb1->a = uvrgb0->a = col_a ;
			}
			uvrgb0++;
			uvrgb1++;
		}
	}

	return 1;
}

/* 初期設定値を取得 */
static	int	GetOptionValue( Work *work )
{
		if ( GCL_GetOption( 'p' ) != NULL ) {
			work->center.vx = (float)GCL_GetNextInt() ;
			work->center.vy = (float)GCL_GetNextInt() ;
			work->center.vz = (float)GCL_GetNextInt() ;
			work->center.vw = 1.0f ;
		}else{
			return -1;
		}

		if ( GCL_GetOption( 't' ) != NULL ) {
			work->source.vx = (float)GCL_GetNextInt() ;
			work->source.vy = (float)GCL_GetNextInt() ;
			work->source.vz = (float)GCL_GetNextInt() ;
			work->source.vw = 1.0f ;
		}else{
			return -1;
		}

		if ( GCL_GetOption( 'r' ) != NULL ) {
			work->rot.vx = (short)GCL_GetNextInt() ;
			work->rot.vy = (short)GCL_GetNextInt() ;
			work->rot.vz = 0 ;
		}else{
			return -1;
		}

		if ( GCL_GetOption( 's' ) != NULL ) {
			work->height = (float)GCL_GetNextInt() ;
			work->width  = (float)GCL_GetNextInt() ;
		}else{
			return -1;
		}

		if ( GCL_GetOption( 'l' ) != NULL ) {
			work->length  = (float)GCL_GetNextInt() ;
		}else{
			return -1;
		}

		if ( GCL_GetOption( 'c' ) != NULL ) {
			work->rgba.vx  = (u_char)GCL_GetNextInt() ;
			work->rgba.vy  = (u_char)GCL_GetNextInt() ;
			work->rgba.vz  = (u_char)GCL_GetNextInt() ;
			work->rgba.pad = (u_char)GCL_GetNextInt() ;
		}else{
			return -1;
		}

	return 0;
}

static int GetResources( Work *work )
{
	int	i;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	FVECTOR		fvtemp;

	if( GetOptionValue( work ) < 0 ) return -1;

	work->pos0[0].vx = -work->width;
	work->pos0[0].vy = -work->height;
	work->pos0[0].vz = 0.0f;
	work->pos0[1].vx =  work->width;
	work->pos0[1].vy = -work->height;
	work->pos0[1].vz = 0.0f;
	work->pos0[2].vx = -work->width;
	work->pos0[2].vy =  work->height;
	work->pos0[2].vz = 0.0f;
	work->pos0[3].vx =  work->width;
	work->pos0[3].vy =  work->height;
	work->pos0[3].vz = 0.0f;

	DG_SetPos2( &work->center, &work->rot );
	DG_PutVector( work->pos0, work->pos0, 4 );

	for( i=0; i<4; i++ ){
		work->pos1[i].vx = work->pos0[i].vx - work->source.vx;
		work->pos1[i].vy = work->pos0[i].vy - work->source.vy;
		work->pos1[i].vz = work->pos0[i].vz - work->source.vz;
		work->pos1[i].vw = 0.0f;
		_sceVu0Normalize( &work->pos1[i], &work->pos1[i] );
		_sceVu0ScaleVector( &work->pos1[i], &work->pos1[i], work->length );
		_sceVu0AddVector( &work->pos1[i], &work->pos1[i], &work->pos0[i] );
	}

	work->size_0 = work->width + work->height;
	_sceVu0SubVector( &fvtemp, &work->pos1[0], &work->pos1[1] );
	work->width_diff = GV_VecLen3F( &fvtemp );
	_sceVu0SubVector( &fvtemp, &work->pos1[0], &work->pos1[2] );
	work->height_diff = GV_VecLen3F( &fvtemp );
	work->size_1 = work->width_diff + work->height_diff;


	tex = DG_GetTexture( 8617636 /*"col128_alp"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

//	tex = DG_GetTexture( 12244206 /*"fog02_msk"*/ );
	tex = DG_GetTexture( 8617636 /*"col128_alp"*/ );
	prim = work->prim_gas = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS_GAS, N_VERTS_GAS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2_Gas( work, prim, tex );




	return 0 ;
}

void *NewWindowLight( int name, int map )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		work->map  = map;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
