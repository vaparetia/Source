//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    harrier_clasterunit_smoke.c
    クラスター弾煙
    2001/04/11 Yuuta Kunibe	
    $Id: harrier_clasterunit_smoke.c,v 1.1.1.3 2002/11/19 11:44:40 Yoshizawa1 Exp $
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


#define N_PRIMS		(12)

#define COLOR		(128)
#define ALPHA		(64)//(32)

#define	LIFE		(300)

#define BASE_SIZE	(3000.0f)//(2400.f)//(10000.f)//(2400.f)//(1200.f)
#define BASE_SPEED	(10000.f)//(20000.f)//(10000.f)//(5000.f)

#define SCR_POS 	(SCRPAD_ADDR)
#define SCR_UVS 	(SCRPAD_ADDR+0x2000)

#define WIND_MAX 	(20.f)


/* グローバル風パラメータ */
extern FVECTOR G_wind;
extern SVECTOR G_wind_rot;
extern int G_wind_intense;
extern int G_wind_intense_max;
extern FMATRIX G_wind_matrix;

extern void *NewDebris_Cm( FVECTOR *bound, FVECTOR *force,
					int num, int objcode,
					int color, float scale, int flags );
extern void *NewHarrierClasterSpark( FVECTOR *pos );



typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FVECTOR	vec[N_PRIMS];
    FVECTOR	wind;

    int		alpha_time;
    int		cnt;

} Work;


/* アクト関数 */
static void Act( Work *work )
{

    int		i;
    int	  	clock;
    int		alpha_cnt;
    FVECTOR  		*pos;
    FVECTOR  		*pos_pre;
    DG_PRIM2_UVRGBWH 	*uvrgbwh;
    DG_PRIM2_UVRGBWH 	*uvrgbwh_pre;
    FVECTOR		*vec;
    FVECTOR		local_wind;
    float		mov_z;
    float		intense;
    FVECTOR		vectmp;


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
    vec 	= work->vec;

    alpha_cnt = 0;


    /* 風計算 */
    intense = WIND_MAX / (float)G_wind_intense_max;
    _sceVu0ScaleVector( &local_wind, &G_wind, intense );
    local_wind.vy = 0.f;
    work->wind.vx = work->wind.vx * 0.98f + local_wind.vx * 0.02f;
    work->wind.vy = work->wind.vy * 0.98f + local_wind.vy * 0.02f;
    work->wind.vz = work->wind.vz * 0.98f + local_wind.vz * 0.02f;


    /* スプライト更新 */
    for ( i = 0 ; i < N_PRIMS ; i++ ) {

	_sceVu0AddVector( pos, pos_pre, vec );
	mov_z = (float)i / (float)N_PRIMS * ( 1.0f + 0.15f * frnd() );
	_sceVu0ScaleVector( &vectmp, &work->wind, mov_z );	
	_sceVu0AddVector( pos, pos, &vectmp );

	if ( work->cnt < 1 ) {
	    //_sceVu0ScaleVector( vec, vec, 0.018f );
	    //vec->vy = vec->vy * ( 0.018f - (float)N_PRIMS * 0.0005f );
	    //vec->vy = vec->vy * ( 0.018f - (float)N_PRIMS * 0.0005f );
	    _sceVu0ScaleVector( vec, vec, ( 0.018f - (float)N_PRIMS * 0.0005f ) );
	}
	else {
	    _sceVu0ScaleVector( vec, vec, 0.96f );
	}	    


	if ( work->cnt >= work->alpha_time ) {
	    if ( !(work->cnt%2) ) {
		if ( uvrgbwh_pre->a > 0 ) {
		    uvrgbwh->a = uvrgbwh_pre->a - 1;	    
		}
		else {
		    uvrgbwh->w = 0;
		    uvrgbwh->h = 0;
		    uvrgbwh->a = 0;
		    alpha_cnt++;
		}
	    }
	    else {
		uvrgbwh->a = uvrgbwh_pre->a;
	    }
	}

	uvrgbwh->w = (short)( (float)uvrgbwh_pre->w * 1.002f );
	uvrgbwh->h = (short)( (float)uvrgbwh_pre->h * 1.002f );
	    	
	/* ポインタ更新 */
	pos++;
	pos_pre++;
	uvrgbwh++;
	uvrgbwh_pre++;
	vec++;
    }

    if ( ++work->cnt >= LIFE || alpha_cnt == N_PRIMS ) {
#if 0	
	printf("cnt : %d, alpha_cnt %d\n",work->cnt, alpha_cnt);
#endif	
	GV_DestroyActor( work );
    }
    
}


