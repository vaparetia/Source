//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  point_debug.c
  位置確認用エフェクト
  2001/04/10 Yuuta Kunibe
  $Id: harrier_claster_spark.c,v 1.1.1.3 2002/11/19 11:44:40 Yoshizawa1 Exp $
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
#define	N_PRIMS		(4)

#define BASE_R		(128)
#define BASE_G		(64)
#define BASE_B		(32)

#define ALPHA		(48)
#define	LIGHT_ALPHA	(96)

#define SIZE		(2000.f)//(8000.f)//(2000.f)

#define LIFE		(30)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)

extern void *NewRunningSmoke2( FVECTOR* pos, float size );


typedef	struct	{

    GV_ACT_EX	actor ;

    DG_PRIM2	*prim ;
    DG_PRIM2	*prim_light;

    FVECTOR	pos;

    SVECTOR	rot[N_PRIMS];
    SVECTOR	rot_add[N_PRIMS];

    float	light_size;
    float	light_alpha;
    
    FVECTOR	vec;

    FVECTOR	vert[4];
    int 	life;

} Work ;



/* 床照り返しアクト */
static void ActLight( Work *work )
{

    int			clock;
    DG_PRIM2		*prim;
    FVECTOR		*pos;
    DG_PRIM2_UVRGB	*uvrgb;



    prim = work->prim_light;    

    DG_SwitchBuffPrim2( prim );
    clock = prim->buffer_clock;

    pos   = prim->pos[clock];
    uvrgb = prim->uvrgb[clock];



    DG_COPY_VEC( pos, &DG_ZeroVector );
    pos->vx = work->light_size * ( 1.0f + frnd()*0.2f );
    pos->vz = work->light_size * ( 1.0f + frnd()*0.2f );
    pos++;
    
    DG_COPY_VEC( pos, &DG_ZeroVector );
    pos->vx =-work->light_size * ( 1.0f + frnd()*0.2f );
    pos->vz = work->light_size * ( 1.0f + frnd()*0.2f );
    pos++;
    
    DG_COPY_VEC( pos, &DG_ZeroVector );
    pos->vx = work->light_size * ( 1.0f + frnd()*0.2f );
    pos->vz =-work->light_size * ( 1.0f + frnd()*0.2f );
    pos++;
    
    DG_COPY_VEC( pos, &DG_ZeroVector );
    pos->vx =-work->light_size * ( 1.0f + frnd()*0.2f );
    pos->vz =-work->light_size * ( 1.0f + frnd()*0.2f );
    pos++;

    uvrgb->a = work->light_alpha;
    uvrgb++;
    uvrgb->a = work->light_alpha;
    uvrgb++;
    uvrgb->a = work->light_alpha;
    uvrgb++;
    uvrgb->a = work->light_alpha;
    uvrgb++;

    if ( work->life >= 15 ) {
	work->light_alpha = LIGHT_ALPHA * ( LIFE-work->life ) / ( LIFE - 15 );
    }


}

    

