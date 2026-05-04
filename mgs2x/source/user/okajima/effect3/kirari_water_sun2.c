//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	kirari_water_sun2.c
	海水きらきら

	2001/06/05 S.Okajima
	$Id: kirari_water_sun2.c,v 1.1.1.3 2002/11/19 11:47:33 Yoshizawa1 Exp $
*/
//mojicode 

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
#define	SCREEN_ANGLE_MAX	( 24.0f )
#define	ZOOM_UP_RATIO	( 4.0f )

#define	RAISE			( 30000 )

#define	SCREEN_NEAR			( 51.0f )

#define	SCR_POS		(SCRPAD_ADDR)

//#define	N_NUM			(2)				/* ２枚で十字を作る */
#define	N_NUM			(4)				/* ４枚で十字を作る */
#define	N_VERTS			(64)
//#define	N_PRIMS			(0x4000/16/N_VERTS)
#define	N_PRIMS			(16)
#define	N_POLYS			(N_VERTS/4)
#define	N_PARTS			(N_PRIMS*N_POLYS/N_NUM)	/* 十字の数 */

#define	WIDTH_RATIO		(0.60f)
#define	WIDTH_RATIO_ADD	(0.25f)

#define	KEY_RATIO		(0.03f)
#define	SHORT_RATIO		(0.7f)
#define	CORE_RATIO		(KEY_RATIO / WIDTH_RATIO)
#define	DOUBLE_RATIO	(0.1f)
#define	DOUBLE_RATIO_S	(0.3f)

#define	SCREEN_WIDTH_RATIO	(0.2f)
#define	SCREEN_WIDTH_MIN	(0.1f)
#define	NEAR_LIMIT			(2000.0f)

#define	FREQUENCE		(0.005f)
//#define	FREQUENCE		(0.1f)

#define	RADIUS_ALPHA_MIN	(0.2f)
#define	RADIUS_ALPHA_MAX	(1.0f)

#define	CAMERA_MARGINE	(30.0f)

#define	PARAM_ADD	(1.0f / 32.0f)
#define	PARAM_SUB	(1.0f / 4.0f)

typedef	struct	{
	GV_ACT_EX	actor ;

	FMATRIX		*world;
	FVECTOR		before_cam_pos;
	FVECTOR		pos;
	FVECTOR		cal_pos[N_PARTS];
	int			parts_flag[N_PARTS];
	float		parts_param[N_PARTS];

	float		screen_angle_before;
	float		before_param;
	float		before_angle;
	float		alpha_max;

	DG_PRIM2	*prim;
	DG_PRIM2	*prim_s;

	int			*col;
} Work ;


/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2_UVRGB	*uvrgb;
	DG_PRIM2_UVRGB	*uvrgb_s;
	DG_PRIM2	*prim;
	FVECTOR		*cal_pos;
	FVECTOR		*cal_pos_after;
	FVECTOR		*pos;
	FVECTOR		*pos_s;
	FVECTOR		d_x;
	FVECTOR		d_y;
	FVECTOR		l_x;
	FVECTOR		l_y;
	FVECTOR		cam_vec;
	FVECTOR		cam_pos;
	FVECTOR		fvtemp2;
	FVECTOR		fvtemp3;
	FVECTOR		sun_pos;
	FVECTOR		sun_pos_mirror;
	FVECTOR		sun_pos_mirror_pre;
	FVECTOR		near_limit;
	FMATRIX		fmat_eye;
	float	screen_near_x;
	float	screen_near_x_pre;
	float	screen_near_y;
	float	alp_sc_min;
	float	alp_sc_max;
	float	screen_angle;
	float	screen_angle_ratio;
	float	ftemp0;
	float	ftemp1;
	float	ftemp2;
	float	limit_y0;
	float	limit_y1;
	float	limit_diff;
	float	alpha_max;
	float	*parts_param;
	int		*parts_flag;
	int	clock;
	int	alpha=0;
	int	alpha2;
	int	flag;
	int	i;
	int	col_r;
	int	col_g;
	int	col_b;

	i = (*work->col);
#if 0
	col_r     = (i>>24)&255;
	col_g     = (i>>16)&255;
	col_b     = (i>> 8)&255;
