//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	slit_light.c
	採光

	2001/03/09 S.Okajima
	$Id: slit_light.c,v 1.1.1.3 2002/11/19 11:47:24 Yoshizawa1 Exp $
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

#include	"camera.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"
#include	"./bubble.h"

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;
/*----------------------------------------------------------------*/

#define	RAISE			(0)
#define	RAISE_DUST		(-30000)

//スクラッチパッドの一部をランダムテーブルに使用
#define	SCR_POS		(SCRPAD_ADDR)

#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/4)
#define	N_PRIMS		(10)
#define	COL_R		(14)
#define	COL_G		(14)
#define	COL_B		(16)
#define	MAX_ALPHA	(128)
#define	SUB_ALPHA	(MAX_ALPHA/N_LOOPS)
#define	WIDTH		(1750.0f)
#define	HEIGHT		( 750.0f)
#define	WIDTH_HALF	(WIDTH*0.5f)
#define	HEIGHT_HALF	(HEIGHT*0.5f)
#define	SLIT_NUM	(10)
#define	ZIGZAG_PARAM	(0.1f)
#define	SHIFT_RATIO_ADD	(0.05f)

#define	N_POLYS_SURFACE		(2)
#define	MAX_ALPHA_SURFACE	(255.0f)

#define	N_VERTS_DUST	(64)
#define	N_POLYS_DUST	(N_VERTS_DUST/2)
#define	N_PRIMS_DUST	(1)
#define	COL_R_DUST		(4)
#define	COL_G_DUST		(4)
#define	COL_B_DUST		(4)
#define	MAX_ALPHA_DUST	(255)