static void Act( Work *work )
{

	int		i;
	int	  	clock;
	FVECTOR 	*pos;
	FVECTOR		vectmp;

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	pos   = work->prim->pos[clock];


	/* 位置更新 */
   work->prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
	DG_COPY_VEC( (FVECTOR *)work->prim->as_world.m[3], &work->pos );
	//if ( work->life > 20 ) {
	    vectmp.vx = 200.f * (float)(work->life) * frnd();
	    vectmp.vy = ( 50.f + frnd()*50.0f ) * (float)(work->life);
	    vectmp.vz = 200.f * (float)(work->life) * frnd();
	    vectmp.vw = 0.f;
	    _sceVu0AddVector( (FVECTOR *)work->prim->as_world.m[3], (FVECTOR *)work->prim->as_world.m[3], &vectmp );
	//}


	/* キラキラ回転 */
	for( i = 0 ; i < N_PRIMS ; i++ ){	    
	    
	    DG_SetPos2( &DG_ZeroVector, &work->rot[i] );
	    DG_PutVector( work->vert, pos, N_VERTS );
	    pos += N_VERTS;

	    work->rot[i].vx += work->rot_add[i].vx;
	    work->rot[i].vy += work->rot_add[i].vy;
	    work->rot[i].vz += work->rot_add[i].vz;

	}



	//if ( work->life > 20 ) {
	    for ( i = 0 ; i < N_VERTS ; i++ ) {
		_sceVu0ScaleVector( &work->vert[i], &work->vert[i], 0.96f );
	    }
	    /*}
	else {
	    for ( i = 0 ; i < N_VERTS ; i++ ) {
		_sceVu0ScaleVector( &work->vert[i], &work->vert[i], 0.99f );
	    }
	}*/


	ActLight( work );



	if ( ++work->life >= LIFE ) {
	    GV_DestroyActor( work );
	}
    
}


static void Die( Work *work )
{
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
    if ( work->prim_light ) work->prim_light = OK_FreePrim2( work->prim_light );
}


