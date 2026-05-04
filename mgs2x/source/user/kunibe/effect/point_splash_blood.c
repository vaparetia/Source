//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  point_splash_blood.c
  刀斬られ血しぶき
  2001/07/26 Yuuta Kunibe
  $Id: point_splash_blood.c,v 1.1.1.3 2002/11/19 11:44:45 Yoshizawa1 Exp $
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

#ifdef ENGLISH
#define	BASE_R		(12)
#define	BASE_G		(0)
#define	BASE_B		(0)
#define ALPHA		(48)
#else
#define	BASE_R		(12)
#define	BASE_G		(0)
#define	BASE_B		(0)
#define ALPHA		(32)
#endif

#define	SPEED		(400.0f)

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

    GV_ACT_EX	actor;

    DG_PRIM2	*prim;
    DG_PRIM2	*prim2;

    FMATRIX		*mat;
	FMATRIX		matrix;

	SVECTOR		rot;

    int			init_alpha;
    float		init_speed;
    int 		life;    

	FVECTOR		vec_splash;
    
    Blood_Param	param[N_PRIMS];
    Blood_Param	param_part[N_PRIMS];

    FVECTOR		splash_point;
    
} Work ;


extern void *NewBloodSmoke( FVECTOR* center );
extern void *NewBloodSmokeInpact( FVECTOR* center );


static void MakeSplashMatrix( Work *work )
{

    
    /* 吹き出し方向をZとする基準マトリクスを生成 */
    DG_SetPos( work->mat );
    DG_RotatePos( &work->rot );
    DG_GetPos( &work->matrix );

    /* 吹き出し点を絶対化してマトリクスの中心とする */
    DG_SetPos( work->mat );
    DG_PutVector( &work->splash_point, (FVECTOR *)work->matrix.m[3], 1 );
    
    //HZX_ViewMatrix( &work->matrix, 1000.0f );
	
}



