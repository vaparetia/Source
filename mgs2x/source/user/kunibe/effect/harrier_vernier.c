//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  harrier_vernier.c
  ハリアーバーニア炎
  2001/04/06 Yuuta Kunibe	
  $Id: harrier_vernier.c,v 1.1.1.3 2002/11/19 11:44:43 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include 	"libutl.h"
#include	"libdg.h"
#include	"rand.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"../../okajima/etc/ok_util.h"


#define COL_R	(32)
#define COL_G	(32)
#define COL_B	(128)
#define ALPHA	(8)

#define N_PRIMS	(64)
#define N_VERTS	(1)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define LIFE	(240)


typedef	struct	{

    GV_ACT_EX	actor;    

    DG_PRIM2	*prim;

    FVECTOR	center;
    FVECTOR	*pos;
    FVECTOR	*vec;

    float	param;
    
    int 	life;
    float	size;

    char	*flag;
    
} Work;


/*	
	スプライトは２頂点の指定ではなく
	スプライトの中心点と幅,高さ及び
	特別に DG_PRIM2_UVRGBWH で２頂点のパラメータを指定するところに注意！
*/
static void Act( Work *work )
{

    int			i;
    int	  		clock;
    FVECTOR  		*pos;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    FVECTOR		vectmp;
    float		scale;
    float		unit_z;
    float		unit_size;

    /* スプライト中心座標取得 */
    work->center = GM_PlayerPosition;

    /* 操作する頂点バッファ取得 */
    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }
    clock = work->prim->buffer_clock;

    pos = work->prim->pos[clock];
    uvrgbwh = work->prim->uvrgb[clock];
    uvrgbwh_pre = work->prim->uvrgb[1-clock];


    //unit_z = 12.f + 14.f + 3.f* (float)(irnd()%5);
    unit_z = work->param * 2.f + 3.f * (float)(irnd()%5);
    unit_size = work->size * ( work->param / 26.f );	


    if ( work->param > 0.0f ) {

	for ( i = 0 ; i < N_PRIMS ; i++ ) {

	    DG_COPY_VEC( &vectmp, work->vec );
	    scale = 750.f + unit_z * (float)i;
	    _sceVu0ScaleVector( &vectmp, &vectmp, scale );
	    _sceVu0AddVector( pos, work->pos, &vectmp );

	    pos->vx += 500.f * frnd() * (float)i / (float)N_PRIMS;
	    pos->vy += 500.f * frnd() * (float)i / (float)N_PRIMS;
	    if ( pos->vx < 3000.f && pos->vx > -3000.f && pos->vy < 0.f ) {
		pos->vy = 0.f;
	    }
	    pos->vz += 500.f * frnd() * (float)i / (float)N_PRIMS;

	    uvrgbwh->w  = 500.f + unit_size * (float)i / N_PRIMS * (1.0f + frnd()*0.2f);
	    uvrgbwh->h  = 500.f + unit_size * (float)i / N_PRIMS * (1.0f + frnd()*0.2f);
	    uvrgbwh->a  = ALPHA * ( i + N_PRIMS ) / N_PRIMS / 2 * ( work->param / 26.f );

	    pos++;
	    uvrgbwh++;

	}

    }

    if ( *work->flag ) {
	if ( work->param < 26.f ) work->param += 2.f;
	DG_VisiblePrim2( work->prim );
    }
    else {
	if ( work->param > 0.f ) work->param -= 1.f;
	else DG_InvisiblePrim2( work->prim );
    }

    work->life++;

    
    /*if (--work->life == 0) {
	GV_DestroyActor( work );
    }*/
    
}


static void Die( Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
}

/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int color1, int color2 )
{
	int		i;
	FVECTOR		*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	FVECTOR		vectmp;
	float		scale;
	float angle;

	int   r1,g1,b1,a1;
	int   r2,g2,b2,a2;



	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	r1 = ( ( color1 >> 24 ) & 0xff );
	g1 = ( ( color1 >> 16 ) & 0xff );
	b1 = ( ( color1 >>  8 ) & 0xff );
	a1 = ( color1 & 0xff );

	r2 = ( ( color2 >> 24 ) & 0xff );
	g2 = ( ( color2 >> 16 ) & 0xff );
	b2 = ( ( color2 >>  8 ) & 0xff );
	a2 = ( color2 & 0xff );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;

	for ( i = 0 ; i < N_PRIMS ; i++ ){

	    DG_COPY_VEC( &vectmp, work->vec );
	    scale = 750.f + 100.f * (float)i;
	    _sceVu0ScaleVector( &vectmp, &vectmp, scale );
	    _sceVu0AddVector( pos, work->pos, &vectmp );

	    pos->vx += 50.f * frnd();
	    pos->vy += 50.f * frnd();
	    pos->vz += 50.f * frnd();

	    uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    /*uvrgbwh->r  = 128 * ( N_PRIMS - i ) / N_PRIMS;
	    uvrgbwh->g  = 128 * ( N_PRIMS - i ) / N_PRIMS;
	    uvrgbwh->b  = COL_B;
	    uvrgbwh->a  = ALPHA * ( i + N_PRIMS ) / N_PRIMS / 2;*/

	    uvrgbwh->r  = r1 * ( N_PRIMS - i ) / N_PRIMS + r2 * i / N_PRIMS;
	    uvrgbwh->g  = g1 * ( N_PRIMS - i ) / N_PRIMS + g2 * i / N_PRIMS;
	    uvrgbwh->b  = b1 * ( N_PRIMS - i ) / N_PRIMS + b2 * i / N_PRIMS;
	    uvrgbwh->a  = a1 * ( N_PRIMS - i ) / N_PRIMS + a2 * i / N_PRIMS;

	    angle  = frnd() * TPI;
	    if ( i <= 10 ) {
		uvrgbwh->w  = 200.f + work->size * (float)i / N_PRIMS * vu0_Cos( angle );
		uvrgbwh->h  = 200.f + work->size * (float)i / N_PRIMS * vu0_Sin( angle );
	    }
	    else {
		uvrgbwh->w  = 200.f + work->size * (float)( 20 - i ) / N_PRIMS;
		uvrgbwh->h  = 200.f + work->size * (float)( 20 - i ) / N_PRIMS;
	    }

	    pos++;
	    uvrgbwh++ ;
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources( Work *work, FVECTOR *pos, FVECTOR *vec, char *flag, int color1, int color2 )
{
    
    DG_PRIM2		*prim ;
    DG_TEX		*tex ;

    work->center = GM_PlayerPosition;

    work->pos = pos;
    work->vec = vec;
    work->size = 1200.f + frnd() * 250.f;
    work->life = 0;
    work->param = 0.0f;
    work->flag = flag;

    prim = work->prim = GM_MakePrim2(
				     DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 4, 16 );
    if ( prim == NULL ) {
	return -1;
    }

    tex = DG_GetTexture( GV_StrCode( "flare0_msk" ) );
    InitPacket( work, prim, tex, color1, color2 );    

    return 0;
}


void *NewHarrierVernier( FVECTOR *pos, FVECTOR *vec, char *flag, int color1, int color2 )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, pos, vec, flag, color1, color2 ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}