static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{

	FVECTOR		*pos;
	DG_PRIM2_UVRGB	*uvrgb;
	int		i;
	float		size;
	float	angle;



	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	
	
	pos   = SCR_POS;
	uvrgb = SCR_UVS;

   work->prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
	DG_COPY_VEC( (FVECTOR *)work->prim->as_world.m[3], &work->pos );	

	work->vec.vx = 200.f * (1.0f + frnd()*0.2f);
	angle = frnd()*TPI;
	work->vec.vz = work->vec.vx * vu0_Sin( angle );
	work->vec.vx = work->vec.vx * vu0_Sin( angle );	
	
	for ( i = 0 ; i < N_PRIMS ; i++ ){

	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;
	    DG_COPY_VEC( pos, &DG_ZeroVector );	pos++;

	    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x8fff;
	    uvrgb->r = BASE_R;
	    uvrgb->g = BASE_G;
	    uvrgb->b = BASE_B;
	    uvrgb->a = ALPHA;
	    uvrgb++;
	    
	    uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x8fff;
	    uvrgb->r = BASE_R;
	    uvrgb->g = BASE_G;
	    uvrgb->b = BASE_B;
	    uvrgb->a = ALPHA;
	    uvrgb++;
	    
	    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x0fff;
	    uvrgb->r = BASE_R;
	    uvrgb->g = BASE_G;
	    uvrgb->b = BASE_B;
	    uvrgb->a = ALPHA;
	    uvrgb++;
	    
	    uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	    uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	    uvrgb->q = 4096;
	    uvrgb->f = 0x0fff;
	    uvrgb->r = BASE_R;
	    uvrgb->g = BASE_G;
	    uvrgb->b = BASE_B;
	    uvrgb->a = ALPHA;
	    uvrgb++;

	    work->rot[i].vx = irnd()%4096;
	    work->rot[i].vy = irnd()%4096;
	    work->rot[i].vz = irnd()%4096;

	    work->rot_add[i].vx = 4096/(irnd()%10 + 10);
	    work->rot_add[i].vy = 4096/(irnd()%10 + 10);
	    work->rot_add[i].vz = 4096/(irnd()%10 + 10);
	}	

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );

	size = SIZE * (0.8f + 0.2f * frnd());
	

	DG_COPY_VEC( &work->vert[0], &DG_ZeroVector );
	work->vert[0].vx = size * ( 1.0f + frnd()*0.2f );
	work->vert[0].vz = size * ( 1.0f + frnd()*0.2f );
	DG_COPY_VEC( &work->vert[1], &DG_ZeroVector );
	work->vert[1].vx =-size * ( 1.0f + frnd()*0.2f );
	work->vert[1].vz = size * ( 1.0f + frnd()*0.2f );
	DG_COPY_VEC( &work->vert[2], &DG_ZeroVector );
	work->vert[2].vx = size * ( 1.0f + frnd()*0.2f );
	work->vert[2].vz =-size * ( 1.0f + frnd()*0.2f );
	DG_COPY_VEC( &work->vert[3], &DG_ZeroVector );
	work->vert[3].vx =-size * ( 1.0f + frnd()*0.2f );
	work->vert[3].vz =-size * ( 1.0f + frnd()*0.2f );




	/* 床照り返し初期化 */
	DG_ConfigPrim2Tex( work->prim_light, tex );
	DG_SetPrim2Alpha( work->prim_light, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

   work->prim_light->flag |= DG_PRIM_AS_CUSTOMWORLD;
	DG_COPY_VEC( (FVECTOR *)work->prim_light->as_world.m[3], &work->pos );	

	pos   = SCR_POS;
	uvrgb = SCR_UVS;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx = work->light_size * ( 1.0f + frnd()*0.2f );
	pos->vz = work->light_size * ( 1.0f + frnd()*0.2f );
	pos++;
	
	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx =-work->light_size * ( 1.0f + frnd()*0.2f );
	pos->vz = work->light_size * ( 1.0f + frnd()*0.2f );
	pos++;
	
	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx = work->light_size * ( 1.0f + frnd()*0.2f );
	pos->vz =-work->light_size * ( 1.0f + frnd()*0.2f );
	pos++;
	
	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos->vx =-work->light_size * ( 1.0f + frnd()*0.2f );
	pos->vz =-work->light_size * ( 1.0f + frnd()*0.2f );
	pos++;
	
	uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb->q = 4096;
	uvrgb->f = 0x8fff;
	uvrgb->r = BASE_R;
	uvrgb->g = BASE_G;
	uvrgb->b = BASE_B;
	uvrgb->a = ALPHA;
	uvrgb++;
	
	uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb->q = 4096;
	uvrgb->f = 0x8fff;
	uvrgb->r = BASE_R;
	uvrgb->g = BASE_G;
	uvrgb->b = BASE_B;
	uvrgb->a = ALPHA;
	uvrgb++;
	
	uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb->q = 4096;
	uvrgb->f = 0x0fff;
	uvrgb->r = BASE_R;
	uvrgb->g = BASE_G;
	uvrgb->b = BASE_B;
	uvrgb->a = ALPHA;
	uvrgb++;
	
	uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb->q = 4096;
	uvrgb->f = 0x0fff;
	uvrgb->r = BASE_R;
	uvrgb->g = BASE_G;
	uvrgb->b = BASE_B;
	uvrgb->a = ALPHA;
	uvrgb++;

	OK_Scr_Mem( work->prim_light->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        4 );
	OK_Scr_Mem( work->prim_light->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        4 );
	OK_Scr_Mem( work->prim_light->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), 4 );
	OK_Scr_Mem( work->prim_light->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), 4 );

	return 1;

}

static int GetResources( Work *work, FVECTOR *pos )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	DG_COPY_VEC( &work->pos, pos );
	//work->pos.vy += 1500.f + 1500.f * frnd();
	work->life = 0;


	work->light_size = 4000.0f + frnd()*1000.0f;
	work->light_alpha = LIGHT_ALPHA;

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if ( prim == NULL ) {
	    return -1;
	}

	work->prim_light = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, 4 );
	if ( work->prim_light == NULL ) {
	    return -1;
	}

	tex = DG_GetTexture( GV_StrCode( "flare0_msk" ) );

	InitPacket( work, prim, tex );

	return 0;

}

void *NewHarrierClasterSpark( FVECTOR *pos )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {

	    GV_SetActor( &( work->actor ), Act, Die );
	    GV_ActorEX( &work->actor );
		
	    if ( GetResources( work, pos ) < 0 ) {
		GV_DestroyActor( work );
		return NULL;
	    }
	}
	return (void *)work;
}
