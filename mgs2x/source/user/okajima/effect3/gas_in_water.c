//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
#if 0
﻿/*
	gas_in_water.c
	水中の濁り（透視変換バージョン）：カメラ高速移動対応

	2001/06/12 S.Okajima
	$Id: gas_in_water.c,v 1.4 2002/11/23 12:24:51 Yoshizawa1 Exp $
*/
#endif
#ifdef PSX2 ///
#include <sys/types.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>

#include	"camera.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;

/*----------------------------------------------------------------*/
#define	RAISE			( 0 )

#define	SCREEN_NEAR			( 51.0f )

#define	SCR_SPACE	(SCRPAD_ADDR)
#define	SCR_SPACE2	(SCRPAD_ADDR + 0x1000)
#define	SCR_POS		(SCRPAD_ADDR + 0x2000)

#define	N_VERTS			(64)
//#define	N_VERTS			(4)
#define	N_POLYS			(N_VERTS/4)
#define	N_PRIMS			(1)


/* 画面前バンダリ */
#define	BOUND_WIDTH			(6000.0f)
#define	BOUND_WIDTH_DOUBLE	(BOUND_WIDTH*2.0f)
/* バウンドの中心と、視点との距離 */
#define	CENTER_DISTANCE	(4500.0f)
//#define	CENTER_DISTANCE	(BOUND_WIDTH)
/* 手前フェードインの距離 */
#define	FADE_IN_DISTANCE	(CENTER_DISTANCE * 0.5f)

#define	MOST_FAR		( CENTER_DISTANCE + BOUND_WIDTH*1.4143f )
#define	MOST_FAR_CLIP	( CENTER_DISTANCE )

#define	WIDTH		(50.0f * CENTER_DISTANCE)
//#define	WIDTH		(10.0f * CENTER_DISTANCE)

#define	SPEED_MIN	(2.0f)
#define	SPEED_RND	(3.0f)

typedef	struct	{
	GV_ACT_EX	actor ;
	int			name;

	CVECTOR		col;
	int			num;

	int		mode;
	int		on_flag;
	int		before_flag;

	FVECTOR before_cam;

	FVECTOR bound_0;
	FVECTOR bound_1;

	FVECTOR space_pos[N_POLYS * N_PRIMS];
	FVECTOR space_vec[N_POLYS * N_PRIMS];

	DG_PRIM2	*prim ;
} Work ;

/*----------------------------------------------------------------*/
static void CalcVerts( FVECTOR *space, FVECTOR *space2, FVECTOR *pos )
{
	float	rs;
	float	rc;
	float	radius;

	radius = WIDTH / space2->vw;
	rs = radius * sinf( space->vw );
	rc = radius * cosf( space->vw );

	pos->vx = space2->vx + rs;
	pos->vy = space2->vy + rc;
	pos->vz = SCREEN_NEAR;
	pos++;

	pos->vx = space2->vx - rc;
	pos->vy = space2->vy + rs;
	pos->vz = SCREEN_NEAR;
	pos++;

	pos->vx = space2->vx + rc;
	pos->vy = space2->vy - rs;
	pos->vz = SCREEN_NEAR;
	pos++;

	pos->vx = space2->vx - rs;
	pos->vy = space2->vy - rc;
	pos->vz = SCREEN_NEAR;
	pos++;

}

/*----------------------------------------------------------------*/
enum {
	REQ_OFF=0,
	REQ_ON,
	REQ_KILL,
	REQ_NO
};

