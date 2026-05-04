//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	set_tex.c
	ワールドにテクスチャセット
	1999/11/02 S.Okajima
	$Id: set_tex.c,v 1.1.1.3 2002/11/19 11:47:13 Yoshizawa1 Exp $

*/

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

extern int ok_flush_status;

/* 広範囲に出現する可能性があるので１プリムに１枚 */
#define	N_VERTS		(4)
#define	N_POLYS		(N_VERTS/4)
#define	MAX_PRIM		(32)

/*----------------------------------------------------------------*/
typedef	struct	{
	DG_PRIM2	*prim ;
} Unit ;


typedef	struct	{
	GV_ACT_EX		actor ;
	int			p_num;
	Unit		unit[0] ;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	i;

	if( ok_flush_status!=0 ){
		for( i=0; i<work->p_num; i++ ){
			DG_SwitchBuffPrim2( work->unit[i].prim );
			DG_InvisiblePrim2( work->unit[i].prim );
		}
		return;
	}else{
		for( i=0; i<work->p_num; i++ ){
			DG_SwitchBuffPrim2( work->unit[i].prim );
			DG_VisiblePrim2( work->unit[i].prim );
		}
	}
}

static void Die( Work *work )
{
	int	i;
	for( i=0; i<work->p_num; i++ ){
		work->unit[i].prim = OK_FreePrim2( work->unit[i].prim );
	}
}

/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int *col, FVECTOR *fv )
{
	DG_PRIM2_UVRGB	*uvrgb0 ;
	DG_PRIM2_UVRGB	*uvrgb1 ;
	FVECTOR			*pos0;
	FVECTOR			*pos1;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos0 = prim->pos[ 0 ] ;
	pos1 = prim->pos[ 1 ] ;

	pos0->vx = pos1->vx = fv[0].vx;
	pos0->vy = pos1->vy = fv[1].vy;
	pos0->vz = pos1->vz = fv[0].vz;
	pos0++;
	pos1++;

	pos0->vx = pos1->vx = fv[1].vx;
	pos0->vy = pos1->vy = fv[1].vy;
	pos0->vz = pos1->vz = fv[1].vz;
	pos0++;
	pos1++;

	pos0->vx = pos1->vx = fv[0].vx;
	pos0->vy = pos1->vy = fv[0].vy;
	pos0->vz = pos1->vz = fv[0].vz;
	pos0++;
	pos1++;

	pos0->vx = pos1->vx = fv[1].vx;
	pos0->vy = pos1->vy = fv[0].vy;
	pos0->vz = pos1->vz = fv[1].vz;
//	pos0++;
//	pos1++;


	uvrgb0 = prim->uvrgb[ 0 ] ;
	uvrgb1 = prim->uvrgb[ 1 ] ;
	uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb0->q = uvrgb1->q = 4096 ;
	uvrgb0->f = uvrgb1->f = 0x8fff ;
	uvrgb0->r = uvrgb1->r = col[0] ;
	uvrgb0->g = uvrgb1->g = col[1] ;
	uvrgb0->b = uvrgb1->b = col[2] ;
	uvrgb0->a = uvrgb1->a = col[3] ;
	uvrgb0++;
	uvrgb1++;

	uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb0->q = uvrgb1->q = 4096 ;
	uvrgb0->f = uvrgb1->f = 0x8fff ;
	uvrgb0->r = uvrgb1->r = col[0] ;
	uvrgb0->g = uvrgb1->g = col[1] ;
	uvrgb0->b = uvrgb1->b = col[2] ;
	uvrgb0->a = uvrgb1->a = col[3] ;
	uvrgb0++;
	uvrgb1++;

	uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb0->q = uvrgb1->q = 4096 ;
	uvrgb0->f = uvrgb1->f = 0x0fff ;
	uvrgb0->r = uvrgb1->r = col[0] ;
	uvrgb0->g = uvrgb1->g = col[1] ;
	uvrgb0->b = uvrgb1->b = col[2] ;
	uvrgb0->a = uvrgb1->a = col[3] ;
	uvrgb0++;
	uvrgb1++;

	uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb0->q = uvrgb1->q = 4096 ;
	uvrgb0->f = uvrgb1->f = 0x0fff ;
	uvrgb0->r = uvrgb1->r = col[0] ;
	uvrgb0->g = uvrgb1->g = col[1] ;
	uvrgb0->b = uvrgb1->b = col[2] ;
	uvrgb0->a = uvrgb1->a = col[3] ;

	return 1;
}