typedef	struct	{
	GV_ACT_EX		actor ;

	FVECTOR		center;
	SVECTOR		rot;

	FVECTOR		direction;
	float		length;

	DG_PRIM2	*prim ;
	DG_PRIM2	*prim_dust ;
	DG_PRIM2	*prim_surface;
	FVECTOR		dust_pos[N_PRIMS_DUST*N_POLYS_DUST];
	FVECTOR		dust_vec[N_PRIMS_DUST*N_POLYS_DUST];
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2_UVRGB	*uvrgb;
	DG_PRIM2		*prim ;
	FVECTOR			*pos;
	FVECTOR			*d_pos;
	FVECTOR			*d_vec;
	FVECTOR			*next;
	int		i, j ;
	int		clock;
	int		alpha;
	float	intense;
	FVECTOR			fvtemp;
	FVECTOR			keep;

	_sceVu0SubVector( &fvtemp, &work->center, (FVECTOR *)DG_Chanls->eye.m[3] ) ;
	_sceVu0Normalize( &fvtemp, &fvtemp );
	_sceVu0Normalize( &keep, &work->direction );
	intense = DG_FABS( _sceVu0InnerProduct( &keep, &fvtemp ) );
	alpha = (int)((float)(MAX_ALPHA/2)*intense*intense)+MAX_ALPHA/2;


	prim = work->prim;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;


	OK_Mem_Scr( SCR_POS, prim->pos[1-clock], sizeof(FVECTOR), N_PRIMS*N_VERTS);

	pos   = SCR_POS;
	next  = SCR_POS;
	next += 4;
	_sceVu0ScaleVector( &fvtemp, &work->direction, pos->vw );
	uvrgb = prim->uvrgb[ clock ];
	for( i=0; i<N_PRIMS; i++ ){
		for( j=0; j<N_POLYS-1; j++ ){
			DG_COPY_VEC( &keep, pos );
			if( ((irnd()>>8)&255)==0 ){
				pos->vw = 1.0f + frnd()*ZIGZAG_PARAM;
//				printf("%f %f\n",pos->vw,next->vw);
			}
			next->vw = next->vw*0.99f + pos->vw*0.01f;
			pos++;
			DG_COPY_VEC( pos, next );
			pos++;
			_sceVu0ScaleVector( &fvtemp, &work->direction, keep.vw );
			_sceVu0AddVector( pos, &keep, &fvtemp) ;
			pos++;
			_sceVu0ScaleVector( &fvtemp, &work->direction, next->vw );
			_sceVu0AddVector( pos, next, &fvtemp) ;
			pos++;
			next+= 4;

			(uvrgb++)->a = alpha;
			(uvrgb++)->a = alpha;
			(uvrgb++)->a = 0;
			(uvrgb++)->a = 0;
		}
		DG_COPY_VEC( &keep, pos );
		pos++;
		DG_COPY_VEC( pos, &keep );
		pos++;
		_sceVu0ScaleVector( &fvtemp, &work->direction, keep.vw );
		_sceVu0AddVector( pos, &keep, &fvtemp) ;
		DG_COPY_VEC( &keep, pos );
		pos++;
		DG_COPY_VEC( pos, &keep );
		pos++;
		next+= 4;

		(uvrgb++)->a = alpha;
		(uvrgb++)->a = alpha;
		(uvrgb++)->a = 0;
		(uvrgb++)->a = 0;
	}

	OK_Scr_Mem( prim->pos[  clock], SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS);


	//----


	alpha = (int)(MAX_ALPHA_SURFACE*intense);
	prim = work->prim_surface;
	uvrgb = prim->uvrgb[ clock ];
	(uvrgb++)->a = alpha;
	(uvrgb++)->a = alpha;
	(uvrgb++)->a = alpha;
	(uvrgb++)->a = alpha;
	(uvrgb++)->a = alpha;
	(uvrgb++)->a = alpha;
	(uvrgb++)->a = alpha;
	(uvrgb++)->a = alpha;


	//----


	prim = work->prim_dust;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	pos   = SCR_POS;
	d_pos = work->dust_pos;
	d_vec = work->dust_vec;
	for( i=0; i<N_PRIMS_DUST; i++ ){
		for( j=0; j<N_POLYS_DUST; j++ ){
			_sceVu0AddVector( d_pos, d_pos, d_vec ) ;
			if( d_pos->vx > WIDTH_HALF ){
				d_pos->vx-= WIDTH;
			}else if( d_pos->vx <-WIDTH_HALF ){
				d_pos->vx+= WIDTH;
			}
			if( d_pos->vy > HEIGHT_HALF ){
				d_pos->vy-= HEIGHT;
			}else if( d_pos->vy <-HEIGHT_HALF ){
				d_pos->vy+= HEIGHT;
			}
			if( d_pos->vw > 1.0f ){
				d_pos->vw-= 1.0f;
			}else if( d_pos->vw < 0.0f ){
				d_pos->vw+= 1.0f;
			}

			DG_COPY_VEC( pos++, d_pos );
			DG_COPY_VEC( pos++, d_pos );

			d_pos++;
			d_vec++;
		}
	}

	pos   = SCR_POS;
	DG_SetPos2( &work->center, &work->rot );
	DG_PutVector( pos, pos, N_PRIMS_DUST*N_VERTS_DUST );

	uvrgb = prim->uvrgb[ clock ];
	pos   = SCR_POS;
	d_pos = work->dust_pos;
	for( i=0; i<N_PRIMS_DUST; i++ ){
		for( j=0; j<N_POLYS_DUST; j++ ){
			uvrgb->a = (int)((float)(MAX_ALPHA)*(1.0f-d_pos->vw));
			_sceVu0ScaleVector( &fvtemp, &work->direction, d_pos->vw );
			_sceVu0AddVector( pos, pos, &fvtemp ) ;
			pos++;
			_sceVu0AddVector( pos, pos, &work->direction ) ;
			pos++;

			uvrgb+=2;
			d_pos++;
		}
	}

	OK_Scr_Mem( prim->pos[  clock], SCR_POS, sizeof(FVECTOR), N_PRIMS_DUST*N_VERTS_DUST);

}


