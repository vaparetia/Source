//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  cul_test2.c
  カリング付きポリゴン表示テスト

  2001/03/13 Yuuta Kunibe
  $Id: cul_test.c,v 1.1.1.3 2002/11/19 11:44:54 Yoshizawa1 Exp $
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
#include	"../../okajima/etc/ok_util.h"


#define	N_VERTS		(4)
#define	N_PRIMS		(1)

#define BASE_RGB	(128)
#define ALPHA		(64)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)

typedef	struct	{

    GV_ACT_EX	actor ;
    int 	name;
    int		map ;

    DG_PRIM2	*prim ;
    FVECTOR	center;
    int life;

} Work ;


static void Act( Work *work )
{
	int		i,j;
	int	  	clock;
	FVECTOR 	*pos;
	DG_PRIM2_UVRGB	*uvrgb;
	DG_PRIM2_UVRGB	*uvrgb_pre;

	work->center = GM_PlayerPosition;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }

	clock = work->prim->buffer_clock;

	pos = work->prim->pos[clock];
	uvrgb = work->prim->uvrgb[clock];
	uvrgb_pre = work->prim->uvrgb[1-clock];

	for( i=0; i<N_PRIMS; i++ ){
	    
	    pos->vx  = work->center.vx + 2000.f;
	    pos->vy  = work->center.vy + 2000.f;
	    pos->vz  = work->center.vz;
	    pos->vw  = 1.0f;
	    pos++;

	    pos->vx  = work->center.vx + 2000.f;
	    pos->vy  = work->center.vy - 2000.f;
	    pos->vz  = work->center.vz;
	    pos->vw  = 1.0f;
	    pos++;

	    pos->vx  = work->center.vx - 2000.f;
	    pos->vy  = work->center.vy + 2000.f;
	    pos->vz  = work->center.vz;
	    pos->vw  = 1.0f;
	    pos++;

	    pos->vx  = work->center.vx - 2000.f;
	    pos->vy  = work->center.vy - 2000.f;
	    pos->vz  = work->center.vz;
	    pos->vw  = 1.0f;
	    pos++;

	    if ( work->life < 30 ) {
		for ( j = 0 ; j < N_VERTS; j++ ) {
		    uvrgb->a = uvrgb_pre->a * 0.95f;
		    uvrgb++;
		    uvrgb_pre++;
		}
	    }

	}
    
	if( --work->life < 0 ){
	    GV_DestroyActor( work );
	}       

}


static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}


/*
  DG_PRIM2_POLYは４頂点指定する
  スプライトと異なりDG_PRIM2_UVRGBを使用
  (スプライトは幅,高さを指定するのでDG_PRIM2_UVRGBWHを使用)
*/
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR		*pos;
	DG_PRIM2_UVRGB	*uvrgb;
	int		i;

	DG_ConfigPrim2Tex( prim, tex );		/* テクスチャ指定 */
	//DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos   = SCR_POS;
	uvrgb = SCR_UVS;

	for ( i = 0 ; i < N_PRIMS ; i++ ){

			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
			DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;

			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096;
			//uvrgb->f = 0x0020;
			uvrgb->f = 0x0000;
			uvrgb->r = BASE_RGB;
			uvrgb->g = BASE_RGB;
			uvrgb->b = BASE_RGB;
			uvrgb->a = ALPHA;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096;
			//uvrgb->f = 0x0020;
			uvrgb->f = 0x0000;
			uvrgb->r = BASE_RGB;
			uvrgb->g = BASE_RGB;
			uvrgb->b = BASE_RGB;
			uvrgb->a = ALPHA;
			uvrgb++;

			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096;
			uvrgb->f = 0x0020;
			//uvrgb->f = 0x0000;
			uvrgb->r = BASE_RGB;
			uvrgb->g = BASE_RGB;
			uvrgb->b = BASE_RGB;
			uvrgb->a = ALPHA;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096;
			//uvrgb->f = 0x0020;
			uvrgb->f = 0x0000;
			uvrgb->r = BASE_RGB;
			uvrgb->g = BASE_RGB;
			uvrgb->b = BASE_RGB;
			uvrgb->a = ALPHA;
			uvrgb++;
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);

	return 1;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->center = GM_PlayerPosition;
	work->life = GCL_GetOptionValue( 'l', -1 );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_CULLPOLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
					  N_PRIMS, N_VERTS );
	if ( prim == NULL ) {
		printf("null prim\n");
		return -1;
	}

	tex = DG_GetTexture( GV_StrCode( "drop01_msk" ) );

	InitPacket( work, prim, tex );

	if( work->life < 0 ) return -1;
	return 0;

}

void *NewCulTest( int name, int map )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) );

	if ( work != NULL ) {

		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;
		work->map = map;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