#else
	col_r = (int)(DG_FogColor.r)&255;
	col_g = (int)(DG_FogColor.g)&255;
	col_b = (int)(DG_FogColor.b)&255;
#endif
	work->alpha_max = (float)(i&255);

	screen_angle = DG_Chanls->screen;
	if( screen_angle > SCREEN_ANGLE_MAX ) screen_angle = SCREEN_ANGLE_MAX;
	screen_angle_ratio = (SCREEN_ANGLE_MAX - screen_angle) / (SCREEN_ANGLE_MAX - 2.0f);
	if( screen_angle_ratio < 0.3f ) screen_angle_ratio = 0.3f;
	alpha_max  = work->alpha_max * screen_angle_ratio;
//	alpha_max  = work->alpha_max;
	screen_angle_ratio = (1.0f - screen_angle_ratio)*ZOOM_UP_RATIO + 1.0f;
	screen_angle_ratio*= 0.02f;
//	screen_angle_ratio = 1.0f;

	/* 太陽の位置 */
	DG_COPY_VEC( &sun_pos, (FVECTOR *)work->world->m[3] );
	DG_COPY_VEC( &sun_pos_mirror, &sun_pos );
	/*  カメラの位置*/
	DG_COPY_MAT( &fmat_eye ,&DG_Chanls->eye );
	DG_COPY_VEC( &cam_pos, (FVECTOR *)fmat_eye.m[3] );
	if( DG_FABS(cam_pos.vy - work->before_cam_pos.vy) < CAMERA_MARGINE ){
		cam_pos.vy = work->before_cam_pos.vy;
		DG_COPY_VEC( (FVECTOR *)fmat_eye.m[3], &cam_pos );
	}

	cam_vec.vx = 0.0f;
	cam_vec.vy = 0.0f;
	cam_vec.vz = 1.0f;
	DG_SetPos( &DG_Chanls->eye );
	DG_RotVector( &cam_vec, &cam_vec, 1 );

	_sceVu0SubVector( &fvtemp2, &sun_pos, &cam_pos ) ;
	if( _sceVu0InnerProduct( &fvtemp2, &cam_vec ) < 0.0f ){
		DG_InvisiblePrim2( work->prim );
		DG_InvisiblePrim2( work->prim_s );
		return;
	}

	if( OK_ItemVisionCheck() ){
		DG_InvisiblePrim2( work->prim );
		DG_InvisiblePrim2( work->prim_s );
	}else{
		DG_VisiblePrim2( work->prim );
		DG_VisiblePrim2( work->prim_s );
	}



	DG_SetPos( &fmat_eye );

	fvtemp2.vx = 0.0f;
	fvtemp2.vy = 0.0f;
	fvtemp2.vz = 1.0f;
	DG_RotVector( &fvtemp2, &fvtemp2, 1 );

	work->before_param = cam_vec.vy;
	work->screen_angle_before = screen_angle;


	screen_near_x_pre = SCREEN_NEAR / (ASPECT_X()                                       );
	screen_near_x     = screen_near_x_pre / screen_angle;
	limit_diff        = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height);
	screen_near_y     = limit_diff / screen_angle;



	screen_near_x_pre*= 0.5f;

	d_x.vx = screen_near_x;
	d_x.vy = 0.0f;
	d_x.vz = 0.0f;
	d_y.vx = 0.0f;
	d_y.vy = screen_near_y;
	d_y.vz = 0.0f;
	DG_RotVector( &d_x, &d_x, 1 );
	DG_RotVector( &d_y, &d_y, 1 );

	_sceVu0AddVector( &l_x, &d_x, &d_y ) ;
	_sceVu0SubVector( &l_y, &d_x, &d_y ) ;


	//------------鏡面
	ftemp0 = sun_pos_mirror.vy - GM_WaterLevel;
	sun_pos_mirror.vy-= ftemp0*2.0f;

	_sceVu0SubVector( &fvtemp2, &sun_pos_mirror, &cam_pos ) ;
	_sceVu0ScaleVector( &fvtemp3, &fvtemp2, (cam_pos.vy - GM_WaterLevel)/(cam_pos.vy - sun_pos_mirror.vy) );

	_sceVu0AddVector( &sun_pos_mirror, &fvtemp3, &cam_pos ) ;
