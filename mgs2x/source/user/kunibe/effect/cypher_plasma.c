//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  cypher_plasma.c
  サイファー用ダメージプラズマ
  2001/06/13 Yuuta Kunibe	
  $Id: cypher_plasma.c,v 1.1.1.3 2002/11/19 11:44:35 Yoshizawa1 Exp $
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

#define N_PRIMS			(32)//(16)
#define N_VERTS 		(32)
#define SPRT_NUM 		( N_PRIMS * N_VERTS )

#define SCR_POS 		(SCRPAD_ADDR)
#define SCR_UVS 		(SCRPAD_ADDR+0x2000)

#define	WAVE_HEIGHT		(50.0f)
#define SPEED			(50.0f)

#define LIFE			(10)

#define	PLASMA_TEX		( GV_StrCode( "flare0_msk" ) )
#define	BODY_RADIUS		(750.0f)
#define	BODY_WIDTH		(30.0f)

#define	NOISE			(300.0f)


typedef struct {
    FVECTOR	center;
    FVECTOR	height;
    FVECTOR	wave;
    FVECTOR	noise;
    FVECTOR	speed;
} PointParam;


typedef	struct {

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;

    FMATRIX	*body;
    
    FVECTOR	*from;
    FVECTOR	middle;
    FVECTOR	to;

    PointParam	param[SPRT_NUM];

	FVECTOR	color;
    int		alpha;
    int		sub_alpha;
    int		life;

    float	width;

} Work;