/*----------------------------------------------------------------*/
static void Die( Work *work )
{
	work->prim         = OK_FreePrim2( work->prim );
	work->prim_surface = OK_FreePrim2( work->prim_surface );
	work->prim_dust    = OK_FreePrim2( work->prim_dust );
}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR			fvtemp;
	FVECTOR			*pos;
	DG_PRIM2_UVRGB	*uvrgb;
	int		i, j ;
	float			width;
	float			width_add;
	float			height;
	float			ftemp;


	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	prim->raise = RAISE;

	uvrgb = SCR_POS;
	for ( j = 0 ; j < N_PRIMS*N_POLYS ; j++ ){
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = COL_R ;
		uvrgb->g = COL_G ;
		uvrgb->b = COL_B ;
		uvrgb->a = MAX_ALPHA;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = COL_R ;
		uvrgb->g = COL_G ;
		uvrgb->b = COL_B ;
		uvrgb->a = MAX_ALPHA;
		uvrgb++;

		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = COL_R ;
		uvrgb->g = COL_G ;
		uvrgb->b = COL_B ;
		uvrgb->a = 0;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = COL_R ;
		uvrgb->g = COL_G ;
		uvrgb->b = COL_B ;
		uvrgb->a = 0;
		uvrgb++;
	}
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_POS, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_POS, sizeof(DG_PRIM2_UVRGB), N_PRIMS*N_VERTS);


	pos = SCR_POS;
	height     =-HEIGHT_HALF;
	for( i=0; i<N_PRIMS; i++ ){
		width      =-WIDTH_HALF;
		width_add  = width + WIDTH/(N_POLYS-1);
		for( j=0; j<N_POLYS; j++ ){
			pos->vx = width;
			pos->vy = height;
			pos->vz = 0.0f;
			pos++;
			pos->vx = width_add;
			pos->vy = height;
			pos->vz = 0.0f;
			pos++;
			pos->vx = width;
			pos->vy = height;
			pos->vz = 0.0f;
			pos++;
			pos->vx = width_add;
			pos->vy = height;
			pos->vz = 0.0f;
			pos++;

			width     = width_add;
			width_add+= WIDTH/(N_POLYS-1);
		}
		height+= HEIGHT/N_PRIMS;
	}

	pos   = SCR_POS;
	DG_SetPos2( &work->center, &work->rot );
	DG_PutVector( pos, pos, N_PRIMS*N_VERTS );

	ftemp = 1.0f + frnd()*ZIGZAG_PARAM;
	_sceVu0ScaleVector( &fvtemp, &work->direction, ftemp );
	for( i=0; i<N_PRIMS; i++ ){
		for( j=0; j<N_POLYS; j++ ){
			pos->vw = ftemp;
			pos++;
			pos->vw = ftemp = 1.0f + frnd()*ZIGZAG_PARAM;
			pos++;
			_sceVu0AddVector( pos, pos, &fvtemp) ;
			pos++;
			_sceVu0ScaleVector( &fvtemp, &work->direction, ftemp );
			_sceVu0AddVector( pos, pos, &fvtemp) ;
			pos++;
		}
	}


	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS*N_VERTS);


}

/*----------------------------------------------------------------*/
static void InitPacketSurface( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR			*pos;
	DG_PRIM2_UVRGB	*uvrgb;
	int		i ;


	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	prim->raise = RAISE;

	uvrgb = SCR_POS;
	for( i=0; i<N_POLYS_SURFACE; i++ ){
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = COL_R ;
		uvrgb->g = COL_G ;
		uvrgb->b = COL_B ;
		uvrgb->a = MAX_ALPHA;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = COL_R ;
		uvrgb->g = COL_G ;
		uvrgb->b = COL_B ;
		uvrgb->a = MAX_ALPHA;
		uvrgb++;

		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = COL_R ;
		uvrgb->g = COL_G ;
		uvrgb->b = COL_B ;
		uvrgb->a = MAX_ALPHA;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = COL_R ;
		uvrgb->g = COL_G ;
		uvrgb->b = COL_B ;
		uvrgb->a = MAX_ALPHA;
		uvrgb++;
	}

	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_POS, sizeof(DG_PRIM2_UVRGB), N_POLYS_SURFACE*4 );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_POS, sizeof(DG_PRIM2_UVRGB), N_POLYS_SURFACE*4 );

	pos = SCR_POS;
	for( i=0; i<N_POLYS_SURFACE; i++ ){
		pos->vx = WIDTH_HALF;
		pos->vy = HEIGHT_HALF;
		pos->vz = 0.0f;
		pos++;
		pos->vx =-WIDTH_HALF;
		pos->vy = HEIGHT_HALF;
		pos->vz = 0.0f;
		pos++;
		pos->vx = WIDTH_HALF;
		pos->vy =-HEIGHT_HALF;
		pos->vz = 0.0f;
		pos++;
		pos->vx =-WIDTH_HALF;
		pos->vy =-HEIGHT_HALF;
		pos->vz = 0.0f;
		pos++;
	}

	pos   = SCR_POS;
	DG_SetPos2( &work->center, &work->rot );
	DG_PutVector( pos, pos, N_POLYS_SURFACE*4 );

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR), N_POLYS_SURFACE*4);
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR), N_POLYS_SURFACE*4);
}

