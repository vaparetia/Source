//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  poly_plasma.c
  ３点指定ポリゴンプラズマ(位置固定型)
  2001/05/21 Yuuta Kunibe	
  $Id: poly_plasma.c,v 1.1.1.3 2002/11/19 11:44:45 Yoshizawa1 Exp $
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


#define COLOR			(255)
#define ALPHA			(128)

#define N_PRIMS			(2)
#define N_VERTS 		(64)
#define CENTER_NUM 		( N_PRIMS * N_VERTS / 2 )

#define SCR_POS 		(SCRPAD_ADDR)
#define SCR_UVS 		(SCRPAD_ADDR+0x2000)

#define	NODE_NUM		(14)
#define NODE_POINT_NUM		(6)

#define	WAVE_HEIGHT		(20.0f)//(50.0f)//(100.0f)
#define SPEED			(10.0f)//(20.0f)//(50.0f)

#define LIFE			(15)




typedef	struct	{

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FVECTOR	from;
    FVECTOR	middle;
    FVECTOR	to;

    FVECTOR	center[CENTER_NUM];
    FVECTOR	bottom[CENTER_NUM];
    FVECTOR	height[CENTER_NUM];
    FVECTOR	wave[CENTER_NUM];
    FVECTOR	noise[CENTER_NUM];
    FVECTOR	speed[CENTER_NUM];

    int		alpha;
    int		sub_alpha;
    int		life;
    int		cnt;

    float	width;

} Work;


//void *NewPolyPlasmaScn( FVECTOR *from, FVECTOR *middle, FVECTOR *to, float width, FVECTOR *color );
void *NewPolyPlasmaScn( int name, int map );



/* アクト関数 */
static void Act( Work *work )
{

    int 		i;
    int	  		clock;
    FVECTOR		*pos;
    FVECTOR		*pos_pre;
    DG_PRIM2_UVRGB	*uvrgb;
    DG_PRIM2_UVRGB	*uvrgb_pre;
    FVECTOR		vectmp;
    FVECTOR		vec_width;



    GM_GroupPrim2( work->prim, GM_PlayerBody->map_name );

    /* 操作する頂点バッファ取得 */
    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }

    clock = work->prim->buffer_clock;
    pos	      = work->prim->pos[clock];
    pos_pre   = work->prim->pos[1-clock];
    uvrgb     = work->prim->uvrgb[clock];
    uvrgb_pre = work->prim->uvrgb[1-clock];


    /* 始点 */
    _sceVu0AddVector( &work->center[0], &work->from, &work->height[0] );
    _sceVu0AddVector( &work->center[0], &work->center[0], &work->wave[0] );

    _sceVu0ScaleVector( &work->height[0], &work->height[0], 0.97f + frnd()*0.01f );
    _sceVu0ScaleVector( &work->wave[0], &work->wave[0], 0.95f );


    for ( i = 0 ; i < N_PRIMS * N_VERTS / 2 ; i++ ) {

	/* 最終点 */
	if ( i == N_PRIMS * N_VERTS / 2 - 1 ) {		
	    _sceVu0SubVector( pos, &work->to, &vec_width );
	    pos++;
	    _sceVu0AddVector( pos, &work->to, &vec_width );
	    pos++;
	}
	/* 繋ぎ目 */	
	else if ( i!=0 && !(i%32) ) {

	    DG_COPY_VEC( pos, (pos-2) );
	    pos++;
	    DG_COPY_VEC( pos, (pos-2) );
	    pos++;

	    DG_COPY_VEC( &work->center[i], &work->center[i-1] );

	    _sceVu0AddVector( &work->center[i+1], &work->bottom[i+1], &work->height[i+1] );
	    _sceVu0AddVector( &work->center[i+1], &work->center[i+1], &work->noise[i+1] );
	    _sceVu0AddVector( &work->center[i+1], &work->center[i+1], &work->wave[i+1] );
	    
	    _sceVu0SubVector( &work->noise[i+1], &work->noise[i+1], &work->speed[i+1] );
	    _sceVu0ScaleVector( &work->height[i+1], &work->height[i+1], 0.97f + frnd()*0.01f );
	    _sceVu0ScaleVector( &work->wave[i+1], &work->wave[i+1], 0.95f );

	}
	/* 通常処理 */
	else {

	    /* 次点を算出 */
	    _sceVu0AddVector( &work->center[i+1], &work->bottom[i+1], &work->height[i+1] );
	    _sceVu0AddVector( &work->center[i+1], &work->center[i+1], &work->noise[i+1] );
	    _sceVu0AddVector( &work->center[i+1], &work->center[i+1], &work->wave[i+1] );

	    /* 次点パラメータ更新 */	    
	    _sceVu0SubVector( &work->noise[i+1], &work->noise[i+1], &work->speed[i+1] );
	    _sceVu0ScaleVector( &work->height[i+1], &work->height[i+1], 0.97f + frnd()*0.01f );
	    _sceVu0ScaleVector( &work->wave[i+1], &work->wave[i+1], 0.95f );

	    
	    /* 次の中心位置へのベクトルから幅ベクトル算出 */
	    _sceVu0SubVector( &vectmp, &work->center[i+1], &work->center[i] );
	    
	    _sceVu0OuterProduct( &vectmp, &vectmp, (FVECTOR *)DG_Chanls[0].eye.m[2] );
	    _sceVu0Normalize( &vectmp, &vectmp );
	    _sceVu0ScaleVector( &vec_width, &vectmp, work->width );
	    
	    /* 頂点設定 */
	    _sceVu0SubVector( pos, &work->center[i], &vec_width );
	    pos++;	    
	    _sceVu0AddVector( pos, &work->center[i], &vec_width );
	    pos++;

	    _sceVu0ScaleVector( &work->speed[i], &work->speed[i], 0.9f );
	    
	}

	uvrgb->a = work->alpha;
	uvrgb++;
	uvrgb->a = work->alpha;
	uvrgb++;

    }

    /* プラズマ幅更新 */
    if ( work->life < 10 ) {
	work->width *= 0.95f;
    }

    /* プラズマアルファ更新 */
    work->alpha -= work->sub_alpha;


    
    if ( ++work->life >= LIFE ) {
	GV_DestroyActor( work );
    }


}