static void Die( Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work, FVECTOR *position, DG_PRIM2 *prim, DG_TEX *tex )
{

	int			i;
	FVECTOR			*pos;
	FVECTOR			*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh;

	float			base_size;
	float			size;
	float 			ftmp;
	float			angle;

	FVECTOR			vectmp;
	SVECTOR			rot;

	FVECTOR bound[2];
	FVECTOR force;
	int	num;
	int 	objcode;
	int 	color;
	float	scale;
	int 	flags;
	FVECTOR position2;

	
	
	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	pos       = SCR_POS;
	uvrgbwh   = SCR_UVS;
	vec	  = work->vec;	

	ftmp = ( 0.6f + 0.4f * frnd() );
	base_size = BASE_SIZE * ftmp;

	DG_COPY_VEC( &position2, position );
	position2.vx += frnd() * 5000.f;
	position2.vz += frnd() * 5000.f;

	rot.vx = irnd()%1024 - 512;
	rot.vy = 0;
	rot.vz = irnd()%1024 - 512;
	
	for ( i = 0 ; i < N_PRIMS; i++ ) {

	    //DG_COPY_VEC( pos, &position2 );
	    DG_COPY_VEC( pos, position );
	    pos->vx += frnd() * 50.f;
	    pos->vy += frnd() * 50.f;
	    pos->vz += frnd() * 50.f;

	    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q0 = 4096;
	    uvrgbwh->f0 = 0x0fff;

	    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
	    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
	    uvrgbwh->q1 = 4096;
	    uvrgbwh->f1 = 0x0fff;

	    ftmp = 1.5f + frnd()* 0.5f;
	    uvrgbwh->r  = COLOR*ftmp;
	    uvrgbwh->g  = COLOR*ftmp;
	    uvrgbwh->b  = COLOR*ftmp;
	    uvrgbwh->a  = ALPHA + irnd()%5 - 2;

	    angle  = frnd() * TPI;
	    ftmp = 0.6f + 0.4f * frnd();
	    size = base_size / 4.f + base_size * ftmp; 
	    uvrgbwh->w  = (short)( size * vu0_Cos( angle ) );
	    uvrgbwh->h  = (short)( size * vu0_Sin( angle ) );

	    ftmp = ( 1.0f - ftmp ) * BASE_SPEED * base_size / BASE_SIZE;
	    DG_COPY_VEC( &vectmp, &DG_ZeroVector );
	    vectmp.vy = ftmp + 500.f;
	    vectmp.vx = ftmp * 0.04f * frnd() * 0.2f;
	    vectmp.vz = ftmp * 0.04f * frnd() * 0.2f;
	    /*_sceVu0ScaleVector( vec, &vectmp, ftmp );
	    vec->vx += 500.f + ftmp * 0.04f * frnd() * 0.2f;
	    vec->vx += ftmp * 0.04f * frnd() * 0.2f;
	    vec->vz += ftmp * 0.04f * frnd() * 0.2f;*/

	    DG_SetPos( &DG_UnitMatrix );
	    DG_RotatePos( &rot );
	    DG_RotVector( &vectmp, vec, 1 );

	    
	    pos++;
	    uvrgbwh++;
	    vec++;

	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3 );




	/* 柴田君破片コモデル呼び出し */
	if ( !(irnd()%5) ) {

	    DG_COPY_VEC( &bound[0], position );
	    bound[0].vx -= base_size;
	    bound[0].vz -= base_size;

	    DG_COPY_VEC( &bound[1], position );
	    bound[1].vx += base_size;
	    bound[1].vy += BASE_SPEED * base_size / BASE_SIZE * 1.2f;
	    bound[1].vz += base_size;

	    force.vx = 0.f;
	    force.vy = 1.f;
	    force.vz = 0.f;
	    force.vw = BASE_SPEED * base_size / BASE_SIZE / 60.f;

	    num = (int)( 20.f * base_size / BASE_SIZE );
	    
	    objcode = GV_StrCode("cgr_frg1_cm");

	    color = ( (128<<24)&(128<<16)&(128<<8) );

	    scale = 20.0f + frnd()*10.0f;

	    flags = 0;
	
	    NewDebris_Cm( bound, &force, num, objcode, color, scale, flags );
	}


	return 1;



}


static int GetResources( Work *work, FVECTOR *position )
{
    
    DG_PRIM2	*prim;
    DG_TEX	*tex;

    work->cnt = 0;
    work->alpha_time = irnd()%120;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, 12 );
    if ( prim == NULL ) {
	return -1;
    }

    tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );
    InitPacket( work, position, prim, tex );    

    NewHarrierClasterSpark ( position ); 
    NewHarrierClasterSpark ( position );

    return 0;

}


void *NewHarrierClasterUnitSmoke( FVECTOR *position )
{

	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, position ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work;

}