static int GetResources( Work *work )
{
	int	i;
	int	tex_name[MAX_PRIM];
	int	col[MAX_PRIM][4];
	float		ftemp1[MAX_PRIM];
	float		ftemp2[MAX_PRIM];
	short		rot_y[MAX_PRIM];
	FVECTOR		center;
	FVECTOR		*pos;
	FVECTOR		fvtemp[2];
	SVECTOR		rot;
	DG_PRIM2	*prim=NULL ;
	DG_TEX		*tex[MAX_PRIM] ;

	work->p_num=MAX_PRIM;
	i=0;
	if ( GCL_GetOption( 't' ) != NULL ) {
		for(i=0; i<MAX_PRIM; i++){
			if( GCL_NextStr() != NULL ){
				tex_name[i]       = GCL_GetNextInt();
				tex[i] = DG_GetTexture( tex_name[i] );
			}else{
				break;
			}
		}
	}
	work->p_num=i;

	if ( GCL_GetOption( 'r' ) != NULL ) {
		for(i=0; i<work->p_num; i++){
			if( GCL_NextStr() != NULL ){
				rot_y[i] = (float)GCL_GetNextInt() ;
			}
		}
	}

	if ( GCL_GetOption( 's' ) != NULL ) {
		for(i=0; i<work->p_num; i++){
			if( GCL_NextStr() != NULL ){
				ftemp1[i] = (float)GCL_GetNextInt() ;
				ftemp2[i] = (float)GCL_GetNextInt() ;
			}
		}
	}

	if ( GCL_GetOption( 'c' ) != NULL ) {
		for(i=0; i<work->p_num; i++){
			if( GCL_NextStr() != NULL ){
				col[i][0]         = GCL_GetNextInt() & 255;
				col[i][1]         = GCL_GetNextInt() & 255;
				col[i][2]         = GCL_GetNextInt() & 255;
				col[i][3]         = GCL_GetNextInt() & 255;
			}
		}
	}



	if ( GCL_GetOption( 'p' ) != NULL ) {
		pos = SCRPAD_ADDR;
		for(i=0; i<work->p_num; i++){
			if( GCL_NextStr() != NULL ){
				center.vx   = (float)GCL_GetNextInt() ;
				center.vy   = (float)GCL_GetNextInt() ;
				center.vz   = (float)GCL_GetNextInt() ;

				rot.vx=0;
				rot.vy=rot_y[i];
				rot.vz=0;

				fvtemp[0].vx = -ftemp1[i];
				fvtemp[0].vy = -ftemp2[i];
				fvtemp[0].vz = 0.0f;
				fvtemp[1].vx =  ftemp1[i];
				fvtemp[1].vy =  ftemp2[i];
				fvtemp[1].vz = 0.0f;

				DG_SetPos2( &center, &rot );
				DG_PutVector( fvtemp, fvtemp, 2 );

//				prim = work->unit[i].prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, N_VERTS );
				prim = work->unit[i].prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, N_VERTS );
				if(prim==NULL){
					printf("null prim\n");
					return -1;
				}
				InitPacket2( work, prim, tex[i], &col[i][0], fvtemp );
				DG_VisiblePrim2( prim ) ;
			}
		}
	}



	return 0 ;
}

void *NewPutWorldTex( void )
{
	Work		*work ;
	int			i;

	OPERATOR() ;

	i=0;
	if ( GCL_GetOption( 't' ) != NULL ) {
		for(i=0; i<MAX_PRIM; i++){
			if( GCL_NextStr() == NULL ) break;
		}
	}

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) + sizeof( Unit )*i ) ;
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