static inline void AddVector4( FVECTOR *r, FVECTOR *a, FVECTOR *b, FVECTOR *c, FVECTOR *d  )
{
// yano /*未検証*/
#ifdef BP_PSX2_ASM
	asm volatile ("
	lqc2		vf1, 0(%0)
	lqc2		vf2, 0(%1)
	vadd.xyz	vf5, vf2, vf1
	lqc2		vf3, 0(%2)
	vadd.xyz	vf6, vf3, vf5
	lqc2		vf4, 0(%3)
	vadd.xyz	vf7, vf4, vf6
	sqc2		vf7, 0(%4)
	" : : "r"(a), "r"(b), "r"(c), "r"(d), "r"(r) ) ;
#else
	FVECTOR fvdmy0, fvdmy1;
	_sceVu0AddVector( &fvdmy0, a, b );
	_sceVu0AddVector( &fvdmy1, c, d );
	_sceVu0AddVector( r, &fvdmy0, &fvdmy1 );
#endif
}



/* アクト関数 */
static void Act( Work *work )
{

    int 		i,j,k;
    int			clock;
    FVECTOR		*pos;
    DG_PRIM2_UVRGBWH	*uvrgbwh;
    PointParam		*param;


    /* 操作するバッファ取得 */
    DG_SwitchBuffPrim2( work->prim );
    clock = work->prim->buffer_clock;
    pos		= work->prim->pos[clock];
    uvrgbwh     = work->prim->uvrgb[clock];
    param 	= work->param;

    /* 本体マトリクス更新 */
    work->prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
    DG_COPY_MAT( &work->prim->as_world, work->body );


    /* スプライト更新 */
    for ( i = 0 ; i < 2 ; i++ ) {

	pos = SCR_POS;
	
        for ( j = 0 ; j < N_PRIMS/2 ; j++ ) {

	    for ( k = 0 ; k < N_VERTS ; k++ ) {

		AddVector4( pos, &param->center, &param->height, &param->wave, &param->noise );

		if ( work->life < 2 ) {
		    _sceVu0ScaleVector( &param->height, &param->height, 0.95f );
		}
		else {
		    _sceVu0ScaleVector( &param->height, &param->height, 0.95f );
		}

		uvrgbwh->a = work->alpha;

		pos++;
		param++;
		uvrgbwh++;

	    }

	}

	OK_Scr_Mem( &work->prim->pos[clock][i*N_PRIMS/2*N_VERTS], SCR_POS, sizeof(FVECTOR), N_PRIMS/2*N_VERTS );

    }


    /* アルファ更新 */
    work->alpha -= work->sub_alpha;


    if ( ++work->life > LIFE ) {
	GV_DestroyActor( work );
    }

}



static void Die( Work *work )
{
    /* メモリ解放 */
    if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
}



/* プリミティブ初期化関数 */
static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{

	int			i,j,k,l;
	FVECTOR			*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	FMATRIX			mat;
	SVECTOR			rot;
	SVECTOR			rot2;
	short			rot_add;
	FVECTOR			vectmp;
	float			angle;
	float			add_angle;
	float			noise[3];
	float			add_noise[3];
	int			noise_flag[3];

	PointParam		*param;
	

	prim->raise = 0;
	
	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );



   work->prim->flag |= DG_PRIM_AS_CUSTOMWORLD;
	DG_COPY_MAT( &work->prim->as_world, work->body );
	
	
	DG_SetPos( &DG_UnitMatrix );
	rot.vx = 0;
	rot.vy = irnd()%4096;
	rot.vz = 0;
	DG_RotatePos( &rot );
	DG_GetPos( &mat );
	
	rot.vx = 0;
	rot.vy = 4096/SPRT_NUM;
	rot.vz = 0;

	rot_add = 4096*6/SPRT_NUM;
	rot2.vx = 0;
	rot2.vy = 0;
	rot2.vz = 0;

	vectmp.vx = 150.0f;
	vectmp.vy = 0.0f;
	vectmp.vz = 0.0f;
	vectmp.vw = 1.0f;

	angle = 0.0f;
	add_angle = TPI*32.0f/(float)SPRT_NUM;

	noise[0] = 0.f;
	noise[1] = 0.f;
	noise[2] = 0.f;
	add_noise[0] = 0.f;
	add_noise[1] = 0.f;
	add_noise[2] = 0.f;
	noise_flag[0] = 0;
	noise_flag[1] = 0;
	noise_flag[2] = 0;

	param = work->param;

	
	for ( i = 0 ; i < 4 ; i++ ) {

	    pos   	= SCR_POS;
	    uvrgbwh	= SCR_UVS;

	    for ( j = 0 ; j < N_PRIMS/4 ; j++ ) {

		for ( k = 0 ; k < N_VERTS ; k++ ) {

		    if ( !(k%8) ) {

			for ( l = 0 ; l < 3 ; l++ ) {

			    if ( noise_flag[l] == 1 ) {
				add_noise[l] = -0.08f + frnd()*0.02f;
				if ( noise[l] + add_noise[l] < 0.50f ) {
				    noise_flag[l] = 0;
				}
			    }
			    else if ( noise_flag[l] == -1 ) {
				add_noise[l] = 0.08f + frnd()*0.02f;
				if ( noise[l] + add_noise[l] > -0.50f ) {
				    noise_flag[l] = 0;
				}
			    }
			    else {			   			    
				add_noise[l] = frnd()*0.10f;
				if ( noise[l] + add_noise[l] > 0.50f ) {
				    noise_flag[l] = 1;
				}
				else if ( noise[l] + add_noise[l] < -0.50f ) {
				    noise_flag[l] = -1;
				}
			    }

			    add_noise[l] *= 1.f/8.0f;

			}

		    }
			    

		    DG_SetPos( &mat );
		    DG_RotatePos( &rot );
		    DG_GetPos( &mat );
		    _sceVu0ScaleVector( &param->center, (FVECTOR *)mat.m[0], BODY_RADIUS );
		   		    
		    rot2.vz += rot_add;
		    if ( rot2.vz >= 4096 ) {
			rot2.vz -= 4096;
		    }
		    DG_RotatePos( &rot2 );
		    DG_PutVector( &vectmp, &param->height, 1 );		    

		    angle += add_angle;
		    if ( angle > PI ) {
			angle -= TPI;
		    }

		    param->wave.vx = 0.0f;
		    param->wave.vy = BODY_WIDTH * vu0_Sin( angle );
		    param->wave.vz = 0.0f;
		    
		    param->noise.vx = 500.0f * noise[0];
		    param->noise.vy = 500.0f * noise[1];
		    param->noise.vz = 500.0f * noise[2];

		    noise[0] += add_noise[0];
		    noise[1] += add_noise[1];
		    noise[2] += add_noise[2];



		    DG_COPY_VEC( pos, &DG_ZeroVector );

		    uvrgbwh->u0 = FTOI12( 0.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v0 = FTOI12( 0.0f * tex->v_scale + tex->v_offset );
		    uvrgbwh->q0 = 4096;
		    uvrgbwh->f0 = 0x0fff;

		    uvrgbwh->u1 = FTOI12( 1.0f * tex->u_scale + tex->u_offset );
		    uvrgbwh->v1 = FTOI12( 1.0f * tex->v_scale + tex->v_offset );
		    uvrgbwh->q1 = 4096;
		    uvrgbwh->f1 = 0x0fff;

		    uvrgbwh->w = work->width;
		    uvrgbwh->h = work->width;

		    uvrgbwh->r = work->color.vx;
		    uvrgbwh->g = work->color.vy;
		    uvrgbwh->b = work->color.vz;
		    uvrgbwh->a = work->color.vw;

		    pos++;
		    uvrgbwh++;
		    param++;
		    
		}

	    }

	    
	    OK_Scr_Mem( &prim->pos[ 0 ][ i*N_PRIMS/4*N_VERTS ], SCR_POS, sizeof(FVECTOR), N_PRIMS/4*N_VERTS );
	    OK_Scr_Mem( &prim->pos[ 1 ][ i*N_PRIMS/4*N_VERTS ], SCR_POS, sizeof(FVECTOR), N_PRIMS/4*N_VERTS );
		
	    OK_Scr_Mem( &( (DG_PRIM2_UVRGBWH*)(prim->uvrgb[ 0 ]) )[ i*N_PRIMS/4*N_VERTS ], SCR_UVS,
			sizeof(DG_PRIM2_UVRGBWH), N_PRIMS/4*N_VERTS );
	    OK_Scr_Mem( &( (DG_PRIM2_UVRGBWH*)(prim->uvrgb[ 1 ]) )[ i*N_PRIMS/4*N_VERTS ], SCR_UVS,
			sizeof(DG_PRIM2_UVRGBWH), N_PRIMS/4*N_VERTS );


	}

	prim->flag |= ( DG_PRIM2_INVISIBLE1 | DG_PRIM2_INVISIBLE2 | DG_PRIM2_INVISIBLE3 );

	return 1;

}



static int GetResources( Work *work, FMATRIX *body, float width )
{
    
    DG_PRIM2		*prim;
    DG_TEX		*tex;


    work->body = body;

    work->alpha     = work->color.vw;
    work->sub_alpha = (int)work->color.vw / LIFE;
    work->life      = 0;

    work->width = width;

    prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT | DG_PRIM2_ALPHA | DG_PRIM2_SHADE | DG_PRIM2_TEX,
									  N_PRIMS, N_VERTS );
    if ( prim == NULL ) {
		return -1;
    }

    tex = DG_GetTexture( PLASMA_TEX );
    InitPacket( work, prim, tex );

    return 0;

}



/*-------- NewCypherPlasma : スプライトプラズマ呼び出し関数 --------
     FMATRIX	body    : サイファー本体マトリクス     
     float	width	: スプライト幅
     FVECTOR	*color	: 色
------------------------------------------------------------------*/  
void *NewCypherPlasma( FMATRIX *body, float width, FVECTOR *color )
{

	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->color.vx = color->vx;
		work->color.vy = color->vy;
		work->color.vz = color->vz;
		work->color.vw = color->vw;
		if ( GetResources( work, body, width ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



void *NewDemoCypherPlasma( FMATRIX *body, float width, int color )
{

	Work	*work;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		work->color.vx = (float)( (color>>24) & 255 );
		work->color.vy = (float)( (color>>16) & 255 );
		work->color.vz = (float)( (color>>8) & 255 );
		work->color.vw = (float)( color & 255 );
		if ( GetResources( work, body, width ) < 0 ) {
			GV_DestroyActor( work );
			return NULL;
		}
	}

	return (void *)work ;

}