//	AN_Test_Eye2( &sun_pos_mirror, 2 );

	_sceVu0Normalize( &fvtemp2, &fvtemp2 ) ;
	_sceVu0ScaleVector( &fvtemp3, &fvtemp2, NEAR_LIMIT );
	_sceVu0AddVector( &near_limit, &fvtemp3, &cam_pos ) ;
	near_limit.vy = GM_WaterLevel;


	DG_SetPos( &DG_Chanls->eye_pers );

	DG_PutVector( &sun_pos, &sun_pos, 1 );
	sun_pos.vx*= screen_near_x/sun_pos.vw;
	sun_pos.vy*= screen_near_y/sun_pos.vw;

	DG_PutVector( &sun_pos_mirror, &sun_pos_mirror, 1 );
	DG_COPY_VEC( &sun_pos_mirror_pre, &sun_pos_mirror );
	sun_pos_mirror.vx*= screen_near_x/sun_pos_mirror.vw;
	sun_pos_mirror.vy*= screen_near_y/sun_pos_mirror.vw;
	sun_pos_mirror_pre.vx/= sun_pos_mirror.vw;
	sun_pos_mirror_pre.vy/= sun_pos_mirror.vw;

	DG_PutVector( &near_limit, &near_limit, 1 );
	near_limit.vx*= screen_near_x/near_limit.vw;
	near_limit.vy*= screen_near_y/near_limit.vw;

	limit_y0 = sun_pos.vy*0.45f + sun_pos_mirror.vy*0.55f;	// 水平線

	if( limit_y0 > screen_near_y ){
		DG_InvisiblePrim2( work->prim );
		DG_InvisiblePrim2( work->prim_s );
		return;
	}
	limit_y1 = near_limit.vy;





	if( limit_y0 <-screen_near_y ){
		limit_y0 =-screen_near_y;
	}
	if( limit_y1 > screen_near_y ){
		limit_y1 = screen_near_y;
	}


	//--------------------------------------------
	// 十字
	prim = work->prim;
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );

	GM_GroupPrim2( work->prim_s, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( work->prim_s );

	clock = prim->buffer_clock;


	DG_PutVector( work->cal_pos, SCR_POS, N_PARTS );
	cal_pos_after = SCR_POS;


	DG_SetPos( &fmat_eye );

	alp_sc_min = screen_near_x * RADIUS_ALPHA_MIN;
	alp_sc_max = screen_near_x * RADIUS_ALPHA_MAX;

	cal_pos       = work->cal_pos;
	cal_pos_after = SCR_POS;
	pos          = prim->pos[  clock];
	uvrgb        = prim->uvrgb[  clock];
	pos_s        = work->prim_s->pos[  clock];
	uvrgb_s      = work->prim_s->uvrgb[  clock];
	parts_param  = work->parts_param;
	parts_flag   = work->parts_flag;
	for ( i=0; i<N_PARTS; i++ ){
		ftemp2 = cal_pos_after->vy*screen_near_y/cal_pos_after->vw;
		ftemp1 = DG_FABS(cal_pos_after->vx/cal_pos_after->vw);

		if( rnd() < FREQUENCE ) (*parts_flag) = 1;

		flag = 0;
		if( (*parts_flag) ){
			(*parts_param)-= PARAM_SUB;
			if( (*parts_param) < 0.0f ){
				flag = 1;
			}
		}else if( (ftemp1 > 1.0f )
		 || (ftemp2 < limit_y0 )
		 || (ftemp2 > limit_y1 )
		  ){
			flag = 1;
		}else{
			fvtemp2.vx = cal_pos_after->vx / cal_pos_after->vw;
			fvtemp2.vy = cal_pos_after->vy / cal_pos_after->vw;
			ftemp1 = 1.0f - DG_FABS(fvtemp2.vx - sun_pos_mirror_pre.vx)*3.0f/screen_angle;	// スクリーン座標系での、鏡像からの距離比Ｘ
			ftemp2 = 1.0f - DG_FABS(fvtemp2.vy - sun_pos_mirror_pre.vy)*1.0f/screen_angle;	// スクリーン座標系での、鏡像からの距離比Ｙ
			ftemp0 = ftemp1 * ftemp2;
			if( (ftemp1 < 0.0f)
			 || (ftemp2 < 0.0f)
			 || (rnd()*4.0f < 1.0f - ftemp0)
			  ){
				flag = 1;
			}else{
//				ftemp0*= ftemp0;
				if( (*parts_param) < 0.25f ) (*parts_param) = rnd()*0.25f;
				(*parts_param)+= PARAM_ADD;
				if( (*parts_param) > 1.0f ) (*parts_param) = 1.0f;
				alpha = (int)(alpha_max * (*parts_param) * ftemp0);
			}
		}


		if( flag ){
			ftemp2 = rnd()*frnd();
			if( ftemp2 < 0.25f
			 && ftemp2 >-0.25f ){
				ftemp1 = cosf( PI*ftemp2*2.0f );
				ftemp1*= ftemp1;
			}else{
				ftemp1 = 0.0f;
			}

			cal_pos_after->vx = sun_pos_mirror.vx + (SCREEN_WIDTH_MIN + SCREEN_WIDTH_RATIO*ftemp1) * screen_near_x_pre * frnd();
			cal_pos_after->vy = sun_pos_mirror.vy + limit_diff*ftemp2;
			cal_pos_after->vz = SCREEN_NEAR;

			DG_PutVector( cal_pos_after, cal_pos, 1 );
			_sceVu0SubVector( &fvtemp2, cal_pos, &cam_pos ) ;
#ifdef KP_XBOX
			{
				float dmdm ;
				dmdm = cam_pos.vy - cal_pos->vy ;
				if ( dmdm == 0.0f ) dmdm = 0.0001f ;
				_sceVu0ScaleVector( cal_pos, &fvtemp2, (cam_pos.vy - GM_WaterLevel)/dmdm );
			}
#endif
			ftemp1 = GV_VecLen3F( cal_pos );
			_sceVu0AddVector( cal_pos, cal_pos, &cam_pos ) ;
			cal_pos->vw = ftemp1;
			alpha = 0;
			(*parts_param) = 0.0f;
			(*parts_flag)  = 0;
		}


		ftemp1 = (WIDTH_RATIO + WIDTH_RATIO_ADD*rnd()) * cal_pos->vw * screen_angle_ratio;

		_sceVu0ScaleVector( &fvtemp2, &l_x, ftemp1 );
		_sceVu0ScaleVector( &fvtemp3, &l_y, ftemp1 * CORE_RATIO );

		_sceVu0AddVector( pos++, cal_pos, &fvtemp2 ) ;
		_sceVu0AddVector( pos++, cal_pos, &fvtemp3 ) ;
		_sceVu0SubVector( pos++, cal_pos, &fvtemp3 ) ;
		_sceVu0SubVector( pos++, cal_pos, &fvtemp2 ) ;

		_sceVu0ScaleVector( &fvtemp2, &fvtemp2, DOUBLE_RATIO );
		_sceVu0ScaleVector( &fvtemp3, &fvtemp3, DOUBLE_RATIO_S );

		_sceVu0AddVector( pos++, cal_pos, &fvtemp2 ) ;
		_sceVu0AddVector( pos++, cal_pos, &fvtemp3 ) ;
		_sceVu0SubVector( pos++, cal_pos, &fvtemp3 ) ;
		_sceVu0SubVector( pos++, cal_pos, &fvtemp2 ) ;



		_sceVu0ScaleVector( &fvtemp2, &l_x, ftemp1 * CORE_RATIO );
		_sceVu0ScaleVector( &fvtemp3, &l_y, ftemp1 );

		_sceVu0AddVector( pos++, cal_pos, &fvtemp2 ) ;
		_sceVu0AddVector( pos++, cal_pos, &fvtemp3 ) ;
		_sceVu0SubVector( pos++, cal_pos, &fvtemp3 ) ;
		_sceVu0SubVector( pos++, cal_pos, &fvtemp2 ) ;

		_sceVu0ScaleVector( &fvtemp2, &fvtemp2, DOUBLE_RATIO_S );
		_sceVu0ScaleVector( &fvtemp3, &fvtemp3, DOUBLE_RATIO );

		_sceVu0AddVector( pos++, cal_pos, &fvtemp2 ) ;
		_sceVu0AddVector( pos++, cal_pos, &fvtemp3 ) ;
		_sceVu0SubVector( pos++, cal_pos, &fvtemp3 ) ;
		_sceVu0SubVector( pos++, cal_pos, &fvtemp2 ) ;

		alpha2 = alpha>>1;

		uvrgb->r = col_r;
		uvrgb->g = col_g;
		uvrgb->b = col_b;
		(uvrgb++)->a = alpha2;

		uvrgb->r = col_r;
		uvrgb->g = col_g;
		uvrgb->b = col_b;
		(uvrgb++)->a = alpha2;

		uvrgb->r = col_r;
		uvrgb->g = col_g;
		uvrgb->b = col_b;
		(uvrgb++)->a = alpha2;

		uvrgb->r = col_r;
		uvrgb->g = col_g;
		uvrgb->b = col_b;
		(uvrgb++)->a = alpha2;

		uvrgb->r = col_r;
		uvrgb->g = col_g;
		uvrgb->b = col_b;
		(uvrgb++)->a = alpha;

		uvrgb->r = col_r;
		uvrgb->g = col_g;
		uvrgb->b = col_b;
		(uvrgb++)->a = alpha;

		uvrgb->r = col_r;
		uvrgb->g = col_g;
		uvrgb->b = col_b;
		(uvrgb++)->a = alpha;

		uvrgb->r = col_r;
		uvrgb->g = col_g;
		uvrgb->b = col_b;
		(uvrgb++)->a = alpha;

		uvrgb->r = col_r;
		uvrgb->g = col_g;
		uvrgb->b = col_b;
		(uvrgb++)->a = alpha2;

		uvrgb->r = col_r;
		uvrgb->g = col_g;
		uvrgb->b = col_b;
		(uvrgb++)->a = alpha2;

		uvrgb->r = col_r;
		uvrgb->g = col_g;
		uvrgb->b = col_b;
		(uvrgb++)->a = alpha2;

		uvrgb->r = col_r;
		uvrgb->g = col_g;
		uvrgb->b = col_b;
		(uvrgb++)->a = alpha2;

		uvrgb->r = col_r;
		uvrgb->g = col_g;
		uvrgb->b = col_b;
		(uvrgb++)->a = alpha;

		uvrgb->r = col_r;
		uvrgb->g = col_g;
		uvrgb->b = col_b;
		(uvrgb++)->a = alpha;

		uvrgb->r = col_r;
		uvrgb->g = col_g;
		uvrgb->b = col_b;
		(uvrgb++)->a = alpha;

		uvrgb->r = col_r;
		uvrgb->g = col_g;
		uvrgb->b = col_b;
		(uvrgb++)->a = alpha;


		_sceVu0ScaleVector( &fvtemp2, &d_x, (0.05f + rnd()*0.1f) * ftemp1 );
		_sceVu0ScaleVector( &fvtemp3, &d_y, 0.02f * ftemp1 );

		_sceVu0AddVector( pos_s++, cal_pos, &fvtemp2 ) ;
		_sceVu0AddVector( pos_s++, cal_pos, &fvtemp3 ) ;
		_sceVu0SubVector( pos_s++, cal_pos, &fvtemp3 ) ;
		_sceVu0SubVector( pos_s++, cal_pos, &fvtemp2 ) ;

		uvrgb_s->r = col_r;
		uvrgb_s->g = col_g;
		uvrgb_s->b = col_b;
		(uvrgb_s++)->a = alpha;

		uvrgb_s->r = col_r;
		uvrgb_s->g = col_g;
		uvrgb_s->b = col_b;
		(uvrgb_s++)->a = alpha;

		uvrgb_s->r = col_r;
		uvrgb_s->g = col_g;
		uvrgb_s->b = col_b;
		(uvrgb_s++)->a = alpha;

		uvrgb_s->r = col_r;
		uvrgb_s->g = col_g;
		uvrgb_s->b = col_b;
		(uvrgb_s++)->a = alpha;

		parts_flag++;
		parts_param++;
		cal_pos++;
		cal_pos_after++;
	}

	DG_COPY_VEC( &work->before_cam_pos, &cam_pos );
	work->before_angle = screen_angle;

}