static void Die(Work *work )
{
    /* メモリ解放 */
    work->prim = OK_FreePrim2( work->prim );
}


/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *color )
{

	int		i,j,k,n;
	FVECTOR		*pos;	
	DG_PRIM2_UVRGB	*uvrgb;
	SVECTOR		rot;
	FMATRIX		mat;
	int		rot_flag;

	FVECTOR		center;

	FVECTOR		from_middle;
	FVECTOR		middle_to;

	FVECTOR		vectmp;
	FVECTOR		m_bottom;

	FVECTOR		vec_x1;
	FVECTOR		vec_x2;
	float		length;
	FVECTOR		height;

	float		param[3];


	prim->raise = 0;

	
	//DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos   	= SCR_POS;
	uvrgb	= SCR_UVS;	



	_sceVu0SubVector( &from_middle, &work->middle, &work->from );
	_sceVu0SubVector( &vectmp, &work->to, &work->from );
	_sceVu0Normalize( &vectmp, &vectmp );
	length = _sceVu0InnerProduct( &from_middle, &vectmp );
	_sceVu0ScaleVector( &m_bottom, &vectmp, length );

	_sceVu0SubVector( &height, &from_middle, &m_bottom );
	_sceVu0ScaleVector( &vec_x1, &m_bottom, 1.0f / (float)(N_VERTS/2-1) );

	_sceVu0AddVector( &m_bottom, &m_bottom, &work->from );

	_sceVu0SubVector( &middle_to, &work->to, &work->middle );
	length = _sceVu0InnerProduct( &middle_to, &vectmp );
	_sceVu0ScaleVector( &vec_x2, &vectmp, length );
	_sceVu0ScaleVector( &vec_x2, &vec_x2, 1.0f / (float)(N_VERTS/2-1) );



	OK_DirVecXY( &vectmp, &DG_ZeroVector, &rot );
	DG_SetPos( &DG_UnitMatrix );
	DG_RotatePos( &rot );

	rot.vx = 0;
	rot.vy = 0;
	rot.vz = irnd()%4096;

	DG_RotatePos( &rot );
	DG_GetPos( &mat );	

	param[0] = 0.f;
	param[1] = 0.f;
	param[2] = 0.f;

	if ( irnd()%2 ) {
	    rot_flag = 1;
	}
	else {
	    rot_flag = 0;
	}


	
	for ( i = 0 ; i < N_PRIMS; i++ ) {

	    for ( j = 0 ; j < N_VERTS ; j++ ) {

		DG_COPY_VEC( pos, &DG_ZeroVector );

		pos++;

		if ( j%2 ) {
		    uvrgb->u = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		}
		else {
		    uvrgb->u = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		}

		if ( (j/2)%2 ) {
		    uvrgb->v = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		}
		else {
		    uvrgb->v = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		}

		uvrgb->r = color->vx;
		uvrgb->g = color->vy;
		uvrgb->b = color->vz;
		uvrgb->a = color->vw;
		uvrgb->q = 4096;
		if ( j==0 ) {
		    uvrgb->f = 0x8fff;
		}
		else {
		    uvrgb->f = 0x0fff;
		}

		uvrgb++;

		if ( j % 2 ) {

		    n = ( i * N_VERTS + j ) / 2;

		    if ( rot_flag ) {
			rot.vz = irnd()%150 + 4096/24;
		    }
		    else {
			rot.vz = -irnd()%150 - 4096/24;
		    }
		    DG_SetPos( &mat );
		    DG_RotatePos( &rot );
		    DG_GetPos( &mat );


		    /* 中心点 */
		    if ( n == 0 ) {					/* 開始点 */
			DG_COPY_VEC( &work->bottom[n], &work->from );
			DG_COPY_VEC( &center, &work->from );
			DG_COPY_VEC( &work->height[n], &DG_ZeroVector );
			_sceVu0ScaleVector( &work->wave[n], (FVECTOR *)mat.m[0], WAVE_HEIGHT );
			DG_COPY_VEC( &work->noise[n], &DG_ZeroVector );
			DG_COPY_VEC( &work->speed[n], &DG_ZeroVector );
		    }
		    else if ( n == 31 || n == 32 ) {			/* 中間点 */
			DG_COPY_VEC( &work->bottom[n], &m_bottom );
			DG_COPY_VEC( &work->height[n], &height );
			_sceVu0ScaleVector( &work->wave[n], (FVECTOR *)mat.m[0], WAVE_HEIGHT );
			DG_COPY_VEC( &work->noise[n], &DG_ZeroVector );
			DG_COPY_VEC( &work->speed[n], &DG_ZeroVector );
			DG_COPY_VEC( &center, &m_bottom );
		    }
		    else if ( n == N_PRIMS * N_VERTS / 2 - 1 ) {	/* 最終点 */
			DG_COPY_VEC( &work->center[n], &work->to );
			DG_COPY_VEC( &work->height[n], &DG_ZeroVector );
			DG_COPY_VEC( &work->noise[n], &DG_ZeroVector );
			DG_COPY_VEC( &work->speed[n], &DG_ZeroVector );
		    }
		    else {
			if ( i == 0 ) {
			    _sceVu0AddVector( &center, &center, &vec_x1 );
			    DG_COPY_VEC( &work->bottom[n], &center );
			    _sceVu0ScaleVector( &work->height[n],
						&height, (float)( 31*31 - (31-n)*(31-n) ) / (float)(31*31) );
			    _sceVu0ScaleVector( &work->wave[n], (FVECTOR *)mat.m[0], WAVE_HEIGHT );

			    _sceVu0AddVector( &work->center[n], &work->bottom[n], &work->height[n] );

			}
			else {
			    _sceVu0AddVector( &center, &center, &vec_x2 );
			    DG_COPY_VEC( &work->bottom[n], &center );
			    _sceVu0ScaleVector( &work->height[n],
						&height, (float)( 31*31 - (n-32)*(n-32) ) / (float)(31*31) );
			    _sceVu0ScaleVector( &work->wave[n], (FVECTOR *)mat.m[0], WAVE_HEIGHT );
			    
			    _sceVu0AddVector( &work->center[n], &work->bottom[n], &work->height[n] );
			}
		    }

		    /* ノイズパラメータ更新 */
		    for ( k = 0 ; k < 3 ; k++ ) {
			param[k] += frnd()*0.10f;
			if ( param[k] > 0.30f ) param[k] = 0.25f;
			else if ( param[k] < -0.30f ) param[k] = -0.25f;
		    }

		    work->speed[n].vx = SPEED * param[0];
		    work->speed[n].vy = SPEED * param[1];
		    work->speed[n].vz = SPEED * param[2];

		    _sceVu0ScaleVector( &work->noise[n], &work->speed[n], 17.5f + frnd()*2.5f );
		    _sceVu0AddVector( &work->center[n], &work->center[n], &work->noise[n] );
		    
		}

	    }
	    
	}


	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );

	prim->flag |= ( DG_PRIM2_INVISIBLE1 | DG_PRIM2_INVISIBLE2 | DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources( Work *work, FVECTOR *from, FVECTOR *middle, FVECTOR *to, float width, FVECTOR *color )
{
    
    DG_PRIM2		*prim;
    DG_TEX		*tex;


    work->from   = *from;
    work->middle = *middle;
    work->to     = *to;

    /*AN_Test_Eye2( &work->from, 500.f );
    AN_Test_Eye2( &work->middle, 500.f );
    AN_Test_Eye2( &work->to, 500.f );*/


    work->alpha     = color->vw;
    work->sub_alpha = (int)color->vw / LIFE;
    work->life      = 0;

    work->width = width;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY | DG_PRIM2_ALPHA | DG_PRIM2_SHADE,// | DG_PRIM2_TEX,
				      N_PRIMS, N_VERTS );

    //tex = DG_GetTexture( GV_StrCode( "smoke_lp3_alp" ) );
    tex = DG_GetTexture( GV_StrCode( "flare0_msk" ) );
    InitPacket( work, prim, tex, color );
    
    return 0;

}