static void ActPart( Work *work )
{

    int				i,j;
    int				clock;
    FVECTOR			*pos;
    FVECTOR			*pos_pre;
    DG_PRIM2_UVRGB	*uvrgb;
    DG_PRIM2_UVRGB	*uvrgb_pre;

	FVECTOR			vectmp;
    Blood_Param		*param;
	float			ftmp;
	FVECTOR			position[4];


   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim2 ) )
   {
      return;
   }
    
    clock     = work->prim2->buffer_clock;
    pos       = work->prim2->pos[clock];
    pos_pre   = work->prim2->pos[1-clock];    
    uvrgb     = work->prim2->uvrgb[clock];
    uvrgb_pre = work->prim2->uvrgb[1-clock];
    param     = work->param_part;
    


    for ( i = 0 ; i < N_PRIMS ; i++ ) {

		param->alpha -= 4;

		if ( param->alpha < 0 ) {

			param->param1 = 0.40f + frnd()*0.50f;
			param->param2 = param->param1 + 0.30f+frnd()*0.10f;


			vectmp.vx = frnd()*100.0f;//frnd()*10.0f;
			vectmp.vy = frnd()*50.0f;//0.0f;
			vectmp.vz = 200.0f + frnd()*100.0f;//50.0f + frnd()*20.0f;
			//_sceVu0ScaleVector( &vectmp, &vectmp, 0.25f );
			_sceVu0ScaleVector( &vectmp, &vectmp, 0.25f+frnd()*0.05f );


			position[0].vx = 40.0f + frnd()*10.0f;//120.0f;
			position[0].vy = frnd()*10.0f;
			position[0].vz = 0.0f;
			position[0].vw = 1.0f;
			_sceVu0AddVector( &position[0], &position[0], &vectmp );	// 根本出ないように速度分前に出しておく

			position[1].vx =-40.0f - frnd()*10.0f;//-120.0f;
			position[1].vy = frnd()*10.0f;
			position[1].vz = 0.0f;
			position[1].vw = 1.0f;
			_sceVu0AddVector( &position[1], &position[1], &vectmp );	// 根本出ないように速度分前に出しておく


			ftmp = 200.0f + frnd()*100.0f;

			DG_COPY_VEC( &position[2], &position[0] );
			position[2].vz += ftmp;
			position[2].vx += ftmp * vectmp.vx / vectmp.vz;
			position[2].vy += ftmp * vectmp.vy / vectmp.vz;
			position[2].vw = 1.0f;
	
			DG_COPY_VEC( &position[3], &position[1] );
			position[3].vz += ftmp;
			position[3].vx += ftmp * vectmp.vx / vectmp.vz;
			position[3].vy += ftmp * vectmp.vy / vectmp.vz;
			position[3].vw = 1.0f;


			/* 速度絶対化 */
			DG_SetPos( &work->matrix );
			DG_RotVector( &vectmp, &param->vec, 1 );

			/* 初期位置いっぺんに絶対化 */
			DG_SetPos( &work->matrix );
			DG_PutVector( position, pos, 4 );


			pos += 4;
			pos_pre += 4;			



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
			
				_sceVu0ScaleVector( &vectmp, &param->vec, 1.5f );
				_sceVu0AddVector( &pos[2], &pos_pre[2], &vectmp );
				_sceVu0AddVector( &pos[3], &pos_pre[3], &vectmp );

				_sceVu0ScaleVector( &vectmp, &param->vec, 5.0f );
				_sceVu0SubVector( &pos[0], &pos[2], &vectmp );
				_sceVu0SubVector( &pos[1], &pos[3], &vectmp );

				pos += 4;
				pos_pre += 4;


			for ( j = 0 ; j < N_VERTS ; j++ ) {
				uvrgb->a = param->alpha;
				uvrgb++;
			}

			param->vec.vy -= 1.0f;				

		}

		param++;
	
    }


    if ( GM_PlayerStatus & PLAYER_WATCH ) {
	extern void PutCameraBlood( FVECTOR *pos_pers, int white );
	FVECTOR pers;

	DG_SetPos( &DG_Chanls[0].eye_pers );
	DG_PutVector( (FVECTOR *)work->matrix.m[3], &pers, 1 );

	//if( pers.vz >  pers.vw ) {	
	    pers.vw = (pers.vw  > 0.0f)? pers.vw: -pers.vw;
	//}

	pers.vx /= pers.vw;
	pers.vy /= pers.vw;
	pers.vz = 0.0f; ///= pers.vw;
	/*printf("%f %f %f %f\n",pers.vx,pers.vy,pers.vz,pers.vw);*/
	if( pers.vy > -1.1f && pers.vy < 1.1f && pers.vx > -1.1f && pers.vx < 1.1f ) {
	    PutCameraBlood( &pers, 0 );
	}

	/*pers.vx = 0.0f;
	pers.vy = 0.0f;
	pers.vz = 200.0f;
	PutCameraBlood( &pers, 0 );*/

    }

}