static void CheckMesgParam( Work *work )
{
	GV_MSG *msg;
	int mes_num;
	int num;

	mes_num=GV_ReceiveMessage( work->name, &msg );
	msg += mes_num-1;
	while( --mes_num >= 0 ){
		num=msg->message[0];
		switch( num ){
		  case REQ_OFF:
			work->on_flag = 0;
			break;
		  case REQ_ON:
			work->on_flag = 1;
			break;
		  case REQ_KILL:
			GV_DestroyActor( work ) ;
			break;
		  default:
			break;
		}
		msg--;
	}
}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	clock;
	int	i;
	int		alpha;
	DG_PRIM2		*prim;
	DG_PRIM2_UVRGB	*uvrgb ;	/* スプライト用 */
	FVECTOR	*pos;
	FVECTOR	*space_vec;
	FVECTOR	*space;
	FVECTOR	*space2;
	FVECTOR	fvtemp;
	FVECTOR	fvtemp1;
	FVECTOR	bound_0;
	FVECTOR	bound_1;
	FVECTOR	b_dis;
	FVECTOR	cam_pos;
	FVECTOR	cam_z;
	float	screen_near_x;
	float	screen_near_y;
	float	alpha_max;
	float	ftemp;
	float	ft_x;
	float	ft_y;

	CheckMesgParam( work );

	DG_COPY_VEC( &cam_pos, (FVECTOR *)DG_Chanls->eye.m[3] );

	prim = work->prim;
	if( (!work->on_flag)
	 || (cam_pos.vy > GM_WaterLevel) ){
		DG_InvisiblePrim2( prim );
		work->before_flag = 1;
		return;
	}

	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_VisiblePrim2( prim );

	_sceVu0SubVector( &fvtemp, &cam_pos, &work->before_cam) ;
	if( GV_VecLen3F( &fvtemp ) > BOUND_WIDTH_DOUBLE ) work->before_flag = 1;


	/* カメラのまえのバンダリをつくる */
	b_dis.vx=0.0f;
	b_dis.vy=0.0f;
	b_dis.vz=CENTER_DISTANCE;
	DG_SetPos( &DG_Chanls->eye );
	DG_PutVector( &b_dis, &b_dis, 1 );

#if 0
	_sceVu0ScaleVector( &bound_0, &work->bound_0, 2.0f/DG_Chanls->screen );
	_sceVu0ScaleVector( &bound_1, &work->bound_1, 2.0f/DG_Chanls->screen );
	_sceVu0AddVector( &bound_0, &bound_0, &b_dis ) ;
	_sceVu0AddVector( &bound_1, &bound_1, &b_dis ) ;
#else
	_sceVu0AddVector( &bound_0, &work->bound_0, &b_dis ) ;
	_sceVu0AddVector( &bound_1, &work->bound_1, &b_dis ) ;
