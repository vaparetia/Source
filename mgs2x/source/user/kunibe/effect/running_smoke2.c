//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    rising_smoke.c
    立ち昇る煙
    2001/03/28 Yuuta Kunibe	
    $Id: running_smoke2.c,v 1.1.1.3 2002/11/19 11:44:48 Yoshizawa1 Exp $
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


#define N_PRIMS (4)
#define N_VERTS	(16)
#define	N_SPRTS	(N_PRIMS*N_VERTS)

#define COLOR	(64)
#define ALPHA	(32)
#define SIZE	(100.0f)

#define	LIFE	(150)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define WIND_MAX (20.f)


/* グローバル風パラメータ */
extern FVECTOR G_wind;
extern SVECTOR G_wind_rot;
extern int G_wind_intense;
extern int G_wind_intense_max;
extern FMATRIX G_wind_matrix;



typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FVECTOR	*pos;
    FVECTOR	prevpos;

    float	radius[N_SPRTS];
    float	cos[N_SPRTS];
    float	sin[N_SPRTS];

    float	size;
    FVECTOR	wind;

    int  	id;
    int		cnt;
    int		water_flag;

} Work;



/* アクト関数 */
static void Act( Work *work )
{

    int			i;
    int			new;
    int	  		clock;

    FVECTOR  		*pos;
    FVECTOR  		*pos_pre;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;

    float		intense;
    float		len;
    float		ftmp;
    float		angle;
    FVECTOR		vectmp;
    FVECTOR		vec;
    FVECTOR		unit_vec;
    FVECTOR		local_wind;



    /* 操作する頂点バッファ取得 */
    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }

    clock = work->prim->buffer_clock;

    pos		= work->prim->pos[clock];
    pos_pre	= work->prim->pos[1-clock];
    uvrgbwh	= work->prim->uvrgb[clock];
    uvrgbwh_pre = work->prim->uvrgb[1-clock];




    /* 風計算 */
    intense = WIND_MAX / (float)G_wind_intense_max;
    _sceVu0ScaleVector( &local_wind, &G_wind, intense );
    local_wind.vy = 0.f;
    work->wind.vx = work->wind.vx * 0.98f + local_wind.vx * 0.02f;
    work->wind.vy = work->wind.vy * 0.98f + local_wind.vy * 0.02f;
    work->wind.vz = work->wind.vz * 0.98f + local_wind.vz * 0.02f;



    /* スプライト更新 */
    for ( i = 0 ; i < N_SPRTS ; i++ ) {	

	if ( uvrgbwh_pre->a > 0 ) {

	    DG_COPY_VEC( pos, pos_pre );

	    work->radius[i] += work->radius[i] / 60.0f * ( 1.0f + frnd()* 0.20f );
	    uvrgbwh->w = work->radius[i] * work->cos[i];
	    uvrgbwh->h = work->radius[i] * work->sin[i];	    

	    uvrgbwh->r = uvrgbwh_pre->r;
	    uvrgbwh->g = uvrgbwh_pre->g;
	    uvrgbwh->b = uvrgbwh_pre->b;

	    if (clock) {
		uvrgbwh->a = uvrgbwh_pre->a - 1;
	    }
	    else {
		uvrgbwh->a = uvrgbwh_pre->a;
	    }

	}
	else {
	    uvrgbwh->w = 0;
	    uvrgbwh->h = 0;
	    uvrgbwh->a = 0;
	}

	pos++;
	pos_pre++;
	uvrgbwh++;
	uvrgbwh_pre++;
		
    }


    /* 前の煙との直線距離から発生させる煙の数を算出 */
    if ( work->id < N_SPRTS && !work->water_flag ) {

	DG_COPY_VEC( &vectmp, work->pos );
	if ( vectmp.vy < GM_WaterLevel ) {
	    vectmp.vy = GM_WaterLevel;
	    work->water_flag = 1;
	}
	_sceVu0SubVector( &vec, &vectmp, &work->prevpos );
	len = bp_sqrtf( _sceVu0InnerProduct( &vec, &vec ) );  //BP_MATH - emulate PS2 sqrtf

	new = (int)( len / ( work->size / 3.f ) );
	new += 1;
	ftmp = 1.0f / (float)new;
	_sceVu0ScaleVector( &unit_vec, &vec, ftmp );


	/* 算出した個数分煙生成 */
	pos     = work->prim->pos[clock];
	uvrgbwh = work->prim->uvrgb[clock];
	i = 0;
	while ( ( i < new ) && ( work->id < N_SPRTS ) ) {

	    /* 補間するように初期位置設定 */
	    _sceVu0ScaleVector( &vectmp, &unit_vec, (float)(i+1) );
	    _sceVu0AddVector( &pos[work->id], &work->prevpos, &vectmp );

	    /* 位置ちょっとランダム */
	    pos[work->id].vx += work->size / 4.f * frnd();
	    pos[work->id].vy += work->size / 4.f * frnd();
	    pos[work->id].vz += work->size / 4.f * frnd();

	    /* サイズ,回転初期化 */
	    work->radius[work->id] = work->size * (1.0f + frnd()*0.20f);
	    angle = TPI * frnd();
	    work->cos[work->id] = vu0_Cos( angle );
	    work->sin[work->id] = vu0_Sin( angle );
	    uvrgbwh[work->id].w  = work->radius[work->id] * work->cos[work->id];
	    uvrgbwh[work->id].h  = work->radius[work->id] * work->sin[work->id];
	    
	    uvrgbwh[work->id].r = COLOR + COLOR * (N_SPRTS-work->id) / N_SPRTS;
	    uvrgbwh[work->id].g = COLOR + COLOR * (N_SPRTS-work->id) / N_SPRTS;
	    uvrgbwh[work->id].b = COLOR + COLOR * (N_SPRTS-work->id) / N_SPRTS;	
	    uvrgbwh[work->id].a = ALPHA + irnd()%10;

	    i++;
	    work->id++; 
	    
	}

	DG_COPY_VEC( &work->prevpos, work->pos );
    
	work->size *= 0.90f;

    }

    
    if ( ++work->cnt >= LIFE ) {
	GV_DestroyActor( work );
    }

    
}



static void Die( Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
}



static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{

	int			i;
	FVECTOR			*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;


	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );


	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;


	for ( i = 0 ; i < N_SPRTS ; i++ ){

	    DG_COPY_VEC( pos, &DG_ZeroVector );

	    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    uvrgbwh->r  = COLOR;
	    uvrgbwh->g  = COLOR;
	    uvrgbwh->b  = COLOR;
	    uvrgbwh->a  = 0;

	    uvrgbwh->w  = 0;
	    uvrgbwh->h  = 0;

	    pos++;
	    uvrgbwh++;

	}	

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_SPRTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_SPRTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_SPRTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_SPRTS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources( Work *work, FVECTOR* pos, float size )
{

    DG_PRIM2	*prim;
    DG_TEX	*tex;


    work->pos = pos;
    DG_COPY_VEC( &work->prevpos, pos );
    work->size = size;
    work->id   = 0;    

    work->cnt  = 0;

    work->water_flag = 0;
    
    DG_COPY_VEC( &work->wind, &DG_ZeroVector );
    
    prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
    if ( prim == NULL ) {
	return -1;
    }

    tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );
    InitPacket( work, prim, tex );    

    return 0;

}



void *NewRunningSmoke2( FVECTOR* pos, float size )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );
	//work = (Work *)GV_NewEffectPrio( GV_ACTOR_EFFECT, sizeof( Work ), 0 );

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