static void Act( Work *work )
{

    int				i,j;
    int				clock;
    FVECTOR			*pos;
    FVECTOR			*pos_pre;
    DG_PRIM2_UVRGB	*uvrgb;
    DG_PRIM2_UVRGB	*uvrgb_pre;

    Blood_Param		*param;

    //AR_PARTICLE_HALF
    if( !DG_SwitchBuffPrim2( work->prim ) )
    {
       return;
    }

    /* 吹き出しマトリクス算出 */
    MakeSplashMatrix(work);


    clock     = work->prim->buffer_clock;
    pos       = work->prim->pos[clock];
    pos_pre   = work->prim->pos[1-clock];    
    uvrgb     = work->prim->uvrgb[clock];
    uvrgb_pre = work->prim->uvrgb[1-clock];
    param     = work->param;
    


    for ( i = 0 ; i < N_PRIMS ; i++ ) {

		param->alpha -= 8;

		if ( param->alpha < 0 ) {

			param->param1 = 0.40f + frnd()*0.50f;
			param->param2 = param->param1 + 0.30f+frnd()*0.10f;


			pos->vx = 50.0f;
			pos->vy = frnd()*10.0f;
			pos->vz = 0.0f;
			pos++;

			pos->vx =-50.0f;
			pos->vy = frnd()*10.0f;
			pos->vz = 0.0f;
			pos++;

			DG_COPY_VEC( pos, ( pos - 2 ) );
			pos->vz += 200.0f + frnd()*50.0f;
			pos++;
	
			DG_COPY_VEC( pos, ( pos - 2 ) );
			pos->vz += 200.0f + frnd()*50.0f;
			pos++;


			/*param->vec.vx = frnd()*200.0f;
			param->vec.vy = frnd()*100.0f;
			param->vec.vz = 400.0f + frnd()*200.0f;*/
			param->vec.vx = frnd()*work->init_speed*0.5f;
			param->vec.vy = frnd()*work->init_speed*0.25f;
			param->vec.vz = work->init_speed + frnd()*work->init_speed*0.5f;
			_sceVu0ScaleVector( &param->vec, &param->vec, 0.50f );



			param->alpha = work->init_alpha + irnd()%work->init_alpha;
		
			uvrgb->a = param->alpha;
			uvrgb++;
			uvrgb->a = param->alpha;
			uvrgb++;
			uvrgb->a = param->alpha;
			uvrgb++;
			uvrgb->a = param->alpha;
			uvrgb++;

			pos_pre += 4;
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
				uvrgb++;

			}

		}

		param++;
	
    }


    ActPart( work );

    /*if ( work->life > 10 && work->life < 15 ) {
	work->init_speed *= 0.9f;
    }*/

    if ( work->life == DIRECT_TICK( 20 ) ) {
		FVECTOR vectmp;

		DG_COPY_VEC( &vectmp, &DG_ZeroVector );
		vectmp.vz = 500.0f;
		DG_SetPos( &work->matrix );
		DG_PutVector( &vectmp, &vectmp, 1 );	
		NewBloodSmoke( &vectmp );
    }

	if ( work->life == DIRECT_TICK( 10 ) ) {
		GM_SeSetMode( SD_E_BROODSW1, (FVECTOR *)work->matrix.m[3], GM_SEMODE_NORMAL );
	}


    if ( ++work->life > DIRECT_TICK( 90 ) ) {
		if ( work->life%2 ) {
			work->init_alpha -= 1;
			work->init_speed *= 0.98f;
		}
    }

    if ( work->init_alpha <= 0 ) {
		GV_DestroyActor( work );
    }
    
    
}


