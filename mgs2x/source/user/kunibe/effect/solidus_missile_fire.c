//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  solidus_missile_fire.c

  2001/05/11 Yuuta Kunibe	
  $Id: solidus_missile_fire.c,v 1.1.1.3 2002/11/19 11:44:50 Yoshizawa1 Exp $
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

#define COL_R	(30)
#define COL_G	(50)//(60)//(80)
#define COL_B	(60)//(70)
#define ALPHA	(64)//(128)

#define	SIZE	(500.0f)

#define N_PRIMS	(16)
#define N_VERTS	(1)

#define OFFSET	(0.0f)
#define LIFE	(180)




typedef	struct	{

    GV_ACT_EX	actor;
    int 	name;

    DG_PRIM2	*prim;

    FVECTOR	*pos;
    FVECTOR	*vec;

    int		*phase;
    int		pre_phase;
    int		demo_phase;

    float	size;

    int 	life;

} Work;




static int RecieveMessage( Work *work )
{
    GV_MSG*	msg;
    int 	n_msg;
    int		ret;


    ret = 1;
    if ( work->name == 0 ) {
	return ret;
    }

    /* メッセージ取得 */
    n_msg = GV_ReceiveMessage( work->name, &msg ); 

    /* メッセージ反映 */
    while ( n_msg-- > 0 ){

	switch ( msg->message[ 0 ] ) {
	case 0:	/* 加速 */
	    work->demo_phase = 1;
	    break;
	case 1:	/* 着弾 */
	    ret = 0;
	    break;		       		       
	}
	msg++;
    }

    return ret;

}




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



    /* デモ用フェーズ切替えメッセージ取得関数 */
    if ( !RecieveMessage( work ) ) {
	GV_DestroyActor( work );
	return;
    }    


    if ( work->pre_phase == 0 && *work->phase == 1 ) {
	work->size = SIZE * 1.5f;
    }
    else {
    
	switch ( *work->phase ) {
	case 0:
	    work->size = SIZE / 4.0f;
	    break;
	case 1:
	    work->size = SIZE / 2.0f * 0.1f + work->size * 0.9f;
	    break;
	}

    }
    
	
    width = work->size / 6.0f;
    work->pre_phase = *work->phase;






    _sceVu0Normalize( &vec, work->vec );
    
    for ( i = 0 ; i < N_PRIMS ; i++ ) {

	scale = OFFSET + width * (float)i;
	_sceVu0ScaleVector( &vectmp, &vec, scale );
	_sceVu0SubVector( pos, work->pos, &vectmp );

	pos->vx += work->size / 10.0f * frnd();
	pos->vy += work->size / 10.0f * frnd();
	pos->vz += work->size / 10.0f * frnd();

	if ( i <= 8 ) {
	    uvrgbwh->w  = work->size + work->size * (float)i / N_PRIMS;
	    uvrgbwh->h  = work->size + work->size * (float)i / N_PRIMS;
	}
	else {
	    uvrgbwh->w  = work->size + work->size * (float)( 16 - i ) / N_PRIMS;
	    uvrgbwh->h  = work->size + work->size * (float)( 16 - i ) / N_PRIMS;
	}

	pos++;
	uvrgbwh++;

    }

    /*if ( ++work->life >= LIFE ) {
	GV_DestroyActor( work );
    }*/

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
	    uvrgbwh->g  = COL_G;// * ( N_PRIMS - i ) / N_PRIMS;
	    uvrgbwh->b  = COL_B/2 + COL_B/2 * ( N_PRIMS - i ) / N_PRIMS;
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
	    uvrgbwh++;
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

    work->pos       = pos;
    work->vec       = vec;

    work->size = SIZE;
    work->life = 0;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, 16 );
    if ( prim == NULL ) {
	return -1;
    }

    tex = DG_GetTexture( GV_StrCode( "flare0_msk" ) );
    InitPacket( work, prim, tex );    

    return 0;
}


void *NewSolidusMissileFire( FVECTOR *pos, FVECTOR *vec, int *phase )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = 0;
		work->phase = phase;
		work->pre_phase = *phase;
		if ( GetResources( work, pos, vec ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}




/* デモ用呼びだし口 */
void *NewDemoSolidusMissileFire( int name, FMATRIX *mat )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->name = name;
		work->demo_phase = 0;
		work->phase = &work->demo_phase;
		work->pre_phase = *work->phase;
		if ( GetResources( work, (FVECTOR *)mat->m[3], (FVECTOR *)mat->m[1] ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}