/*----------------------------------------------------------------*/
static void Die( Work *work )
{
	work->prim   = OK_FreePrim2( work->prim );
	work->prim_s = OK_FreePrim2( work->prim_s );
}

/*----------------------------------------------------------------*/
static void InitPacketCross( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB		*uvrgb0 ;
	DG_PRIM2_UVRGB		*uvrgb1 ;
	int		i, j ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	prim->raise = RAISE;

	uvrgb0 = prim->uvrgb[0] ;
	uvrgb1 = prim->uvrgb[1] ;

	for ( i = 0 ; i < N_PRIMS*N_POLYS/4 ; i++ ){
		for( j=0; j<2; j++ ){
			uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x8fff ;
			uvrgb0->a = uvrgb1->a = 0 ;
			uvrgb0++;
			uvrgb1++;

			uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x8fff ;
			uvrgb0->a = uvrgb1->a = 0 ;
			uvrgb0++;
			uvrgb1++;

			uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x0fff ;
			uvrgb0->a = uvrgb1->a = 0 ;
			uvrgb0++;
			uvrgb1++;

			uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x0fff ;
			uvrgb0->a = uvrgb1->a = 0 ;
			uvrgb0++;
			uvrgb1++;



			uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x8fff ;
			uvrgb0->a = uvrgb1->a = 0 ;
			uvrgb0++;
			uvrgb1++;

			uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x8fff ;
			uvrgb0->a = uvrgb1->a = 0 ;
			uvrgb0++;
			uvrgb1++;

			uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x0fff ;
			uvrgb0->a = uvrgb1->a = 0 ;
			uvrgb0++;
			uvrgb1++;

			uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb0->q = uvrgb1->q = 4096 ;
			uvrgb0->f = uvrgb1->f = 0x0fff ;
			uvrgb0->a = uvrgb1->a = 0 ;
			uvrgb0++;
			uvrgb1++;

		}
	}

}

