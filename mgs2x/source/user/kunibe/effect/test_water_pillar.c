//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  harrier_vernier.c
  ハリアーバーニア炎
  2001/04/06 Yuuta Kunibe	
  $Id: test_water_pillar.c,v 1.1.1.3 2002/11/19 11:44:53 Yoshizawa1 Exp $
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
#define COL_G	(128)
#define COL_B	(255)
#define ALPHA	(128)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define LIFE	(90)


typedef	struct	{

    GV_ACT_EX	actor;    

    DG_PRIM2	*prim;

    FVECTOR	pos;

    float	size;
    float	height;
    
    float	rising_speed;
    float 	alpha;

    int 	phase;
    int 	life;
    
} Work;


/*	
	スプライトは２頂点の指定ではなく
	スプライトの中心点と幅,高さ及び
	特別に DG_PRIM2_UVRGBWH で２頂点のパラメータを指定するところに注意！
*/
static void Act( Work *work )
{

    int	  		clock;
    FVECTOR  		*pos;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;


    /* 操作する頂点バッファ取得 */
    DG_SwitchBuffPrim2( work->prim );
    clock = work->prim->buffer_clock;

    pos     = work->prim->pos[clock];
    uvrgbwh = work->prim->uvrgb[clock];


    switch (work->phase) {
    case 0:
	work->height += work->rising_speed;
	work->rising_speed *= 0.5f;
	if ( work->rising_speed < work->size * 0.05f ) {
	    work->phase = 1;
	}
	break;
    case 1:
	work->size *= 1.01f;
	work->height -= work->size * 0.03f;
	work->alpha -= 1.50f;
	break;
    case 2:
	break;
    }


    DG_COPY_VEC( pos, &work->pos );
    pos->vy += work->height;

    uvrgbwh->w = work->size;
    uvrgbwh->h = work->height;
    uvrgbwh->a = (u_short)work->alpha;
    
    if ( ++work->life >= LIFE || work->alpha == 0 ) {
	GV_DestroyActor( work );
    }
    
}


static void Die( Work *work )
{
    /* メモリ解放 */
    work->prim = OK_FreePrim2( work->prim );
}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{

	FVECTOR			*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;


	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );


	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;



	    DG_COPY_VEC( pos, &work->pos );

	    uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 0.5F * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    uvrgbwh->w = work->size;
	    uvrgbwh->h = work->height;

	    uvrgbwh->r  = COL_R;
	    uvrgbwh->g  = COL_G;
	    uvrgbwh->b  = COL_B;
	    uvrgbwh->a  = ALPHA;

	    pos++;
	    uvrgbwh++ ;


	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          1 );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          1 );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), 1 );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), 1 );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources( Work *work, FVECTOR *pos, float size )
{
    
    DG_PRIM2		*prim ;
    DG_TEX		*tex ;


    work->pos    = *pos;
    work->size   = size;
    work->height = 0.0f;

    work->phase  = 0;
    work->life   = 0;
    work->alpha  = (float)ALPHA;

    work->rising_speed = work->size * 12.0f;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, 1 );

    tex = DG_GetTexture( GV_StrCode( "flare0_msk" ) );
    InitPacket( work, prim, tex );    

    return 0;
}


void *NewTestWaterPillar( FVECTOR *pos, float size )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, pos, size ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}
