#endif
//	NewBoundingBoxView( &bound_1, &bound_0, 255, 128, 64 ) ;

	cam_z.vx = 0.0f;
	cam_z.vy = 0.0f;
	cam_z.vz = 1.0f;
	DG_RotVector( &cam_z, &cam_z, 1 );




	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	screen_near_x = SCREEN_NEAR / (ASPECT_X()                                       ) / DG_Chanls->screen;
	screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / DG_Chanls->screen;

	OK_Mem_Scr( SCR_SPACE,  work->space_pos, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
	OK_Mem_Scr( SCR_SPACE2, work->space_vec, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;

	space     = SCR_SPACE;
	space_vec = SCR_SPACE2;
	for ( i = 0 ; i < N_PRIMS*N_POLYS ; i++ ){
		space->vx+= space_vec->vx;
		space->vy+= space_vec->vy;
		space->vz+= space_vec->vz;
		space++;
		space_vec++;
	}

	if( work->before_flag ){
		space     = SCR_SPACE;
		for ( i = 0 ; i < N_PRIMS*N_POLYS ; i++ ){
			space->vx = bound_1.vx + BOUND_WIDTH_DOUBLE * rnd();
			space->vy = bound_1.vy + BOUND_WIDTH_DOUBLE * rnd();
			space->vz = bound_1.vz + BOUND_WIDTH_DOUBLE * rnd();
			space++;
		}
	}


	/* 座標を全部透視変換する */
	DG_SetPos( &DG_Chanls->eye_pers );
	DG_PutVector( SCR_SPACE, SCR_SPACE2, N_POLYS * N_PRIMS );



	DG_SetPos( &DG_Chanls->eye );
	alpha_max = (float)work->col.cd;
	uvrgb = prim->uvrgb[ clock ];
	space = SCR_SPACE;
	space2= SCR_SPACE2;
	pos   = SCR_POS;
	for ( i = 0 ; i < N_PRIMS*N_POLYS ; i++ ){
		if( space2->vz < space2->vw ){
			space2->vw = (space2->vw  > 0.0f)? space2->vw: -space2->vw;

			if( space2->vw > MOST_FAR_CLIP ){ // カメラ前方の奥に行ったので、カメラ座標に生成
				space->vx = BOUND_WIDTH*frnd();
				space->vy = BOUND_WIDTH*frnd();
				space->vz = SCREEN_NEAR;
				ftemp = space->vw;
				DG_PutVector( space, space, 1 );
				space->vw = ftemp;
				DG_COPY_VEC( pos, space2 );	pos++;
				DG_COPY_VEC( pos, space2 );	pos++;
				DG_COPY_VEC( pos, space2 );	pos++;
				DG_COPY_VEC( pos, space2 );	pos++;
				uvrgb+= 4;
			}else{	// カメラ前にあるので通常処理
				ft_x = space2->vx / space2->vw;
				ft_y = space2->vy / space2->vw;
				if( !vu0_CheckBoundingBox( space, &bound_0, &bound_1 ) ){
//				if( DG_FABS(ft_x) > 3.0f
//				 || DG_FABS(ft_y) > 3.0f ){
//if(i==0)printf("a0:%f %f %f\n",space->vx,space->vy,space->vz);
//if(i==0)AN_Test_Eye2( space, 2 );
					_sceVu0SubVector( &fvtemp, space, &cam_pos ) ;
//if(i==0)printf("a1:%f %f %f\n",fvtemp.vx,fvtemp.vy,fvtemp.vz);
					ftemp = _sceVu0InnerProduct( &cam_z, &fvtemp );
					_sceVu0ScaleVector( &fvtemp1, &cam_z, ftemp );
//if(i==0)printf("a2:%f %f %f\n",fvtemp1.vx,fvtemp1.vy,fvtemp1.vz);
					_sceVu0SubVector( &fvtemp1, &fvtemp1, &fvtemp ) ;
					_sceVu0ScaleVector( &fvtemp1, &fvtemp1, 1.5f+0.4f*rnd() );	// 2.0f で 等距離左右反転
					space->vx+= fvtemp1.vx ;
					space->vy+= fvtemp1.vy ;
					space->vz+= fvtemp1.vz ;
//if(i==0)printf("b0:%f %f %f\n",space->vx,space->vy,space->vz);
//if(i==0)AN_Test_Eye2( space, 2 );
				}
				if( space->vy < GM_WaterLevel - 1000.0f){
//					space2->vz = SCREEN_NEAR;
					space2->vx = screen_near_x * ft_x;
					space2->vy = screen_near_y * ft_y;

					CalcVerts( space, space2, pos );
					pos+=4;

					if( space2->vw < FADE_IN_DISTANCE ){
						alpha = (int)(alpha_max * space2->vw / FADE_IN_DISTANCE);
					}else{
						alpha = (int)(alpha_max * (1.0f - (space2->vw - FADE_IN_DISTANCE) / (MOST_FAR_CLIP - FADE_IN_DISTANCE)));
					}
					(uvrgb++)->a = alpha;
					(uvrgb++)->a = alpha;
					(uvrgb++)->a = alpha;
					(uvrgb++)->a = alpha;
				}else{
					DG_COPY_VEC( pos, space2 );	pos++;
					DG_COPY_VEC( pos, space2 );	pos++;
					DG_COPY_VEC( pos, space2 );	pos++;
					DG_COPY_VEC( pos, space2 );	pos++;
					uvrgb+= 4;
				}
			}
		}else{	// カメラより手前に来たのでカメラ前方の奥に生成
			space->vx = BOUND_WIDTH*frnd();
			space->vy = BOUND_WIDTH*frnd();
			space->vz = MOST_FAR_CLIP ;
			ftemp = space->vw;
			DG_PutVector( space, space, 1 );
			space->vw = ftemp;

			DG_COPY_VEC( pos, space2 );	pos++;
			DG_COPY_VEC( pos, space2 );	pos++;
			DG_COPY_VEC( pos, space2 );	pos++;
			DG_COPY_VEC( pos, space2 );	pos++;
			uvrgb+= 4;
		}
		space++;
		space2++;
	}
	OK_Scr_Mem( work->space_pos,      SCR_SPACE, sizeof(FVECTOR),        N_VERTS * N_PRIMS ) ;
	OK_Scr_Mem( prim->pos[ clock ],   SCR_POS,   sizeof(FVECTOR),        N_VERTS * N_PRIMS ) ;


	work->before_flag = 0;

	DG_COPY_VEC( &work->before_cam, &cam_pos );


}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB		*uvrgb0 ;
	DG_PRIM2_UVRGB		*uvrgb1 ;
	int		i ;
	int		col_r;
	int		col_g;
	int		col_b;
	int		col_a;

	col_r = work->col.r;
	col_g = work->col.g;
	col_b = work->col.b;
	col_a = work->col.cd;

	DG_ConfigPrim2Tex( prim, tex );

	if( work->mode==0 ){
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}else if( work->mode==1 ){
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	}else if( work->mode==2 ){
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	}else if( work->mode==3 ){
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 1, 0, 0, 1, 0x00 ) );
	}

