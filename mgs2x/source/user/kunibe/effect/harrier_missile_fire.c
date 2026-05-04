//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  harrier_missile_fire.c
  ミサイル弾炎
  2001/04/05 Yuuta Kunibe
	
  $Id: harrier_missile_fire.c,v 1.1.1.3 2002/11/19 11:44:41 Yoshizawa1 Exp $
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


#define COL_R	(128)
#define COL_G	(96)
#define COL_B	(8)
#define ALPHA	(48)

#define N_PRIMS	(16)
#define N_VERTS	(1)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define SPEED	(300.f)

extern void *NewHarrierMissileSmokeLine( FVECTOR *pos, FVECTOR *vec, char *flag );
extern void *NewHarrierFlare( FVECTOR *pos );


typedef	struct	{

    GV_ACT_EX	actor;    

    DG_PRIM2	*prim;

    FVECTOR	center;
    FVECTOR	*pos;
    FVECTOR	*vec;

    float	size;

    int 	cnt;
    char 	*flag;

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

#if 0    
    printf("cnt : %d\n",work->cnt);
#endif    

    /* ポッド通過時決め打ち */
    switch ( work->cnt ) {
    case 0:
	work->size = 600.f + frnd() * 100.f;
	break;
    case 1:
	work->size = 500.f + frnd() * 100.f;
	break;
    case 2:
	work->size = 400.f + frnd() * 100.f;
	break;
    case 3:
	work->size = 300.f + frnd() * 100.f;
	break;
    case 4:
	work->size = 200.f;
	break;
    case 5:
	NewHarrierMissileSmokeLine( work->pos, work->vec, work->flag );
	work->size = 200.f;
	break;
    case 6:
	work->size = 1000.f + frnd() * 250.f;
	break;
    case 7:
	work->size = 2000.f + frnd() * 500.f;
	break;
    case 8:
	work->size = 4000.f + frnd() * 1000.f;
	break;
    default:
	work->size = work->size * 0.8f + 400.f * 0.2f;
	break;
    }
    
    for ( i = 0 ; i < N_PRIMS ; i++ ) {

	if ( work->cnt >= 5 ) {
	    DG_COPY_VEC( &vectmp, work->vec );
	    scale = 750.f + 100.f * (float)i;
	    _sceVu0ScaleVector( &vectmp, &vectmp, scale );
	    _sceVu0SubVector( pos, work->pos, &vectmp );

	    pos->vx += 50.f * frnd();
	    pos->vy += 50.f * frnd();
	    pos->vz += 50.f * frnd();
	}
	else {
	    DG_COPY_VEC( &vectmp, work->vec );
	    scale = 750.f + work->cnt * SPEED + 100.f * (float)i;
	    _sceVu0ScaleVector( &vectmp, &vectmp, scale );
	    _sceVu0SubVector( pos, work->pos, &vectmp );
	}
	    

	if ( i <= 10 ) {
	    uvrgbwh->w  = 200.f + work->size * (float)i / N_PRIMS;
	    uvrgbwh->h  = 200.f + work->size * (float)i / N_PRIMS;
	}
	else {
	    uvrgbwh->w  = 200.f + work->size * (float)( 20 - i ) / N_PRIMS;
	    uvrgbwh->h  = 200.f + work->size * (float)( 20 - i ) / N_PRIMS;
	}

	pos++;
	uvrgbwh++;

    }

    work->cnt++;
    	
    if ( *work->flag ) {
	GV_DestroyActor( work );
    }
    

}


static void Die(Work *work )
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
	    _sceVu0SubVector( pos, work->pos, &vectmp );

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

	    /*uvrgbwh->r  = COL_R;
	    uvrgbwh->g  = COL_G * ( N_PRIMS - i ) / N_PRIMS;
	    uvrgbwh->b  = COL_B;
	    uvrgbwh->a  = ALPHA * ( N_PRIMS - i ) / N_PRIMS;*/

	    uvrgbwh->r  = r1 * ( N_PRIMS - i ) / N_PRIMS + r2 * i / N_PRIMS;
	    uvrgbwh->g  = g1 * ( N_PRIMS - i ) / N_PRIMS + g2 * i / N_PRIMS;
	    uvrgbwh->b  = b1 * ( N_PRIMS - i ) / N_PRIMS + b2 * i / N_PRIMS;
	    uvrgbwh->a  = a1 * ( N_PRIMS - i ) / N_PRIMS + a2 * i / N_PRIMS;

	    uvrgbwh->w  = 200.f + 250.0f * (float)i / N_PRIMS;
	    uvrgbwh->h  = 200.f + 250.0f * (float)i / N_PRIMS;

	    if ( i <= 10 ) {
		uvrgbwh->w  = 200.f + work->size * (float)i / N_PRIMS;
		uvrgbwh->h  = 200.f + work->size * (float)i / N_PRIMS;
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
    work->flag = flag;
    work->size = 1000.f + frnd() * 200.f;
    work->cnt = 0;

    //prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, 1 );
    prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, 16 );
    if ( prim == NULL ) {
	return -1;
    }

    tex = DG_GetTexture( GV_StrCode( "drop01_msk" ) );
    InitPacket( work, prim, tex, color1, color2 );    

    return 0;
}


void *NewHarrierMissileFire( FVECTOR *pos, FVECTOR *vec, char *flag, int color1, int color2 )
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