/*----------------------------------------------------------------*/
static void InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB		*uvrgb0 ;
	DG_PRIM2_UVRGB		*uvrgb1 ;
	int		i ;


	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	prim->raise = RAISE;

	uvrgb0 = prim->uvrgb[0] ;
	uvrgb1 = prim->uvrgb[1] ;

	for ( i = 0 ; i < N_PRIMS*N_POLYS/4 ; i++ ){
		uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x8fff ;
		uvrgb0->a = uvrgb1->a = 0 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x8fff ;
		uvrgb0->a = uvrgb1->a = 0 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x0fff ;
		uvrgb0->a = uvrgb1->a = 0 ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x0fff ;
		uvrgb0->a = uvrgb1->a = 0 ;
		uvrgb0++;
		uvrgb1++;
	}

}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	FVECTOR		*cal_pos;
	int			i;

//	tex = DG_GetTexture( 15625989 /*"blood_2bw_alp"*/ );
//	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
//	tex = DG_GetTexture( 12244206 /*"fog02_msk"*/ );
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
//	tex = DG_GetTexture( 2998476 /*"xlit24a_alp"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );


	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	InitPacketCross( work, prim, tex );
	DG_InvisiblePrim2( prim );


	prim = work->prim_s = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS/4, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	InitPacket( work, prim, tex );
	DG_InvisiblePrim2( prim );


	cal_pos       = work->cal_pos;
	for ( i=0; i<N_PARTS; i++ ){
		cal_pos->vy = SCREEN_NEAR * frnd();
		cal_pos->vw = frnd();
		work->parts_param[i]=0.0f;
		work->parts_flag[i] = 1;
		cal_pos++;
	}



	return 0 ;
}

void *NewKirariWaterSun2( FMATRIX *world, int *col )
{
	Work		*work ;

	OPERATOR() ;

//	work = (Work *)GV_NewActorPrio( GV_ACTOR_EFFECT, sizeof( Work ), GV_PRIO_MAX ) ;
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->world = world;

		work->col = col;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}