//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 1, 2, 2, 1, 0xff ) );

	prim->raise = RAISE;

	uvrgb0 = prim->uvrgb[0] ;
	uvrgb1 = prim->uvrgb[1] ;
	for ( i = 0 ; i < N_PRIMS*N_POLYS ; i++ ){
		uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x8fff ;
		uvrgb0->r = uvrgb1->r = col_r ;
		uvrgb0->g = uvrgb1->g = col_g ;
		uvrgb0->b = uvrgb1->b = col_b ;
		uvrgb0->a = uvrgb1->a = col_a ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x8fff ;
		uvrgb0->r = uvrgb1->r = col_r ;
		uvrgb0->g = uvrgb1->g = col_g ;
		uvrgb0->b = uvrgb1->b = col_b ;
		uvrgb0->a = uvrgb1->a = col_a ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x0fff ;
		uvrgb0->r = uvrgb1->r = col_r ;
		uvrgb0->g = uvrgb1->g = col_g ;
		uvrgb0->b = uvrgb1->b = col_b ;
		uvrgb0->a = uvrgb1->a = col_a ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x0fff ;
		uvrgb0->r = uvrgb1->r = col_r ;
		uvrgb0->g = uvrgb1->g = col_g ;
		uvrgb0->b = uvrgb1->b = col_b ;
		uvrgb0->a = uvrgb1->a = col_a ;
		uvrgb0++;
		uvrgb1++;
	}

	return 1;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	FVECTOR		*space_pos;
	FVECTOR		*space_vec;
	SVECTOR		rot;
	int	i;

	work->mode  = 0;
	if ( GCL_GetOption( 'm' ) != NULL ){
		work->mode  = GCL_GetNextInt();
	}

	work->col.r = 128;
	work->col.g = 196;
	work->col.b = 255;
	work->col.cd = 32;
	if ( GCL_GetOption( 'c' ) != NULL ){
		work->col.r  = GCL_GetNextInt();
		work->col.g  = GCL_GetNextInt();
		work->col.b  = GCL_GetNextInt();
		work->col.cd = GCL_GetNextInt();
	}

	work->on_flag = 1 ;
	work->before_flag = 1;

	/* 表示範囲 */
	work->bound_0.vx =  BOUND_WIDTH*4.0f ;
	work->bound_0.vy =  BOUND_WIDTH*4.0f ;
	work->bound_0.vz =  BOUND_WIDTH*4.0f ;
	work->bound_1.vx = -BOUND_WIDTH*4.0f ;
	work->bound_1.vy = -BOUND_WIDTH*4.0f ;
	work->bound_1.vz = -BOUND_WIDTH*4.0f ;

//	tex = DG_GetTexture( 15625989 /*"blood_2bw_alp"*/ );
//	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
	tex = DG_GetTexture( 12244206 /*"fog02_msk"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
//	tex = DG_GetTexture( 10972307 /*"chi01_alp"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	if( !InitPacket2( work, prim, tex ) ) return -1;
	DG_InvisiblePrim2( prim );


	space_pos = work->space_pos;
	space_vec = work->space_vec;
	for ( i = 0 ; i < N_PRIMS*N_POLYS ; i++ ){
		space_vec->vx = 0.0f;
		space_vec->vy = 0.0f;
		space_vec->vz = SPEED_MIN + SPEED_RND*rnd();
		rot.vx = ((irnd()>>8)&2047)-1024;
		rot.vy = ((irnd()>>8)&4095);
		rot.vz = 0;
		DG_SetPos2( &DG_ZeroVector, &rot );
		DG_RotVector( space_vec, space_vec, 1 );

		space_pos->vw = TPI*rnd();
		space_pos++;
		space_vec++;
	}

	return 0 ;
}

#ifdef KP_WINDOWS
static	void	DmyAct( Work *work ){ CheckMesgParam(work) ; }
static	void	DmyDie( Work *work ){}
#endif

void *NewGasInWater( int name, int where )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
#ifndef KP_WINDOWS
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->name = name;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
#else
		if( DG_GetInWaterEffectLevel() > 0x40 )		// Levelが一定以上ならば
		{
			GV_SetActor( &( work->actor ), Act, Die ) ;
			GV_ActorEX( &work->actor )

			work->name = name;

			if ( GetResources( work ) < 0 ) {
				GV_DestroyActor( work ) ;
				return NULL ;
			}
		}
		else
		{
			GV_SetActor( &( work->actor ), DmyAct, DmyDie ) ;
			GV_ActorEX( &work->actor )
			work->name = name;
		}
#endif
	}
	return (void *)work ;
}