static void Die( Work *work )
{
	if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
	if ( work->prim2 ) work->prim2 = OK_FreePrim2( work->prim2 );
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

    work->prim->root = &work->matrix;

    tex = DG_GetTexture( GV_StrCode( "chi01_alp" ) );
    DG_ConfigPrim2Tex( prim, tex );
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );


    pos   = SCR_POS;
    uvrgb = SCR_UVS;
    param = work->param;
	
    for ( i = 0 ; i < N_PRIMS ; i++ ) {


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
	width = frnd()*100.0f;
	param->vec.vx = 1200.0f + frnd()*400.0f;
	param->vec.vy = width;
	param->vec.vz = 0.0f;
	_sceVu0ScaleVector( &param->vec, &param->vec, 0.25f );



	param->alpha = irnd()%ALPHA;
	
	uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb->v = FTOI12( 0.5F * tex->v_scale + tex->v_offset ) ;
	uvrgb->q = 4096;
	uvrgb->f = 0x8fff;
	uvrgb->r = BASE_R;
	uvrgb->g = BASE_G;
	uvrgb->b = BASE_B;
	uvrgb->a = param->alpha;
	uvrgb++;
	
	uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb->v = FTOI12( 0.5F * tex->v_scale + tex->v_offset ) ;
	uvrgb->q = 4096;
	uvrgb->f = 0x8fff;
	uvrgb->r = BASE_R;
	uvrgb->g = BASE_G;
	uvrgb->b = BASE_B;
	uvrgb->a = param->alpha;
	uvrgb++;
	
	uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb->q = 4096;
	uvrgb->f = 0x0fff;
	uvrgb->r = BASE_R;
	uvrgb->g = BASE_G;
	uvrgb->b = BASE_B;
	uvrgb->a = param->alpha;
	uvrgb++;
	
	uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
	uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
	uvrgb->q = 4096;
	uvrgb->f = 0x0fff;
	uvrgb->r = BASE_R;
	uvrgb->g = BASE_G;
	uvrgb->b = BASE_B;
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





static int InitPacket2( Work *work )
{

    DG_PRIM2		*prim;
    DG_TEX		*tex;
    FVECTOR		*pos;
    DG_PRIM2_UVRGB	*uvrgb;
    int			i;
    float		width;
    Blood_Param		*param;


    prim = work->prim2 = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
    if ( prim == NULL ) {
		return 0;
    }

    //work->prim2->root = &work->matrix;

    tex = DG_GetTexture( GV_StrCode( "chi03_alp" ) );
    DG_ConfigPrim2Tex( prim, tex );
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );


    pos   = SCR_POS;
    uvrgb = SCR_UVS;
    param = work->param_part;
	
    for ( i = 0 ; i < N_PRIMS ; i++ ) {


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
	width = frnd()*100.0f;
	param->vec.vx = 1200.0f + frnd()*400.0f;
	param->vec.vy = width;
	param->vec.vz = 0.0f;
	_sceVu0ScaleVector( &param->vec, &param->vec, 0.25f );



	param->alpha = irnd()%ALPHA;


	switch (i%4) {
	case 0:
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x8fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = 0;
		uvrgb++;
	
		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x8fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = 0;
		uvrgb++;
	
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = 0;
		uvrgb++;
	
		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = 0;
		uvrgb++;
		break;
	case 1:
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x8fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = 0;
		uvrgb++;
	
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x8fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = 0;
		uvrgb++;
	
		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = 0;
		uvrgb++;
	
		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = 0;
		uvrgb++;
		break;
	case 2:
		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x8fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = 0;
		uvrgb++;
	
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x8fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = 0;
		uvrgb++;
	
		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = 0;
		uvrgb++;
	
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = 0;
		uvrgb++;
		break;
	case 3:
		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x8fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = 0;
		uvrgb++;
	
		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x8fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = 0;
		uvrgb++;
	
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = 0;
		uvrgb++;
	
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = BASE_R;
		uvrgb->g = BASE_G;
		uvrgb->b = BASE_B;
		uvrgb->a = 0;
		uvrgb++;
		break;

	}


	param++;

    }	
    
    OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
    OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS );
    OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );
    OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS );
 
    return 1;

}



