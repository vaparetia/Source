//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  test_fire2.c
  炎テスト
  2001/04/19 Yuuta Kunibe	
  $Id: test_fire2.c,v 1.4 2002/11/23 12:36:05 Yoshizawa1 Exp $
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
#define COL_G	(48)
#define COL_B	(0)
#define ALPHA	(128)//(64)

#define N_PRIMS	(5)
#define N_VERTS (64)

#define	RATE	(0.95f)

#define SCR_POS SCRPAD_ADDR
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define LIFE	(120)


/* グローバル風パラメータ */
extern FVECTOR G_wind;
extern SVECTOR G_wind_rot;
extern int G_wind_intense;
extern int G_wind_intense_max;
extern FMATRIX G_wind_matrix;


typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;
    DG_TEX	*tex;

    int 	life;

    float	u[N_PRIMS*N_VERTS];

    FVECTOR	center[N_PRIMS*N_VERTS];
    FVECTOR	offset_pos[N_PRIMS*N_VERTS];
    
    float	u_scale;
    float	u_offset;
    float	add_u[N_PRIMS];

    float	rad[N_PRIMS];
    float	add_rad[N_PRIMS];
    float	swing_h[N_PRIMS];
    float	alpha[N_PRIMS*N_VERTS];

    float	frad2[N_PRIMS];
    float	add_rad2[N_PRIMS];
    float	swing_w[N_PRIMS];    

    float	add_height;

    float	param;
    
    FVECTOR	wind;

} Work;


void *NewTestFire2( DG_TEX *tex, float add_height );


/* アクト関数 */
static void Act( Work *work )
{

    int 		i,j;
    int	  		clock;
    FVECTOR		*pos;
    DG_PRIM2_UVRGB	*uvrgb;
    FVECTOR		vectmp;
    FVECTOR		vectmp2;
    float		param;
    float		param2;
    

    /* 風計算 */
    /*intense = 1.f / (float)G_wind_intense_max;
    _sceVu0ScaleVector( &local_wind, &G_wind, intense );
    local_wind.vy = 0.f;
    work->wind.vx = work->wind.vx * 0.98f + local_wind.vx * 0.02f;
    work->wind.vy = work->wind.vy * 0.98f + local_wind.vy * 0.02f;
    work->wind.vz = work->wind.vz * 0.98f + local_wind.vz * 0.02f;*/

    /* 風による軸傾け (最大傾き45°) */
    /*DG_COPY_VEC( &world_y, &DG_ZeroVector );
    world_y.vy = 1.0f;
    _sceVu0AddVector( &world_y, &world_y, &work->wind );
    _sceVu0Normalize( &world_y, &world_y );
    DG_COPY_VEC( (FVECTOR *)(&work->prim->world.m[1]), &world_y );    */


    work->life++;

    
    /* 操作する頂点バッファ取得 */
    DG_SwitchBuffPrim2( work->prim );
    clock = work->prim->buffer_clock;
    pos	      = work->prim->pos[clock];
    uvrgb     = work->prim->uvrgb[clock];

    for ( i = 0 ; i < N_PRIMS; i++ ) {	    	    

	work->rad[i] += work->add_rad[i];
	if ( work->rad[i] > TPI ) {
	    work->rad[i] -= TPI;
	    work->add_rad[i] = TPI / ( 90.f + frnd()*90.f );
	    work->swing_h[i] = 600.f + 200.f * frnd();
	}
	param = 1.f * vu0_Cos( work->rad[i] );

	work->frad2[i] += work->add_rad2[i];
	if ( work->frad2[i] > TPI ) {
	    work->frad2[i] -= TPI;
	    work->add_rad2[i] = TPI / ( 120.f + frnd()*120.f );
	    work->swing_w[i] = 400.f + 300.f * frnd();
	}
	param2 = 1.f * vu0_Cos( work->rad[i] );

    
	for ( j = 0 ; j < N_VERTS ; j++ ){

	    _sceVu0ScaleVector( &vectmp, &work->offset_pos[i*N_VERTS+j],
				1.f * (float)(N_VERTS-j) / (float)N_VERTS + 2.f * param * (float)j / (float)N_VERTS );

	    DG_COPY_VEC( &vectmp2, &work->center[i*N_VERTS+j] );
	    vectmp2.vx += work->swing_w[i] * param2 * (float)j / (float)N_VERTS;
	    vectmp2.vz += work->swing_w[i] * param2 * (float)j / (float)N_VERTS;
	    _sceVu0AddVector( pos, &vectmp2, &vectmp );

	    pos->vy +=  work->swing_h[i] * param * (float)j / (float)N_VERTS;
	    
	    /* テクスチャスクロール */
	    work->u[i*N_VERTS+j] -= work->add_u[i];
	    if ( work->u[i*N_VERTS+j] < work->u_offset ) {
		work->u[i*N_VERTS+j] += work->u_scale;
		if ( j < N_VERTS-2 ) {
		    (uvrgb+2)->f = 0x8fff;
		}
	    }
	    else {
		if ( j < N_VERTS-2 ) {
		    (uvrgb+2)->f = 0x0fff;
		}
	    }
	    uvrgb->u = FTOI12( work->u[i*N_VERTS+j] );

	    uvrgb->a = work->alpha[i*N_VERTS+j] * work->param;

	    pos++;
	    uvrgb++;

	}
    }


    if ( work->life < 60 ) work->param = work->param * 0.98f + 0.02f;
    else if ( work->life > 60 ) work->param = work->param * 0.98f;

    if ( ++work->life >= LIFE ) {
	GV_DestroyActor( work );
    }

}


