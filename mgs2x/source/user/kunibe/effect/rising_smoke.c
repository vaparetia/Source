//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
    rising_smoke.c
    立ち昇る煙
    2001/03/28 Yuuta Kunibe	
    $Id: rising_smoke.c,v 1.1.1.3 2002/11/19 11:44:47 Yoshizawa1 Exp $
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


#define N_PRIMS (36)

#define COLOR	(64)//(128)
#define ALPHA	(40)
#define SIZE	(100.0f)

#define	LIFE	(360)

#define SCR_POS (SCRPAD_ADDR)
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define WIND_MAX (20.f)

#define	SKIP_DIST2	(6000.0f*6000.0f)



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
    FVECTOR	position;

    FVECTOR	vec[N_PRIMS];
    float	radius[N_PRIMS];
    float	cos[N_PRIMS];
    float	sin[N_PRIMS];

    float	size;
    float	initsize;
    float	rising_spd;
    FVECTOR	wind;

    int  	id;
    int 	life;
    int		cnt;

} Work;



/* アクト関数 */
static void ActPrim( Work *work )
{

    int			i,j;
    int 		n;
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
    FVECTOR		prevpos;
    FVECTOR		unit_vec;
    FVECTOR		local_wind;
    int			alpha;
    

    /* 操作する頂点バッファ取得 */
    // AS_HALFSPEED_PARTICLE
    //AR_PARTICLE_HALF
    if (!DG_SwitchBuffPrim2( work->prim ))
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


    n = work->id - 1;
    if ( n < 0 ) n = N_PRIMS - 1;


    /* スプライト更新 */
    for ( i = 0 ; i < N_PRIMS ; i++ ) {	

	j = work->id-1 - i;
	if ( j < 0 ) j += N_PRIMS;

	if ( j < 12 ) {
	    alpha = ALPHA * j / 12;
	}
	else { 
	    alpha = ALPHA * ( N_PRIMS - j ) / N_PRIMS;
	}
	
	ftmp = 1.0f + frnd() * 0.2f;
	_sceVu0ScaleVector( &vectmp, &work->wind, ftmp );
	_sceVu0AddVector( &pos[i], &pos_pre[i], &vectmp );

	//pos[i].vy += work->rising_spd;

	_sceVu0AddVector( &pos[i], &pos[i], &work->vec[i] );
	work->vec[i].vx *= 0.98f;
	work->vec[i].vz *= 0.98f;

	
	uvrgbwh[i].a = alpha;
	if ( uvrgbwh[i].a > 0 ) {
	ftmp = work->radius[i] * (float)( N_PRIMS - j ) / (float)N_PRIMS + work->radius[i] * 2.0f * (float)j / (float)N_PRIMS;//1.5f
	uvrgbwh[i].w = ftmp * work->cos[i];//uvrgbwh_pre[i].w * 1.02f;
	uvrgbwh[i].h = ftmp * work->sin[i];//uvrgbwh_pre[i].h * 1.02f;
	}
	else {
	uvrgbwh[i].w = 0;
	uvrgbwh[i].h = 0;
	}
	/*if ( !( work->cnt % 3 ) ) {
	    if ( uvrgbwh_pre[i].a > 0 ) {
		uvrgbwh[i].a = uvrgbwh_pre[i].a - 1;
	    }
	    else {
		uvrgbwh[i].w = 0.f;
		uvrgbwh[i].h = 0.f;
		uvrgbwh[i].a = 0;
	    }
	}
	else {
	    uvrgbwh[i].a = uvrgbwh_pre[i].a;
	}*/
		
	if ( i == n ) DG_COPY_VEC( &prevpos, &pos[i] );

    }



    /* 前の煙との直線距離から発生させる煙の数を算出 */
    len = bp_sqrtf( ( prevpos.vx - work->pos->vx ) * ( prevpos.vx - work->pos->vx ) //BP_MATH - emulate PS2 sqrtf
		+ ( prevpos.vy - work->pos->vy ) * ( prevpos.vy - work->pos->vy )
		+ ( prevpos.vz - work->pos->vz ) * ( prevpos.vz - work->pos->vz ) );

    n = (int)( len / ( work->size / 3.f ) );
    //n += 1;

    if ( n ) {
    
    ftmp = 1.0f / (float)n;
    _sceVu0SubVector( &unit_vec, work->pos, &prevpos );
    _sceVu0ScaleVector( &unit_vec, &unit_vec, ftmp );

    
    work->initsize = work->initsize * ( 1.f + frnd()*0.05f );

    if (!(irnd()%120)) work->initsize = work->size * 1.25f;
    
    if ( work->initsize > work->size * 1.25f ) {
	work->initsize = work->size * 1.25f;
    }
    else if ( work->initsize < work->size * 0.85f ) {
	work->initsize = work->size * 0.85f;
    }

    
      
    /* 算出した個数分煙生成 */
    for ( i = 0 ; i < n ; i++ ) {
	DG_COPY_VEC( &pos[work->id], &prevpos );

	_sceVu0ScaleVector( &vectmp, &unit_vec, (float)(i+1) );
	_sceVu0AddVector( &pos[work->id], &pos[work->id], &vectmp );
	pos[work->id].vx += work->size / 4.f * frnd();
	pos[work->id].vy += work->size / 4.f * frnd();
	pos[work->id].vz += work->size / 4.f * frnd();

	work->radius[work->id] = work->initsize * ( 1.0f + frnd() * 0.20f );
	angle = TPI * frnd();
	work->cos[work->id] = vu0_Cos( angle );
	work->sin[work->id] = vu0_Sin( angle );
	uvrgbwh[work->id].w  = work->radius[work->id] * work->cos[work->id];
	uvrgbwh[work->id].h  = work->radius[work->id] * work->sin[work->id];
	
	uvrgbwh[work->id].a = 0;//ALPHA + irnd()%10 - 5;

	work->vec[work->id].vy = work->rising_spd;	
	work->vec[work->id].vx = work->rising_spd * frnd()*0.2f;	
	work->vec[work->id].vz = work->rising_spd * frnd()*0.2f;	
	
	if ( ++work->id >= N_PRIMS ) work->id = 0;
    }

    }

    work->cnt++;

}