static int GetResourcesScn( Work *work ) {
    
    DG_PRIM2		*prim;
    DG_TEX		*tex;
    FVECTOR		vectmp;
    FVECTOR		color;
    float		rot;

    work->from.vx = 6700.0f + frnd()*500.0f;
    work->from.vy = 0.0f;
    work->from.vz =-240700.0f + frnd()*500.0f;
    work->from.vw = 1.0f;

    rot = frnd()*PI;
    vectmp.vx = 1.0f * vu0_Cos(rot);
    vectmp.vy = 0.0f;
    vectmp.vz = 1.0f * vu0_Sin(rot);

    DG_COPY_VEC( &work->middle, &work->from );
    work->middle.vx += vectmp.vx * ( 1000.0f + frnd()*500.0f );
    work->middle.vy += 500.0f + frnd()*300.0f;
    work->middle.vz += vectmp.vz * ( 1000.0f + frnd()*500.0f );
    work->middle.vw = 1.0f;

    DG_COPY_VEC( &work->to, &work->middle );
    work->to.vx += vectmp.vx * ( 1000.0f + frnd()*500.0f );
    work->to.vy = 0.0f;
    work->to.vz += vectmp.vz * ( 1000.0f + frnd()*500.0f );
    work->to.vw = 1.0f;

    /*AN_Test_Eye2( &work->from, 500.f );
    AN_Test_Eye2( &work->middle, 500.f );
    AN_Test_Eye2( &work->to, 500.f );*/
    
    color.vx = 255.0f;
    color.vy = 128.0f;
    color.vz = 64.0f;
    color.vw = 64.0f;
    
    work->alpha     = color.vw;
    work->cnt = 15 + irnd()%10;
    work->sub_alpha = (int)color.vw / work->cnt;
    work->life      = 0;

    work->width = 8.0f + frnd()*2.0f;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY | DG_PRIM2_ALPHA | DG_PRIM2_SHADE,// | DG_PRIM2_TEX,
				      N_PRIMS, N_VERTS );

    //tex = DG_GetTexture( GV_StrCode( "smoke_lp3_alp" ) );
    tex = DG_GetTexture( GV_StrCode( "flare0_msk" ) );
    InitPacket( work, prim, tex, &color );
    
    return 0;

}



/*
  NewPolyPlasma : 呼び出し関数
  FVECTOR	*from
  FVECTOR	*middle
  FVECTOR	*to
  float		width
  FVECTOR	*color
*/  
void *NewPolyPlasma( FVECTOR *from, FVECTOR *middle, FVECTOR *to, float width, FVECTOR *color )
{
	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, from, middle, to, width, color ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}


void *NewPolyPlasmaScn( int name, int map )
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