static void Die(Work *work )
{
    /* メモリ解放 */
    NewTestFire2( work->tex, work->add_height );    
    work->prim = OK_FreePrim2( work->prim );
}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work, float size, FVECTOR* position, int flag, DG_PRIM2 *prim, DG_TEX *tex )
{
	int		i,j;
	FMATRIX		mat1;
	FMATRIX		mat2;
	float		frad1;
	float		frad2;
	SVECTOR		rot;
	FVECTOR		vectmp;
	FVECTOR		center;
	float		height;
	float		add_y;
	float		radius;
	float		g;
	float		alpha;
	FVECTOR		*pos;	
	DG_PRIM2_UVRGB	*uvrgb;


	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	if ( flag ) {
	    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}
	else {
	    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	}

	pos   	= SCR_POS;
	uvrgb	= SCR_UVS;		   


	/* テクスチャスクロールパラメータ取得 */
	work->u_offset = tex->u_offset;
	work->u_scale  = tex->u_scale;


	for ( i = 0 ; i < N_PRIMS; i++ ) {	    


	    work->add_u[i] = work->u_scale * ( 1.5f + 0.5f *frnd() ) / (float)( N_PRIMS * N_VERTS / 2 );

	    work->rad[i] = TPI*frnd();
	    work->add_rad[i] = TPI / ( 50.f + frnd()*40.f );//( 75.f + frnd()*25.f );
	    work->swing_h[i] = 600.f + 200.f * frnd();

	    work->frad2[i] = TPI*frnd();
	    work->add_rad2[i] = TPI / ( 25.f + frnd()*10.f );
	    work->swing_w[i] = 400.f + 300.f * frnd();

	
	    /* 回転角度初期化 */
	    rot.vx = 0;
	    rot.vz = 0;
	    rot.vy = 0;


	    /* 螺旋中心マトリクス初期化 */
	    DG_SetPos( &DG_UnitMatrix );
	    rot.vx = irnd()%512;
	    rot.vy = 2048 + 512 + irnd()%1024;
	    DG_RotatePos( &rot );
	    DG_GetPos( &mat1 );

	    DG_SetPos( &DG_UnitMatrix );
	    rot.vx = irnd()%512;
	    rot.vy = 2048 + 512 + irnd()%1024;
	    DG_RotatePos( &rot );
	    DG_GetPos( &mat2 );


	    rot.vx = 0;
	    rot.vz = 0;
	    rot.vy = 0;

	/* 螺旋各加速度初期化 */
	//frad1 = ( 48.f + frnd() * 16.f );
	frad1 = ( 96.f + frnd() * 32.f );
	frad2 = frad1 / 2.f;
	/*frad1 = 128.f + frnd() * 64.f;
	frad2 = frad1 / ( frnd()*0.5f+2.f );*/

	/* 螺旋中心位置初期化 */	
	DG_COPY_VEC( &center, position );
	center.vy -= 1000.f;
	center.vx += 3000.f * frnd();
	center.vz += 300.f * frnd();

	/* その他各パラメータ初期化 */
	radius = size + frnd() * ( size / 2.f );
	alpha  = ALPHA;
	height = 0.f;
	//add_y  = 50.f + frnd() * 10.f;
	add_y  = work->add_height + frnd() * work->add_height / 3.f;
	g = (float)COL_G + (float)COL_G * (0.5f + frnd()* 0.5f);

	
	for ( j = 0 ; j < N_VERTS/2 ; j++ ){

	    
	    radius *= 1.f - (float)j*0.004f;
	    height += add_y;

	    
	    /*DG_COPY_VEC( &work->center[ i*N_VERTS + j*2 ], &center );
	    work->center[ i*N_VERTS + j*2 ].vy += height;*/
	    _sceVu0ScaleVector( &vectmp, (FVECTOR *)(&mat1.m[1]), height );
	    _sceVu0AddVector( &work->center[ i*N_VERTS + j*2 ], &center, &vectmp );
	    
	    /* 螺旋１の頂点算出 */
	    DG_SetPos( &mat1 );
	    rot.vy = (int)( 4096.f / frad1 * (float)j / 32.f );
	    DG_RotatePos( &rot );
	    DG_GetPos( &mat1 );
	    
	    _sceVu0ScaleVector( &vectmp, (FVECTOR *)(&mat1.m[2]), radius );
	    _sceVu0AddVector( pos, &center, &vectmp );
	    
	    DG_COPY_VEC( &work->offset_pos[ i*N_VERTS + j*2 ], &vectmp );
	    
	    _sceVu0ScaleVector( &vectmp, (FVECTOR *)(&mat1.m[1]), height );
	    _sceVu0AddVector( pos, pos, &vectmp );
	    
	    pos++;
	    
	    height += add_y;
	    
	    /* 螺旋２の頂点算出 */
	    DG_SetPos( &mat2 );
	    rot.vy = (int)( 4096.f / frad2 * (float)j / 32.f );
	    DG_RotatePos( &rot );
	    DG_GetPos( &mat2 );
	    
	    /*DG_COPY_VEC( &work->center[ i*N_VERTS + j*2 + 1 ], &center );
 	    work->center[ i*N_VERTS + j*2 + 1 ].vy += height;*/
	    _sceVu0ScaleVector( &vectmp, (FVECTOR *)(&mat2.m[1]), height );
	    _sceVu0AddVector( &work->center[ i*N_VERTS + j*2 + 1 ], &center, &vectmp );	    

	    _sceVu0ScaleVector( &vectmp, (FVECTOR *)(&mat1.m[1]), height );
	    _sceVu0AddVector( &work->center[ i*N_VERTS + j*2 ], &center, &vectmp );
	    
	    _sceVu0ScaleVector( &vectmp, (FVECTOR *)(&mat2.m[0]), radius );
	    _sceVu0AddVector( pos, &center, &vectmp );
	    
	    DG_COPY_VEC( &work->offset_pos[ i*N_VERTS + j*2 + 1 ], &vectmp );
	    
	    _sceVu0ScaleVector( &vectmp, (FVECTOR *)(&mat2.m[1]), height );
	    _sceVu0AddVector( pos, pos, &vectmp );	    
	    
	    pos++;


	    if ( j < 16 ) {
	    alpha *= 0.92f;//RATE;
	    }
	    else {
	    alpha *= 0.8f;//RATE;
	    }
	    
	    work->u[ i*N_VERTS + j*2 ]
		= (float)( i*N_VERTS/2 + j ) / (float)( N_PRIMS*N_VERTS/2 ) * tex->u_scale + tex->u_offset;
	    uvrgb->u = FTOI12( work->u[ i*N_VERTS + j*2 ] );
	    uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset );
	    uvrgb->q = 4096;
	    uvrgb->f = 0x0fff;
	    uvrgb->r = COL_R;
	    uvrgb->g = g;
	    //uvrgb->g = COL_G;
	    uvrgb->b = COL_B;
	    uvrgb->a = 0;//(u_short)alpha;
	    work->alpha[ i*N_VERTS + j*2 ] = alpha;
	    
	    uvrgb++;
	    
	    work->u[ i*N_VERTS + j*2 + 1 ]
		= (float)( i*N_VERTS/2 + j ) / (float)( N_PRIMS*N_VERTS/2 ) * tex->u_scale + tex->u_offset;
	    uvrgb->u = FTOI12( work->u[ i*N_VERTS + j*2 + 1 ] );
	    uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset );
	    uvrgb->q = 4096;
	    uvrgb->f = 0x0fff;
	    uvrgb->r = COL_R;
	    uvrgb->g = g;
	    //uvrgb->g = COL_G;
	    uvrgb->b = COL_B;
	    uvrgb->a = 0;//(u_short)alpha;
	    work->alpha[ i*N_VERTS + j*2 + 1] = alpha;
	    
	    uvrgb++;

	    g *= 0.995f;
	    
	}
	
	}
	
	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1 | DG_PRIM2_INVISIBLE2 | DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources( Work *work, float add_height, DG_TEX *tex  )
{
    
    DG_PRIM2		*prim;
    //DG_TEX		*tex;
    float		size;
    FVECTOR		position;
    int			flag;
    

    size = 1200.f;//100.f;//1200.f;//800.f;
    DG_COPY_VEC( &position, &GM_PlayerPosition );
    flag = 1;

    work->add_height = add_height;
    work->life = 0;
    work->param = 0.0f;
    DG_COPY_VEC( &work->wind, &DG_ZeroVector );
    work->tex = tex;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY | DG_PRIM2_ALPHA | DG_PRIM2_TEX, N_PRIMS, N_VERTS );

    //tex = DG_GetTexture( GV_StrCode( "blood_2bw_msk" ) );
    //tex = DG_GetTexture( GV_StrCode( "bombgas6_alp" ) );
    //tex = DG_GetTexture( GV_StrCode( "fire3_alp" ) );
    //tex = DG_GetTexture( GV_StrCode( "bombpowder7_msk" ) );
    InitPacket( work, size, &position, flag, prim, tex );
    
    return 0;

}


/*
  NewTestSpiral2 :
  
*/  
void *NewTestFire2( DG_TEX *tex, float add_height )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, add_height, tex ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}





