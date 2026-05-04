//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  splash_blood.c
  刀斬られ血しぶき
  2001/07/26 Yuuta Kunibe
  $Id: splash_blood.c,v 1.1.1.3 2002/11/19 11:44:51 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>
#include	"gameheader.h"
#include	"libmt.h"
#include	"utl_dma.h"
#include	"../../okajima/etc/ok_util.h"



#define	N_PRIMS		(64)
#define	N_VERTS		(4)

#define BASE_RGB	(24)
#define ALPHA		(64)
#define	SPEED		(1200.0f)

#define LIFE		(300)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)



typedef struct {
    float	param1;
    float	param2;
    FVECTOR	vec;
    int		alpha;
} Blood_Param;



typedef	struct	{

    GV_ACT_EX	actor ;

    DG_PRIM2	*prim ;

    FMATRIX	mat;
    FVECTOR	*pos1;
    FVECTOR	*pos2;

    FVECTOR	pos2_tmp;

    int		init_alpha;	
    float	init_speed;
    int 	life;	

	FVECTOR	vec_splash;
    
    Blood_Param	param[N_PRIMS];
    
} Work ;



static void MakeSplashMatrix( Work *work )
{

    FMATRIX	matInv;
    FVECTOR	vectmp;
    FVECTOR	vectmp2;
    FVECTOR	vec_splash;
    float	ftmp;

    //FVECTOR	pos1;
    //FVECTOR	pos2;


    DG_COPY_MAT( &work->mat, &DG_UnitMatrix );
    //DG_COPY_VEC( &pos1, (FVECTOR *)GM_PlayerBody->objs->objs[3].world.m[3] );
    //DG_COPY_VEC( &pos2, (FVECTOR *)GM_PlayerBody->objs->objs[17].world.m[3] );

    /* Z軸(斬られ方向)取得 */
    _sceVu0SubVector( &vectmp, work->pos2, work->pos1 );
    _sceVu0Normalize( (FVECTOR *)work->mat.m[2], &vectmp );

    /* Y軸(吹き出し方向)取得 */
    // 吹き出し方向を絶対化
    DG_SetPos( &GM_PlayerBody->objs->objs[0].world );
    DG_RotVector( &work->vec_splash, &vec_splash, 1 );
    ftmp = _sceVu0InnerProduct( &vec_splash, (FVECTOR *)work->mat.m[2] );
    _sceVu0ScaleVector( &vectmp2, (FVECTOR *)work->mat.m[2], ftmp );
    _sceVu0SubVector( &vectmp2, &vec_splash, &vectmp2 );
    _sceVu0Normalize( (FVECTOR *)work->mat.m[1], &vectmp2 );

    /* X軸(幅方向)取得 */
    _sceVu0OuterProduct( (FVECTOR *)work->mat.m[0], (FVECTOR *)work->mat.m[1], (FVECTOR *)work->mat.m[2] );

    /* 位置(斬られ始め位置)取得 */
    DG_COPY_VEC( (FVECTOR *)work->mat.m[3], work->pos1 );

    //HZX_ViewMatrix( &work->mat, 1000.0f );

    /* pos2の座標を相対化 */
    FastInverseMatrix( &matInv, &work->mat );
    _sceVu0ApplyMatrix( &work->pos2_tmp, &matInv, work->pos2 );

    //printf("pos2_tmp : x %f, y %f, z %f\n", work->pos2_tmp.vx, work->pos2_tmp.vy, work->pos2_tmp.vz );

}


static void Act( Work *work )
{

    int			i,j;
    int			clock;
    FVECTOR		*pos;
    FVECTOR		*pos_pre;
    DG_PRIM2_UVRGB	*uvrgb;
    DG_PRIM2_UVRGB	*uvrgb_pre;


    float		width;
    Blood_Param		*param;


    /* 吹き出しマトリクス算出 */
    MakeSplashMatrix(work);
    

    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }

    
    clock     = work->prim->buffer_clock;
    pos       = work->prim->pos[clock];
    pos_pre   = work->prim->pos[1-clock];    
    uvrgb     = work->prim->uvrgb[clock];
    uvrgb_pre = work->prim->uvrgb[1-clock];
    param     = work->param;
    
    //AN_Test_Eye2( &work->pos1, 500.f );
    //AN_Test_Eye2( &work->pos2, 500.f );

    for ( i = 0 ; i < N_PRIMS ; i++ ) {

		param->alpha -= 32;

	if ( param->alpha < 0 ) {

	    width = frnd()*100.0f;

	    param->param1 = 0.40f + frnd()*0.50f;
	    param->param2 = param->param1 + 0.30f+frnd()*0.10f;

	    DG_COPY_VEC( pos, &DG_ZeroVector );
	    pos->vz = work->pos2_tmp.vz * param->param1;
	    pos++;

	    DG_COPY_VEC( pos, &DG_ZeroVector );
	    pos->vz = work->pos2_tmp.vz * param->param2;
	    pos++;

	    DG_COPY_VEC( pos, ( pos - 2 ) );
	    pos->vx += width;
	    //pos->vy += 400.0f + frnd()*100.0f;
	    pos->vy += work->init_speed/3.0f + frnd()*work->init_speed/12.0f;
	    pos->vz += pos->vy * ( param->param1 - 0.5f );
	    pos++;
	
	    DG_COPY_VEC( pos, ( pos - 2 ) );
	    pos->vx += width;
	    //pos->vy += 400.0f + frnd()*100.0f;
	    pos->vy += work->init_speed/3.0f + frnd()*work->init_speed/12.0f;
	    pos->vz += pos->vy * ( param->param2 - 0.5f );
	    pos++;


	    param->vec.vx = width;
	    param->vec.vy = work->init_speed + frnd()*(work->init_speed/3.0f);
	    param->vec.vz = param->vec.vy * ( ( param->param1 + param->param2 )*0.5f - 0.5f );//0.0f;
	    _sceVu0ScaleVector( &param->vec, &param->vec, 0.125f );

	    param->alpha = work->init_alpha + irnd()%work->init_alpha;

	    uvrgb->a = param->alpha;
	    uvrgb++;
	    uvrgb->a = param->alpha;
	    uvrgb++;
	    uvrgb->a = param->alpha;
	    uvrgb++;
	    uvrgb->a = param->alpha;
	    uvrgb++;

	}
	else {
	
	    for ( j = 0 ; j < N_VERTS ; j++ ) {

		if ( j == 2 || j == 3 ) {
		    _sceVu0AddVector( pos, pos_pre, &param->vec );
		}
		else {
		    DG_COPY_VEC( pos, pos_pre );
		}

		pos++;
		pos_pre++;

		uvrgb->a = param->alpha;

	    }

	}

	param++;
	
    }


    if ( ++work->life > 60 ) {
		work->init_alpha -= 1;
		work->init_speed *= 0.98f;
    }


    if ( work->init_alpha <= 0 ) {
		GV_DestroyActor( work );
    }
    
    
}


