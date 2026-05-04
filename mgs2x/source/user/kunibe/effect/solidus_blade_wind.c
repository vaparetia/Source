//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    solidus_dash_fire.c
    ソリダスダッシュ炎
    2001/04/27 Yuuta Kunibe	
    $Id: solidus_blade_wind.c,v 1.1.1.3 2002/11/19 11:44:49 Yoshizawa1 Exp $
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

#include	"../../mode/demo/eft_con.h"



#define	N_INTERP	(8)		/* フレーム間補間数 */
#define	N_VERTS		(N_INTERP*2+2)	/* フレーム間頂点数 */
#define	N_PRIMS		(32)		/* 残像フレーム数 */


#define	WIND_TEX	(8038630)/* GV_StrCode( "wave12_alp_ovl" ) */
#define COL_R		(32)
#define COL_G		(32)
#define COL_B		(32)
#define ALPHA		(16)

#define SCR_POS 	(SCRPAD_ADDR)
#define SCR_UVS 	(SCRPAD_ADDR+0x2000)

#define	SPEED		(300.0f)     




typedef	struct	{


    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FVECTOR	*pos_bottom;
    FVECTOR	*pos_top;
       
    FVECTOR	position[6];

    float	top_param;
    float	bottom_param;

    FVECTOR	vec[N_PRIMS*N_VERTS/2];

    int		*alpha;
    
    int		id;
    int		count;

    /* 前フレーム情報 */
    int		prepos_id;
    FVECTOR	prepos_top;
    FVECTOR	prepos_bottom;

    FVECTOR	prevec_top;
    FVECTOR	prevec_bottom;    

    FVECTOR	prevec_spread;

    int     pre_clock;
} Work;




/* アクト関数 */
static void Act( Work *work )
{

    int 		i, clock;
    FVECTOR		*pos;
    FVECTOR		*pos_pre;
    DG_PRIM2_UVRGB	*uvrgb;
    DG_PRIM2_UVRGB	*uvrgb_pre;
    FVECTOR		*vec;


    FVECTOR		vectmp;


    FVECTOR		vec_top;
    FVECTOR		vec_bottom;
    float		param;
    float		param_now;
    float		param_pre;
    FVECTOR		vectmp_now;
    FVECTOR		vectmp_pre;
	
    

    
    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }

    clock = work->prim->buffer_clock;
    pos       = work->prim->pos[clock];
    pos_pre   = work->prim->pos[1-clock];
    uvrgb     = work->prim->uvrgb[clock];
    uvrgb_pre = work->prim->uvrgb[1-clock];


    // ja armature -- this probably isn't a 100% correct fix
    // occasionally, this skips a clock, or runs twice between clocks
    // which makes the verlet integration it's trying to do explode
    if (clock == work->pre_clock)
       return;
    work->pre_clock = clock;


    for ( i=0 ; i< N_PRIMS*N_VERTS ; i++ ) {

	if ( !(i%2 ) ) {
	    _sceVu0AddVector( &pos[i], &pos_pre[i], &work->vec[i/2] );
	}
	else {
	    DG_COPY_VEC( &pos[i], &pos_pre[i] );
	}

	if ( clock ) {
	    if ( uvrgb_pre[i].a > 0 ) {
		uvrgb[i].a = uvrgb_pre[i].a - 1;
	    }
	    else {
		uvrgb[i].a = 0;
	    }
	}
	else {
	    uvrgb[i].a = uvrgb_pre[i].a;
	}

    }



    /* ここから新規頂点 */
    {

	DG_COPY_VEC( &work->prepos_top,    &pos[work->prepos_id] );
	DG_COPY_VEC( &work->prepos_bottom, &pos[work->prepos_id+1] );

	_sceVu0SubVector( &vec_top,    work->pos_top,    &work->prepos_top );
	_sceVu0SubVector( &vec_bottom, work->pos_bottom, &work->prepos_bottom );
	_sceVu0ScaleVector( &work->prevec_top, &work->prevec_top, 0.75f );
	_sceVu0ScaleVector( &work->prevec_bottom, &work->prevec_bottom, 0.75f );

	pos   = &work->prim->pos[clock][work->id*N_VERTS];
	uvrgb = &( (DG_PRIM2_UVRGB *)(work->prim->uvrgb[clock]) )[work->id*N_VERTS];
	vec   = &work->vec[work->id*N_VERTS/2];
	
	for ( i = 0 ; i < N_INTERP ; i++ ) {

	    /*--- パラメータ算出 ---*/
	    param = (float)i / (float)N_INTERP;
	    param_now = (float)i / (float)N_INTERP;
	    param_pre = 1.0f - param_now;


	    /*--- 先端位置算出 ---*/	    
	    _sceVu0ScaleVector( &vectmp_now, &vec_top,          param * param_now ); /* 現フレーム成分 */	    
	    _sceVu0ScaleVector( &vectmp_pre, &work->prevec_top, param * param_pre ); /* 前フレーム成分 */
	    /* 合成 */
	    _sceVu0AddVector( pos, &vectmp_now, &vectmp_pre );
	    _sceVu0AddVector( pos, pos, &work->prepos_top );
	    pos++;
	    

	    /*--- 末端位置算出 ---*/	    
	    _sceVu0ScaleVector( &vectmp_now, &vec_bottom,          param * param_now ); /* 現フレーム成分 */	    
	    _sceVu0ScaleVector( &vectmp_pre, &work->prevec_bottom, param * param_pre ); /* 前フレーム成分 */
	    /* 合成 */
	    _sceVu0AddVector( pos, &vectmp_now, &vectmp_pre );
	    _sceVu0AddVector( pos, pos, &work->prepos_bottom );

	    /* 速度算出 */
	    if ( i==0 ) {
		DG_COPY_VEC( vec, &work->prevec_spread );
	    }
	    else {
		_sceVu0SubVector( &vectmp, (pos-1), pos );
		_sceVu0Normalize( &vectmp, &vectmp );
		_sceVu0ScaleVector( vec, &vectmp, SPEED );
	    }

	    pos++;
	    vec++;

	    uvrgb->a = *work->alpha;
	    uvrgb++;
	    uvrgb->a = *work->alpha;
	    uvrgb++;
	    
	}

	/*--- 最後現在点 ---*/
	DG_COPY_VEC( pos, work->pos_top );
	pos++;
	DG_COPY_VEC( pos, work->pos_bottom );

	_sceVu0SubVector( &vectmp, (pos-1), pos );
	_sceVu0Normalize( &vectmp, &vectmp );
	_sceVu0ScaleVector( vec, &vectmp, SPEED );

	DG_COPY_VEC( &work->prevec_spread, vec );

	work->prepos_id = (work->id+1)*N_VERTS-2;

	pos++;

	    uvrgb->a = *work->alpha;
	    uvrgb++;
	    uvrgb->a = *work->alpha;
	    uvrgb++;

    }
	    
	    
    
    if ( ++work->id >= 32 ) {
	work->id = 0;
    }

    _sceVu0SubVector( &work->prevec_top,    work->pos_top,    &work->prepos_top );
    _sceVu0SubVector( &work->prevec_bottom, work->pos_bottom, &work->prepos_bottom );
        
}