/*----------------------------------------------------------------*/
static void InitPacketDust( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR			*pos;
	FVECTOR			*vec;
	DG_PRIM2_UVRGB	*uvrgb;
	int		i, j ;



	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	prim->raise = RAISE_DUST;

	uvrgb = SCR_POS;
	for ( j = 0 ; j < N_PRIMS_DUST*N_POLYS_DUST ; j++ ){
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x8fff ;
		uvrgb->r = COL_R_DUST ;
		uvrgb->g = COL_G_DUST ;
		uvrgb->b = COL_B_DUST ;
		uvrgb->a = MAX_ALPHA_DUST;
		uvrgb++;

		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096 ;
		uvrgb->f = 0x0fff ;
		uvrgb->r = COL_R_DUST ;
		uvrgb->g = COL_G_DUST ;
		uvrgb->b = COL_B_DUST ;
		uvrgb->a = 0;
		uvrgb++;
	}
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_POS, sizeof(DG_PRIM2_UVRGB), N_PRIMS_DUST*N_VERTS_DUST);
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_POS, sizeof(DG_PRIM2_UVRGB), N_PRIMS_DUST*N_VERTS_DUST);

	pos = SCR_POS;
	for( i=0; i<N_PRIMS_DUST*N_POLYS_DUST; i++ ){
		DG_COPY_VEC( pos++, &DG_ZeroVector );
		DG_COPY_VEC( pos++, &DG_ZeroVector );
	}
	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS_DUST*N_VERTS_DUST);
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS_DUST*N_VERTS_DUST);

	pos = work->dust_pos;
	vec = work->dust_vec;
	for( i=0; i<N_PRIMS_DUST*N_POLYS_DUST; i++ ){
		pos->vx = WIDTH_HALF  * frnd();
		pos->vy = HEIGHT_HALF * frnd();
		pos->vz = 0.0f;
		pos->vw = rnd();

		vec->vx = WIDTH_HALF  * 0.001f * frnd();
		vec->vy = HEIGHT_HALF * 0.001f * frnd();
		vec->vz = 0.0f;
		vec->vw = frnd()*0.5f*0.001f;

		pos++;
		vec++;
	}

}




static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	FVECTOR		norm;


	/* 設置中心座標 */
	if ( GCL_GetOption( 'p' ) != NULL ) {
		work->center.vx = (float)GCL_GetNextInt() ;
		work->center.vy = (float)GCL_GetNextInt() ;
		work->center.vz = (float)GCL_GetNextInt() ;
		work->center.vw = 1.0f ;
	}

	/* 法線 */
	if ( GCL_GetOption( 'n' ) != NULL ) {
		norm.vx = (float)GCL_GetNextInt() ;
		norm.vy = (float)GCL_GetNextInt() ;
		norm.vz = (float)GCL_GetNextInt() ;
		norm.vw = 1.0f ;
	}

	OK_DirVecXY( &DG_ZeroVector, &norm, &work->rot );


	/* 光線の絶対方向 */
	if ( GCL_GetOption( 'd' ) != NULL ) {
		work->direction.vx = (float)GCL_GetNextInt() ;
		work->direction.vy = (float)GCL_GetNextInt() ;
		work->direction.vz = (float)GCL_GetNextInt() ;
		work->direction.vw = 1.0f ;
	}

	/* 光線の到達距離 */
	if ( GCL_GetOption( 'l' ) != NULL ) {
		work->length = (float)GCL_GetNextInt() ;
	}

	_sceVu0Normalize( &work->direction, &work->direction );
	_sceVu0ScaleVector( &work->direction, &work->direction, work->length );

	tex = DG_GetTexture( 7338993 /*"col256_bld100"*/ );
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

	prim = work->prim_surface = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, N_POLYS_SURFACE*4 );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacketSurface( work, prim, tex );



	prim = work->prim_dust = GM_MakePrim2( DG_PRIM2_LINE|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ANTIALIASING|DG_PRIM2_ALPHA, N_PRIMS_DUST, N_VERTS_DUST );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacketDust( work, prim, tex );


	return 0 ;
}

void *NewSlitLight( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}

	}
	return (void *)work ;
}