/* 柴田作 CV2頂点検索関数 */
float GetCV2NearPosAndLen2( FVECTOR *out, CV2_DEF *def, FMATRIX *root, FVECTOR *pcheck, int joint )
{

	CV2_MDL		*mdl;
	int 		i = 0;
	float		len = FLOAT_MAX;
	FVECTOR		*verts;
	FMATRIX		world;

	if( def->n_models <= joint ){ printf( "ERR!! JOINT\n" ); return 0.0f; }

	mdl = &def->models[joint];
	verts = mdl->verts;
	_sceVu0InversMatrix( &world, root );

	//ローカルに
// yano /*未検証*/
#ifdef BP_PSX2_ASM
	asm volatile ("
		lqc2				vf8,0x00(%1)
		lqc2				vf4,0x00(%0)
		lqc2				vf5,0x10(%0)
		lqc2				vf6,0x20(%0)
		lqc2				vf7,0x30(%0)
		vmulax.xyzw			ACC, vf4,vf8
		vmadday.xyzw			ACC, vf5,vf8
		vmaddaz.xyzw			ACC, vf6,vf8
		vmaddw.xyzw			vf8, vf7,vf8
	": : "r"(&world), "r"(pcheck) );//:"memory" );
#else
	_sceVu0ApplyMatrix( pcheck, &world, pcheck );
#endif
	
	for( i = 0; i < mdl->n_verts; i++ ){
		FVECTOR		fvtemp;

// yano　/*未検証*/
#ifdef BP_PSX2_ASM
		asm volatile ("
			lqc2				vf09,0x00(%1)

			vsub.xyzw			vf10, vf09, vf08
			vmul.xyz			vf11, vf10, vf10
			vmulax.w			ACC,  vf00, vf11x
			vmadday.w			ACC,  vf00, vf11y
			vmaddz.w			vf12, vf00, vf11z

			sqc2				vf12,0x00(%0)
		": : "r"(&fvtemp), "r"(verts) : "memory" );
#else
		fvtemp.vx = verts->vx - pcheck->vx;
		fvtemp.vx = fvtemp.vx * fvtemp.vx;
		fvtemp.vy = verts->vy - pcheck->vy;
		fvtemp.vy = fvtemp.vy * fvtemp.vy;
		fvtemp.vz = verts->vz - pcheck->vz;
		fvtemp.vz = fvtemp.vz * fvtemp.vz;
		fvtemp.vw = fvtemp.vx + fvtemp.vy + fvtemp.vz;
#endif

		if( len > fvtemp.vw ){
			len = fvtemp.vw;

			/* 絶対化 */
			/*asm volatile ("

		lqc2				vf16,0x00(%2)
		lqc2				vf04,0x00(%1)
		lqc2				vf05,0x10(%1)
		lqc2				vf06,0x20(%1)
		lqc2				vf07,0x30(%1)
		vmulax.xyzw			ACC, vf04,vf16
		vmadday.xyzw			ACC, vf05,vf16
		vmaddaz.xyzw			ACC, vf06,vf16
		vmaddw.xyzw			vf16, vf07,vf16

		sqc2				vf16,0x00(%0)
		": : "r"(out), "r"(root), "r"(verts) );//:"memory" );*/	

			DG_COPY_VEC( out, verts );
		}
		verts++;
	}
	
	return len;

}




static int GetResources( Work *work, FVECTOR *damage_pos, FVECTOR *vec, FMATRIX *mat, int joint, int model_name )
{

    FVECTOR		vectmp;
	FMATRIX		matInv;

	work->mat = mat;

	/* 放出方向の相対ベクトル算出 */
    FastInverseMatrix( &matInv, work->mat );
	_sceVu0SubVector( &vectmp, &DG_ZeroVector, vec );
    //_sceVu0ApplyMatrix( &vectmp, &matInv, vec );
    _sceVu0ApplyMatrix( &vectmp, &matInv, &vectmp );
	_sceVu0Normalize( &vectmp, &vectmp );
	
	/* 基準関節に対しての回転を確保 */
	OK_DirVecXY( &DG_ZeroVector, &vectmp, &work->rot );


	/* 血煙中心算出 */
	MakeSplashMatrix( work );

	/* 血煙算出 */
	NewBloodSmokeInpact( (FVECTOR *)work->mat->m[3] );

	work->init_alpha = ALPHA;
	work->init_speed = SPEED;
	work->life       = 0;


	/* ちょっとテストしてみる */
	{
	    CV2_DEF	*def;
	    FVECTOR	out;
	    FVECTOR	out_rot;

	    def = GV_GetCache( GV_CacheID( model_name, 'c' ) );

	    if ( def == NULL ) {
			/* CV2ないよん */
			printf("point_splash_blood.c : cv2_data(%d) not fount\n", model_name );
			ASSERT(0);			
	    }
	    else {	    

			/* 仮ダメージ位置から斬られ位置算出 */
			//AN_Test_Eye2( damage_pos, 500.f );

			GetCV2NearPosAndLen2( &out, def, mat, damage_pos, joint );
			out.vw = 1.0f;

			DG_SetPos( mat );
			DG_PutVector( &out, &out_rot, 1 );
			//AN_Test_Eye2( &out_rot, 500.f );

			out.vx = 0.0f;
			out.vz = 0.0f;

			DG_COPY_VEC( &work->splash_point, &out );				
		
		
	    }
	    
	}

	if ( !( InitPacket( work ) ) ) {
	    return -1;
	}

	if ( !( InitPacket2( work ) ) ) {
	    return -1;
	}


	return 0;

}



void *NewPointSplashBlood( FVECTOR *damage_pos, FVECTOR *vec, FMATRIX *mat, int joint, int model_name )
{
	Work		*work ;

	if ( GM_Configuration & GM_CONFIG_BLOOD_OFF ) {
	    return NULL;
	}

	
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {

		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor )
		if ( GetResources( work, damage_pos, vec, mat, joint, model_name ) < 0 ) {
			GV_DestroyActor( work );
			return NULL ;
		}
	}
	return (void *)work ;

}