static void Die( Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
}




/*-------- InitPacket : プリミティブ初期化関数 --------*/
static int InitPacket( Work *work )
{

	int			i,j;
	DG_PRIM2		*prim;
	DG_TEX			*tex;
	FVECTOR			*pos;
	DG_PRIM2_UVRGB		*uvrgb;




	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY | DG_PRIM2_ALPHA | DG_PRIM2_SHADE | DG_PRIM2_TEX,
					  N_PRIMS, N_VERTS );
	if ( prim == NULL ) {
		return 0;
	}

	prim->raise = 0;

	tex = DG_GetTexture( WIND_TEX );
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );



	work->bottom_param = 0.45f + frnd()*0.45f;	// 0.0f～0.5f
	work->top_param    = work->bottom_param + 0.1f;	// 0.5f～1.0f


	for ( j = 0 ; j < 4 ; j++ ) {

	    pos   = SCR_POS;
	    uvrgb = SCR_UVS;

	    for ( i = 0 ; i < N_PRIMS/4*N_VERTS ; i++ ) {

		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos++;

		if ( i%2 ) {
		    uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		}
		else {
		    uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		    DG_COPY_VEC( &work->vec[(N_PRIMS/4*N_VERTS*j+i)/2], &DG_ZeroVector );	    
		}
		uvrgb->v = FTOI12( (float)(i/2) / (float)(N_PRIMS*N_VERTS/2) * tex->v_scale + tex->v_offset ) ;

		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = COL_R;
		uvrgb->g = COL_G;
		uvrgb->b = COL_B;
		uvrgb->a = ALPHA;
		uvrgb++;

	    }	    

	    OK_Scr_Mem( &prim->pos[ 0 ][N_PRIMS/4*N_VERTS*j],   SCR_POS, sizeof(FVECTOR), N_PRIMS/4*N_VERTS );
	    OK_Scr_Mem( &prim->pos[ 1 ][N_PRIMS/4*N_VERTS*j],   SCR_POS, sizeof(FVECTOR), N_PRIMS/4*N_VERTS );
	    OK_Scr_Mem( &( (DG_PRIM2_UVRGB *)prim->uvrgb[ 0 ] )[N_PRIMS/4*N_VERTS*j],
			SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS/4*N_VERTS );
	    OK_Scr_Mem( &( (DG_PRIM2_UVRGB *)prim->uvrgb[ 1 ] )[N_PRIMS/4*N_VERTS*j],
			SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS/4*N_VERTS );

	}

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}




static int GetResources( Work *work, FVECTOR *pos_bottom, FVECTOR *pos_top, int *alpha )
{

    work->pos_top    = pos_top;
    DG_COPY_VEC( &work->prepos_top, pos_top );
    work->pos_bottom = pos_bottom;
    DG_COPY_VEC( &work->prepos_bottom, pos_bottom );

    DG_COPY_VEC( &work->prevec_top, &DG_ZeroVector );
    DG_COPY_VEC( &work->prevec_bottom, &DG_ZeroVector );

    DG_COPY_VEC( &work->prevec_spread, &DG_ZeroVector );

    work->prepos_id = 0;
    work->pre_clock = -1;

    work->alpha = alpha;
    work->id    = 0;
    work->count = 0;    
    
    if ( InitPacket( work ) ) {
		return 0;
	}
	else {
		return -1;
	}

}






void *NewSolidusBladeWind( FVECTOR *pos_bottom, FVECTOR *pos_top, int *alpha )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, pos_bottom, pos_top, alpha ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}




