//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  point_debug.c
  位置確認用エフェクト
  2001/04/10 Yuuta Kunibe
  $Id: point_debug.c,v 1.1.1.3 2002/11/19 11:44:45 Yoshizawa1 Exp $
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
#define	N_PRIMS		(16)

#define BASE_RGB	(128)
#define ALPHA		(32)

#define SIZE		(500.f)

#define LIFE		(1)//(300)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)


typedef	struct	{

    GV_ACT_EX	actor ;

    DG_PRIM2	*prim ;

    FVECTOR	pos;

    SVECTOR	rot[N_PRIMS];
    SVECTOR	rot_add[N_PRIMS];

    FVECTOR	vert[4];

    int 	life;

} Work ;


static void Act( Work *work )
{
	int		i;
	int	  	clock;
	FVECTOR 	*pos;


	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	pos = work->prim->pos[clock];


	for( i = 0 ; i < N_PRIMS ; i++ ){	    
	    
	    DG_SetPos2( &DG_ZeroVector, &work->rot[i] );
	    DG_PutVector( work->vert, pos, N_VERTS );

	    _sceVu0AddVector( pos, pos, &work->pos );
	    pos++;
	    _sceVu0AddVector( pos, pos, &work->pos );
	    pos++;
	    _sceVu0AddVector( pos, pos, &work->pos );
	    pos++;
	    _sceVu0AddVector( pos, pos, &work->pos );
	    pos++;

	    work->rot[i].vx += work->rot_add[i].vx;
	    work->rot[i].vy += work->rot_add[i].vy;
	    work->rot[i].vz += work->rot_add[i].vz;

	}

	if ( ++work->life >= LIFE ) {
	    GV_DestroyActor( work );
	}

}


static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}


static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{

	FVECTOR		*pos;
	DG_PRIM2_UVRGB	*uvrgb;
	int		i;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos   = prim->pos[0];//SCR_POS;
	uvrgb = prim->uvrgb[0];//SCR_UVS;

	for ( i = 0 ; i < N_PRIMS ; i++ ){

	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;

	    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x8fff;
	    uvrgb->r = BASE_RGB;
	    uvrgb->g = BASE_RGB;
	    uvrgb->b = BASE_RGB;
	    uvrgb->a = ALPHA;
	    uvrgb++;
	    
	    uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x8fff;
	    uvrgb->r = BASE_RGB;
	    uvrgb->g = BASE_RGB;
	    uvrgb->b = BASE_RGB;
	    uvrgb->a = ALPHA;
	    uvrgb++;
	    
	    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x0fff;
	    uvrgb->r = BASE_RGB;
	    uvrgb->g = BASE_RGB;
	    uvrgb->b = BASE_RGB;
	    uvrgb->a = ALPHA;
	    uvrgb++;
	    
	    uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x0fff;
	    uvrgb->r = BASE_RGB;
	    uvrgb->g = BASE_RGB;
	    uvrgb->b = BASE_RGB;
	    uvrgb->a = ALPHA;
	    uvrgb++;

	    work->rot[i].vx = irnd()%4096;
	    work->rot[i].vy = irnd()%4096;
	    work->rot[i].vz = irnd()%4096;

	    work->rot_add[i].vx = irnd()%4096/20;
	    work->rot_add[i].vx = irnd()%4096/20;
	    work->rot_add[i].vx = irnd()%4096/20;
	}	

	/*OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );*/
	OK_Scr_Mem( prim->pos[ 1 ],   prim->pos[ 0 ], sizeof(FVECTOR),        N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], prim->uvrgb[ 0 ], sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );
 
	for ( i = 0 ; i < N_VERTS ; i++ ) {

	    DG_COPY_VEC( &work->vert[0], &DG_ZeroVector );
	    work->vert[0].vx = SIZE;
	    work->vert[0].vz = SIZE;
	    DG_COPY_VEC( &work->vert[1], &DG_ZeroVector );
	    work->vert[1].vx =-SIZE;
	    work->vert[1].vz = SIZE;
	    DG_COPY_VEC( &work->vert[2], &DG_ZeroVector );
	    work->vert[2].vx = SIZE;
	    work->vert[2].vz =-SIZE;
	    DG_COPY_VEC( &work->vert[3], &DG_ZeroVector );
	    work->vert[3].vx =-SIZE;
	    work->vert[3].vz =-SIZE;

	}

	return 1;
}

static int GetResources( Work *work, FVECTOR *pos )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	DG_COPY_VEC( &work->pos, pos );
	work->life = 0;

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if ( prim == NULL ) {
	    return -1;
	}

	tex = DG_GetTexture( GV_StrCode( "flare0_msk" ) );

	InitPacket( work, prim, tex );

	return 0;

}

void *NewPointDebug( FVECTOR *pos )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {

		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor )

		if ( GetResources( work, pos ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

