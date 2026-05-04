//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  test_spiral2.c
  螺旋テスト
  2001/03/29 Yuuta Kunibe
	
  $Id: body_smoke.c,v 1.4 2002/11/23 12:16:42 Yoshizawa1 Exp $
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
#define COL_B	(128)
#define ALPHA	(32)

#define N_PRIMS	(3)
#define N_VERTS (64)

#define	RATE	(0.98f)

#define SCR_POS SCRPAD_ADDR
#define SCR_UVS (SCRPAD_ADDR+0x2000)

#define LIFE	(200)


/* グローバル風パラメータ */
extern FVECTOR G_wind;
extern SVECTOR G_wind_rot;
extern int G_wind_intense;
extern int G_wind_intense_max;
extern FMATRIX G_wind_matrix;


typedef	struct	{

    GV_ACT_EX	actor;
    int		name;
    int		map;

    DG_PRIM2	*prim;

    int 	life;

    float	u[N_PRIMS*N_VERTS];

    float	u_scale;
    float	u_offset;
    float	add_u;

    FVECTOR	wind;

} Work;


/* アクト関数 */
static void Act( Work *work )
{

    int 		i,j;
    int	  		clock;
    DG_PRIM2_UVRGB	*uvrgb;
    float		intense;
    FVECTOR		local_wind;
    FVECTOR		world_y;

    /* 風計算 */
    intense = 1.f / (float)G_wind_intense_max;
    _sceVu0ScaleVector( &local_wind, &G_wind, intense );
    local_wind.vy = 0.f;
    work->wind.vx = work->wind.vx * 0.98f + local_wind.vx * 0.02f;
    work->wind.vy = work->wind.vy * 0.98f + local_wind.vy * 0.02f;
    work->wind.vz = work->wind.vz * 0.98f + local_wind.vz * 0.02f;

    /* 風による軸傾け (最大傾き45°) */
    DG_COPY_VEC( &world_y, &DG_ZeroVector );
    world_y.vy = 1.0f;
    _sceVu0AddVector( &world_y, &world_y, &work->wind );
    _sceVu0Normalize( &world_y, &world_y );
    work->prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
    DG_COPY_VEC( (FVECTOR *)(&work->prim->as_world.m[1]), &world_y );    


    /* 操作する頂点バッファ取得 */
    DG_SwitchBuffPrim2( work->prim );
    clock = work->prim->buffer_clock;
    uvrgb     = work->prim->uvrgb[clock];


    /* テクスチャスクロール */
    for ( i = 0 ; i < N_PRIMS; i++ ) {	    	    
	for ( j = 0 ; j < N_VERTS ; j++ ){
	    work->u[i*N_VERTS+j] -= work->add_u;
	    if ( work->u[i*N_VERTS+j] < work->u_offset ) {
		work->u[i*N_VERTS+j] += work->u_scale;
	    }
	    uvrgb->u = FTOI12( work->u[i*N_VERTS+j] );

	    uvrgb++;
	}
    }

}


