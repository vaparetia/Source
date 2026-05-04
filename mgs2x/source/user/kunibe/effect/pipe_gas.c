//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    pipe_gas.c
    パイプガス
    2001/03/23 Yuuta Kunibe	
    $Id: pipe_gas.c,v 1.1.1.3 2002/11/19 11:44:45 Yoshizawa1 Exp $
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


#define N_PRIMS	(100)

#define COLOR	(128)
#define ALPHA	(8)
#define SIZE	(200.0f)

#define	LIFE	(360)

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

    FVECTOR*	pos;
    FVECTOR*	dir;
    FVECTOR	pre_dir;

    FVECTOR	vec[N_PRIMS];

    FVECTOR	wind;

    int  	id;
    int 	life;
    int		cnt;

} Work;



/* アクト関数 */
static void Act( Work *work )
{

    int			i;
    int	  		clock;

    FVECTOR  		*pos;
    FVECTOR  		*pos_pre;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    float		intense;
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


    for ( i = 0 ; i < N_PRIMS ; i++ ) {
	_sceVu0AddVector( &pos[i], &pos_pre[i], &work->vec[i] );

	/*if ( !(work->cnt%2) ) {	    
	    if ( uvrgbwh_pre[i].a > 0 ) {
		uvrgbwh[i].a = uvrgbwh_pre[i].a - 1;
	    }
	    else {
		uvrgbwh[i].a = 0;
	    }
	}
	else {
	    uvrgbwh[i].a = uvrgbwh_pre[i].a;
	}*/
    }

    DG_COPY_VEC( &pos[work->id], work->pos );

    DG_COPY_VEC( &work->vec[work->id], work->dir );
    _sceVu0ScaleVector( &work->vec[work->id], &work->vec[work->id], 50.f );
    uvrgbwh[work->id].a = ALPHA;
    uvrgbwh_pre[work->id].a = ALPHA;
    
    if ( ++work->id >= N_PRIMS ) work->id = 0;

    work->cnt++;
}


static void Die( Work *work )
{
    /* メモリ解放 */
    work->prim = OK_FreePrim2( work->prim );
}

/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	int			i;
	FVECTOR			*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	float 			ftmp;
	float			angle;

	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;

	for ( i = 0 ; i < N_PRIMS ; i++ ){

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

	    ftmp  = SIZE * ( 1.20f + frnd() * 0.70f );
	    angle = TPI * frnd();
	    uvrgbwh->w  = ftmp * vu0_Cos( angle );
	    uvrgbwh->h  = ftmp * vu0_Sin( angle );

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



static int GetResources( Work *work, FVECTOR* pos, FVECTOR* dir )
{

    DG_PRIM2	*prim;
    DG_TEX	*tex;

    work->life  = LIFE;
    work->cnt = 0;

    work->pos = pos;
    work->dir = dir;
    work->id  = 0;    

    DG_COPY_VEC( &work->wind, &DG_ZeroVector );
    
    prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, 1 );

    tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );
    InitPacket( work, prim, tex );    

    return 0;

}

/*
  NewPipeGas
      FVECTOR* pos : 発生位置
      FVECTOR* dir : 発生方向
*/
void *NewPipeGas( FVECTOR* pos, FVECTOR* dir )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, pos, dir ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}