static void Act( Work *work )
{

    FVECTOR	m_pos;
    float	temp_x,temp_y;
    FVECTOR	vec;
    float	len2;

    {

	/* 透視変換した座標 */
	DG_SetPos( &DG_Chanls[0].eye_pers );
	DG_PutVector( &work->position, &m_pos, 1 );

	temp_x = fpu_Abs( m_pos.vx / m_pos.vw );
	temp_y = fpu_Abs( m_pos.vy / m_pos.vw );

	/* 炎とカメラとの平面距離算出 */
	_sceVu0SubVector( &vec, (FVECTOR *)DG_Chanls[0].eye.m[3], &work->position );
	len2 = vec.vx * vec.vx + vec.vz *vec.vz;

	/* 平面距離と透視変換座標からクリッピング処理 */
	if ( len2 > SKIP_DIST2 && ( m_pos.vz >= m_pos.vw ) ) {
	    DG_InvisiblePrim2( work->prim );
	    return ;
	}

    }

    
    DG_VisiblePrim2( work->prim );
    ActPrim( work );
    
}


static void Die( Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
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
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

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

	    ftmp = COLOR * ( frnd() + 1.f );
	    uvrgbwh->r  = ftmp;
	    uvrgbwh->g  = ftmp;
	    uvrgbwh->b  = ftmp;
	    uvrgbwh->a  = 0;

	    ftmp  = work->size * ( 1.20f + frnd() * 0.70f );
	    angle = TPI * frnd();
	    uvrgbwh->w  = 0;//ftmp * vu0_Cos( angle );
	    uvrgbwh->h  = 0;//ftmp * vu0_Sin( angle );

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



static int GetResources( Work *work, FVECTOR* pos, float size )
{

    DG_PRIM2	*prim;
    DG_TEX	*tex;

    work->life  = LIFE;
    work->cnt = 0;

    work->pos = pos;
    work->size = size;
    work->initsize = size;
    work->rising_spd = 20.f * size / 300.f * 0.7f;//15.f// 400.f
    work->id  = 0;    

    DG_COPY_VEC( &work->wind, &DG_ZeroVector );
    
    prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
									  N_PRIMS/4, 4 );
	if ( prim == NULL ) {
		return -1;
	}

    tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );
    InitPacket( work, prim, tex );    

    return 0;

}

static int GetResourcesScn( Work *work )
{

    DG_PRIM2	*prim;
    DG_TEX	*tex;

    work->life  = LIFE;
    work->cnt = 0;

    // 位置の取得    
    GCL_GetOption( 'p' );                                 
    work->position.vx = ( float )GCL_GetNextInt();	
    work->position.vy = ( float )GCL_GetNextInt();	
    work->position.vz = ( float )GCL_GetNextInt();
    work->position.vw = 1.0f;
#if 0    
    printf("pos : vx %f, vy %f, vz %f\n", work->position.vx, work->position.vy, work->position.vz );
#endif    
    
    work->pos = &work->position;

    work->size = (float)( GCL_GetOptionValue( 'r', 0 ) );
#if 0    
    printf("size : %f\n",work->size);
#endif    
    work->rising_spd = 15.f * work->size / 100.f * 0.5f;
    work->id  = 0;    

    DG_COPY_VEC( &work->wind, &DG_ZeroVector );
    
    prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS/4, 4 );
	if ( prim == NULL ) {
		return -1;
	}

    tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );
    InitPacket( work, prim, tex );    

    return 0;

}

/*
  NewRisingSmoke
      FVECTOR* pos : 発生位置
*/
void *NewRisingSmoke( FVECTOR* pos, float size )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	printf("new\n");

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


void *NewRisingSmokeScn( int name, int where )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResourcesScn( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