static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}




static int InitPacket( Work *work )
{

    DG_PRIM2		*prim;
    DG_TEX		*tex;
    FVECTOR		*pos;
    DG_PRIM2_UVRGB	*uvrgb;
    int			i;
    float		width;
    Blood_Param		*param;



    prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
    if ( prim == NULL ) {
	return 0;
    }

    work->prim->root = &work->mat;

    tex = DG_GetTexture( GV_StrCode( "chi01_alp" ) );
    DG_ConfigPrim2Tex( prim, tex );
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );


    pos   = SCR_POS;
    uvrgb = SCR_UVS;
    param = work->param;
	
    for ( i = 0 ; i < N_PRIMS ; i++ ) {

	width = frnd()*100.0f;

	param->param1 = 0.50f + frnd()*0.50f;
	param->param2 = param->param1 + 0.40f+frnd()*0.20f;

	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos++;
	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos++;
	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos++;
	DG_COPY_VEC( pos, &DG_ZeroVector );
	pos++;



	/* 速度算出 */
	param->vec.vx = width;
	param->vec.vy = 1200.0f + frnd()*400.0f;
	param->vec.vz = 0.0f;
	_sceVu0ScaleVector( &param->vec, &param->vec, 0.25f );




	param->alpha = irnd()%ALPHA;
	
	uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb->v = FTOI12( 0.5F * tex->v_scale + tex->v_offset ) ;
	uvrgb->q = 4096;
	uvrgb->f = 0x8fff;
	uvrgb->r = BASE_RGB;
	uvrgb->g = 0;
	uvrgb->b = 0;
	uvrgb->a = param->alpha;
	uvrgb++;
	
	uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb->v = FTOI12( 0.5F * tex->v_scale + tex->v_offset ) ;
	uvrgb->q = 4096;
	uvrgb->f = 0x8fff;
	uvrgb->r = BASE_RGB;
	uvrgb->g = 0;
	uvrgb->b = 0;
	uvrgb->a = param->alpha;
	uvrgb++;
	
	uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb->q = 4096;
	uvrgb->f = 0x0fff;
	uvrgb->r = BASE_RGB;
	uvrgb->g = 0;
	uvrgb->b = 0;
	uvrgb->a = param->alpha;
	uvrgb++;
	
	uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb->q = 4096;
	uvrgb->f = 0x0fff;
	uvrgb->r = BASE_RGB;
	uvrgb->g = 0;
	uvrgb->b = 0;
	uvrgb->a = param->alpha;
	uvrgb++;


	param++;

    }	
    
    OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
    OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
    OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );
    OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );
 
    return 1;

}




static int GetResources( Work *work, FVECTOR *pos1, FVECTOR *pos2, FVECTOR *vec )
{

	FMATRIX	matInv;

	work->pos1 = pos1;
	work->pos2 = pos2;

	/* 血しぶき放出方向を本体に対して相対化して持っておく */
	DG_COPY_VEC( &work->vec_splash, vec );
    FastInverseMatrix( &matInv, &GM_PlayerBody->objs->objs[0].world );
	DG_COPY_VEC( &matInv.m[3], &DG_ZeroVector );
    _sceVu0ApplyMatrix( &work->vec_splash, &matInv, &work->vec_splash );
	_sceVu0Normalize( &work->vec_splash, &work->vec_splash );

	work->init_alpha = ALPHA;
	work->init_speed = SPEED;
	work->life       = 0;
	
	if ( InitPacket( work ) > 0 ) {
	    return 0;
	}
	else {
	    return -1;
	}

}



void *NewSplashBlood( FVECTOR *pos1, FVECTOR *pos2, FVECTOR *vec )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {

		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor )
		if ( GetResources( work, pos1, pos2, vec ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
