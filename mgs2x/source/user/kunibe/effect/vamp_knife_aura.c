//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  vamp_knife_aura.c

  2001/05/11 Yuuta Kunibe	
  $Id: vamp_knife_aura.c,v 1.1.1.3 2002/11/19 11:44:54 Yoshizawa1 Exp $
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


#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define COL_R	(128)
#define COL_G	(96)
#define COL_B	(32)
#define ALPHA	(15)

#define N_PRIMS	(16)
#define N_VERTS	(1)

#define OFFSET	(100.f)
#define LIFE	(180)




typedef	struct	{

    GV_ACT_EX	actor;    

    DG_PRIM2	*prim;

    FVECTOR	*pos;
    FVECTOR	*vec;

    float	size;
    char	*flag;

    int 	life;

} Work;




static void Act( Work *work )
{

    int			i;
    int	  		clock;
    FVECTOR  		*pos;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    FVECTOR		vectmp;
    float		scale;
    float		width;

    FVECTOR		vec;

    /* 操作する頂点バッファ取得 */
    DG_SwitchBuffPrim2( work->prim );
    clock = work->prim->buffer_clock;

    pos = work->prim->pos[clock];
    uvrgbwh = work->prim->uvrgb[clock];


    width = 100.f + frnd()*25.0f;

    _sceVu0Normalize( &vec, work->vec );
    
    for ( i = 0 ; i < N_PRIMS ; i++ ) {

	scale = OFFSET + width * (float)i;
	//_sceVu0ScaleVector( &vectmp, work->vec, scale );
	_sceVu0ScaleVector( &vectmp, &vec, scale );
	_sceVu0SubVector( pos, work->pos, &vectmp );

	pos->vx += 50.f * frnd();
	pos->vy += 50.f * frnd();
	pos->vz += 50.f * frnd();

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

    if ( ++work->life >= LIFE ) {
	GV_DestroyActor( work );
    }

}


static void Die(Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	int		i;
	FVECTOR		*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	FVECTOR		vectmp;
	float		scale;

	FVECTOR		vec;

	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );


	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;

	_sceVu0Normalize( &vec, work->vec );
	
	for ( i = 0 ; i < N_PRIMS ; i++ ){

	    //DG_COPY_VEC( &vectmp, work->vec );
	    DG_COPY_VEC( &vectmp, &vec );
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

	    uvrgbwh->r  = COL_R;
	    uvrgbwh->g  = COL_G * ( N_PRIMS - i ) / N_PRIMS;
	    uvrgbwh->b  = COL_B;
	    uvrgbwh->a  = ALPHA * ( N_PRIMS - i ) / N_PRIMS;

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



static int GetResources( Work *work, FVECTOR *pos, FVECTOR *vec )
{
    
    DG_PRIM2		*prim ;
    DG_TEX		*tex ;

    work->pos  = pos;
    work->vec  = vec;
    //work->size = 4000.f + frnd() * 500.f;
    work->size = 1500.f + frnd() * 200.f;
    work->life = 0;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, 16 );
    if ( prim == NULL ) {
	return -1;
    }

//    tex = DG_GetTexture( GV_StrCode( "flare0_msk" ) );
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
    InitPacket( work, prim, tex );    

    return 0;
}


void *NewVampKnifeAura( FVECTOR *pos, FVECTOR *vec )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, pos, vec ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}