static void Die(Work *work )
{
    /* メモリ解放 */
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
	float		alpha;
	FVECTOR		*pos;	
	DG_PRIM2_UVRGB	*uvrgb;



	prim->raise = 0;

	DG_ConfigPrim2Tex( prim, tex );
	if ( flag ) {
	    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}
	else {
	    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}



	/* テクスチャスクロールパラメータ取得 */
	work->u_offset = tex->u_offset;
	work->u_scale  = tex->u_scale;
	work->add_u = work->u_scale * ( 0.8f + 0.5f *frnd() ) / (float)( N_PRIMS * N_VERTS / 2 );
	
	/* 回転角度初期化 */
	rot.vx = 0;
	rot.vz = 0;
	rot.vy = 0;

	/* 螺旋中心マトリクス初期化 */
	DG_SetPos( &DG_UnitMatrix );
	rot.vy = irnd()%4096;
	DG_RotatePos( &rot );
	DG_GetPos( &mat1 );

	DG_SetPos( &DG_UnitMatrix );
	rot.vy = irnd()%4096;
	DG_RotatePos( &rot );
	DG_GetPos( &mat2 );

	/* 螺旋各加速度初期化 */
	frad1 = 128.f + frnd() * 64.f;
	frad2 = frad1 / 2.f;

	/* 螺旋中心位置初期化 */	
	DG_COPY_VEC( &center, position );
	center.vy -= 1000.f;
	center.vx += frnd() * 500.f;
	center.vz += frnd() * 500.f;

	/* その他各パラメータ初期化 */
	//radius = 800.f + frnd() * 200.f;
	radius = size + frnd() * ( size / 4.f );
	alpha  = ALPHA;
	height = 0.f;
	//add_y  = 50.f + frnd() * 25.f;
	add_y  = 25.f + frnd() * 10.f;

	pos   	= SCR_POS;
	uvrgb	= SCR_UVS;
	
	for ( i = 0 ; i < N_PRIMS; i++ ) {	    
	    
	    for ( j = 0 ; j < N_VERTS/2 ; j++ ){

		/* ポリゴンの繋ぎ部分 */
		if ( i != 0 && j == 0 ) {

		    DG_COPY_VEC( pos, ( pos-2 ) );
		    pos++;
		    DG_COPY_VEC( pos, ( pos-2 ) );
		    pos++;

		}
		else {		

		    radius *= RATE;
		    height += add_y;
		    
		    /* 螺旋１の頂点算出 */
		    DG_SetPos( &mat1 );
		    rot.vy = (int)( 4096.f / frad1 );
		    DG_RotatePos( &rot );
		    DG_GetPos( &mat1 );

		    _sceVu0ScaleVector( &vectmp, (FVECTOR *)(&mat1.m[2]), radius );
		    _sceVu0AddVector( pos, &center, &vectmp );

		    _sceVu0ScaleVector( &vectmp, (FVECTOR *)(&mat1.m[1]), height );
		    _sceVu0AddVector( pos, pos, &vectmp );
	    
		    pos++;

		    height += add_y;

		    /* 螺旋２の頂点算出 */
		    DG_SetPos( &mat2 );
		    rot.vy = (int)( 4096.f / frad2 );
		    DG_RotatePos( &rot );
		    DG_GetPos( &mat2 );

		    _sceVu0ScaleVector( &vectmp, (FVECTOR *)(&mat2.m[0]), radius );
		    _sceVu0AddVector( pos, &center, &vectmp );

		    _sceVu0ScaleVector( &vectmp, (FVECTOR *)(&mat2.m[1]), height );
		    _sceVu0AddVector( pos, pos, &vectmp );	    
	    
		    pos++;

		}

		alpha *= RATE;

		work->u[ i*N_VERTS + j*2 ]
		    = (float)( i*N_VERTS/2 + j ) / (float)( N_PRIMS*N_VERTS/2 ) * tex->u_scale + tex->u_offset;
		uvrgb->u = FTOI12( work->u[ i*N_VERTS + j*2 ] );
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset );
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = COL_R;
		uvrgb->g = COL_G;
		uvrgb->b = COL_B;
		uvrgb->a = (u_short)alpha;

		uvrgb++;

		work->u[ i*N_VERTS + j*2 + 1 ]
		    = (float)( i*N_VERTS/2 + j ) / (float)( N_PRIMS*N_VERTS/2 ) * tex->u_scale + tex->u_offset;
		uvrgb->u = FTOI12( work->u[ i*N_VERTS + j*2 + 1 ] );
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset );
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = COL_R;
		uvrgb->g = COL_G;
		uvrgb->b = COL_B;
		uvrgb->a = (u_short)alpha;

		uvrgb++;

	    }
	    
	}

	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1 | DG_PRIM2_INVISIBLE2 | DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources( Work *work )
{
    
    DG_PRIM2		*prim;
    DG_TEX		*tex;
    float		size;
    FVECTOR		position;
    int			flag;
    

    size = 200.f;
    DG_COPY_VEC( &position, &GM_PlayerPosition );
    flag = 0;

    work->life = LIFE;
    DG_COPY_VEC( &work->wind, &DG_ZeroVector );

    prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY | DG_PRIM2_ALPHA | DG_PRIM2_TEX, N_PRIMS, N_VERTS );

    tex = DG_GetTexture( GV_StrCode( "smoke_lp2_alp" ) );
    InitPacket( work, size, &position, flag, prim, tex );
    
    return 0;

}


/*
  NewTestSpiral2 :
  
*/  
void *NewBodySmoke(void)
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



