//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
  tng_stage_monitor.c
  天狗兵ステージノイズモニター
  2001/08/16 Yuuta Kunibe
  $Id: tng_stage_monitor.c,v 1.1.1.3 2002/11/19 11:44:54 Yoshizawa1 Exp $
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



#define	N_PRIMS		(1)
#define	N_VERTS		(4)

#define BASE_RGB	(24)
#define ALPHA		(64)
#define	SPEED		(1200.0f)

#define LIFE		(300)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)


#define	SIZE_PARAM	(1000.0f)


typedef	struct	{

    GV_ACT_EX	actor;

    int 		*mode;
	
    DG_PRIM2	*prim;
    DG_PRIM2	*prim_scroll;
    DG_PRIM2	*prim_noise;

	int		flame_tex;
	int		scroll_tex;
	int		noise_tex;

	
    FMATRIX	mat;

    float	hlf_width;
    float	height;

	float	scroll_x;
	float	scroll_y;
	float	scroll_w;
	float	scroll_h;

	float	v_start;
	float	v_end;
	float	v_width;	
	int		scroll_cnt;
	int		scroll_cnt_max;

	float	v_scale;
	float	v_offset;

	float	noise_width;
    
} Work ;



static void ActScroll( Work *work )
{

	int				clock;
	DG_PRIM2_UVRGB	*uvrgb;
	float			rate;
	float			v1, v2;

	clock = work->prim_scroll->buffer_clock;
	uvrgb = work->prim_scroll->uvrgb[clock];

	rate = (float)work->scroll_cnt / (float)work->scroll_cnt_max;

	v1 = ( work->v_end - work->v_start ) * rate + work->v_start;
	v2 = v1 + work->v_width;

	uvrgb->v = FTOI12( v1 * work->v_scale + work->v_offset );
	uvrgb++;
	uvrgb->v = FTOI12( v1 * work->v_scale + work->v_offset );
	uvrgb++;
	uvrgb->v = FTOI12( v2 * work->v_scale + work->v_offset );
	uvrgb++;
	uvrgb->v = FTOI12( v2 * work->v_scale + work->v_offset );
	uvrgb++;

	if ( ++work->scroll_cnt >= work->scroll_cnt_max ) {
		work->scroll_cnt -= work->scroll_cnt_max;
	}	
	
}

static void ActNormal( Work *work )
{

	int 			i,j;
	int 			clock;
	FVECTOR			*pos;
	DG_PRIM2_UVRGB	*uvrgb;
	DG_PRIM2_UVRGB	*uvrgb_pre;
	float			shift_x;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }

    DG_SwitchBuffPrim2( work->prim_scroll );
    DG_SwitchBuffPrim2( work->prim_noise );

	DG_VisiblePrim2( work->prim );
	DG_VisiblePrim2( work->prim_scroll );


	if ( work->noise_width <= 0.0f ) {
		work->noise_width = 0.0f;
		DG_InvisiblePrim2( work->prim_noise );
	}
	else {
		work->noise_width -= 10.0f;
		
		clock     = work->prim_noise->buffer_clock;
		pos       = work->prim_noise->pos[clock];
		uvrgb     = work->prim_noise->uvrgb[clock];
		uvrgb_pre = work->prim_noise->uvrgb[1-clock];

		for ( i = 0 ; i < 2 ; i++ ) {

			shift_x = frnd() * work->noise_width;

			pos->vx = work->hlf_width + shift_x;
			pos++;

			pos->vx =-work->hlf_width + shift_x;
			pos++;

			pos->vx = work->hlf_width + shift_x;
			pos++;

			pos->vx =-work->hlf_width + shift_x;
			pos++;

			for ( j = 0 ; j < 4 ; j++ ) {
				if ( uvrgb_pre->a > 0 ) {
					uvrgb->a = uvrgb_pre->a - 4;
				}
				else {
					uvrgb->a = 0;
				}
				uvrgb++;
			}

		}

	}		
	

	uvrgb = work->prim->uvrgb[work->prim->buffer_clock];
	uvrgb_pre = work->prim->uvrgb[1-work->prim->buffer_clock];

	for ( i = 0 ; i < 4 ; i++ ) {
		if ( uvrgb_pre->a < 128 ) {
			uvrgb->a = uvrgb_pre->a + 12;
		}
		else {
			uvrgb->a = 128;
		}
		uvrgb++;
	}
	
	ActScroll( work );
	
}



static void ActNoise( Work *work )
{

	int				i,j;
	int				clock;
	FVECTOR			*pos;
	DG_PRIM2_UVRGB	*uvrgb;
	DG_PRIM2_UVRGB	*uvrgb_pre;
	float			shift_x;
	

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }

    DG_SwitchBuffPrim2( work->prim_scroll );
    DG_SwitchBuffPrim2( work->prim_noise );


	DG_VisiblePrim2( work->prim );
	DG_InvisiblePrim2( work->prim_scroll );
	DG_VisiblePrim2( work->prim_noise );


	clock     = work->prim_noise->buffer_clock;
	pos       = work->prim_noise->pos[clock];
	uvrgb     = work->prim_noise->uvrgb[clock];
	uvrgb_pre = work->prim_noise->uvrgb[1-clock];


	/* ノイズ振幅増えながらフェードイン */
	if ( work->noise_width < 160.0f ) {
		work->noise_width += 10.0f;
	}

	for ( i = 0 ; i < 2 ; i++ ) {

		shift_x = frnd()*work->noise_width;

		pos->vx = work->hlf_width + shift_x;
		pos++;

		pos->vx =-work->hlf_width + shift_x;
		pos++;

		pos->vx = work->hlf_width + shift_x;
		pos++;

		pos->vx =-work->hlf_width + shift_x;
		pos++;

		for ( j = 0 ; j < 4 ; j++ ) {
			if ( uvrgb_pre->a < 32 ) {
				uvrgb->a = uvrgb_pre->a + 4;
			}
			else {
				uvrgb->a = 32;
			}
			uvrgb++;
		}

	}



	/* モニターのフレームフェードアウト */
	clock     = work->prim->buffer_clock;
	uvrgb     = work->prim->uvrgb[clock];
	uvrgb_pre = work->prim->uvrgb[1-clock];

	for ( i = 0 ; i < 4 ; i++ ) {
		if ( uvrgb_pre->a > 32 ) {
			uvrgb->a = uvrgb_pre->a - 12;
		}
		else {
			uvrgb->a = 32;
		}
		uvrgb++;
	}
	
}

static void ActInvisible( Work *work )
{

    /*DG_SwitchBuffPrim2( work->prim );
    DG_SwitchBuffPrim2( work->prim_scroll );
    DG_SwitchBuffPrim2( work->prim_noise );*/

	DG_InvisiblePrim2( work->prim );
	DG_InvisiblePrim2( work->prim_scroll );
	DG_InvisiblePrim2( work->prim_noise );	

}

static void Act( Work *work )
{

	//HZX_ViewMatrix( &work->mat, 1000.0f );
	switch ( *work->mode ) {
	case 0:
		ActNormal( work );
		break;
	case 1:
		ActNoise( work );
		break;
	case 2:
		ActInvisible( work );
		break;
	case 3:
		GV_DestroyActor( work );
		break;
	}

}

static void Die( Work *work )
{
	if ( work->prim ) work->prim = OK_FreePrim2( work->prim );
	if ( work->prim_scroll ) work->prim_scroll = OK_FreePrim2( work->prim_scroll );
	if ( work->prim_noise ) work->prim_noise = OK_FreePrim2( work->prim_noise );
}




static int InitMonitor( Work *work )
{

    int				i;
    DG_PRIM2		*prim;
    DG_TEX			*tex;
    FVECTOR			*pos;
    DG_PRIM2_UVRGB	*uvrgb;


    prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, 4 );
    if ( prim == NULL ) {
		return 0;
    }

    work->prim->root = &work->mat;


    tex = DG_GetTexture( work->flame_tex );
    DG_ConfigPrim2Tex( prim, tex );
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );


    pos   = SCR_POS;
    uvrgb = SCR_UVS;
	
    for ( i = 0 ; i < N_PRIMS ; i++ ) {

		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos->vx = work->hlf_width;
		pos->vy = work->height;
		pos++;
		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos->vx =-work->hlf_width;
		pos->vy = work->height;
		pos++;
		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos->vx = work->hlf_width;
		pos->vy = 0.0f;
		pos++;
		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos->vx =-work->hlf_width;
		pos->vy = 0.0f;
		pos++;


		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x8fff;
		uvrgb->r = 128;
		uvrgb->g = 128;
		uvrgb->b = 128;
		uvrgb->a = 128;
		uvrgb++;
	
		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x8fff;
		uvrgb->r = 128;
		uvrgb->g = 128;
		uvrgb->b = 128;
		uvrgb->a = 128;
		uvrgb++;
	
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = 128;
		uvrgb->g = 128;
		uvrgb->b = 128;
		uvrgb->a = 128;
		uvrgb++;
	
		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = 128;
		uvrgb->g = 128;
		uvrgb->b = 128;
		uvrgb->a = 128;
		uvrgb++;

    }	

    
    OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        4 );
    OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        4 );
    OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), 4 );
    OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), 4 );
 
    return 1;


}


static int InitScroll( Work *work )
{

    int				i;
    DG_PRIM2		*prim;
    DG_TEX			*tex;
    FVECTOR			*pos;
    DG_PRIM2_UVRGB	*uvrgb;
	

    prim = work->prim_scroll = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 1, 4 );
    if ( prim == NULL ) {
		return 0;
    }

    work->prim_scroll->root = &work->mat;


    tex = DG_GetTexture( work->scroll_tex );
    DG_ConfigPrim2Tex( prim, tex );
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	work->v_scale  = tex->v_scale;
	work->v_offset = tex->v_offset;
	

    pos   = SCR_POS;
    uvrgb = SCR_UVS;


	
    for ( i = 0 ; i < N_PRIMS ; i++ ) {

		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos->vx = work->hlf_width * work->scroll_w;
		pos->vy = work->height - work->height * work->scroll_y;
		pos++;

		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos->vx =-work->hlf_width * work->scroll_w;
		pos->vy = work->height - work->height * work->scroll_y;
		pos++;

		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos->vx = work->hlf_width * work->scroll_w;
		pos->vy = work->height - work->height * ( work->scroll_y + work->scroll_h );
		pos++;

		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos->vx =-work->hlf_width * work->scroll_w;
		pos->vy = work->height - work->height * ( work->scroll_y + work->scroll_h );
		pos++;


		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x8fff;
		uvrgb->r = 128;
		uvrgb->g = 128;
		uvrgb->b = 128;
		uvrgb->a = 128;
		uvrgb++;
	
		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x8fff;
		uvrgb->r = 128;
		uvrgb->g = 128;
		uvrgb->b = 128;
		uvrgb->a = 128;
		uvrgb++;
	
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = 128;
		uvrgb->g = 128;
		uvrgb->b = 128;
		uvrgb->a = 128;
		uvrgb++;
	
		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = 128;
		uvrgb->g = 128;
		uvrgb->b = 128;
		uvrgb->a = 128;
		uvrgb++;

    }	

    
    OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        4 );
    OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        4 );
    OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), 4 );
    OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), 4 );
 
    return 1;


}




static int InitNoise( Work *work )
{

    int				i;
    DG_PRIM2		*prim;
    DG_TEX			*tex;
    FVECTOR			*pos;
    DG_PRIM2_UVRGB	*uvrgb;

    prim = work->prim_noise = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, 2, 4 );
    if ( prim == NULL ) {
		return 0;
    }

    work->prim_noise->root = &work->mat;


    tex = DG_GetTexture( work->noise_tex );
    DG_ConfigPrim2Tex( prim, tex );
    DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );


    pos   = SCR_POS;
    uvrgb = SCR_UVS;
	
    for ( i = 0 ; i < 2 ; i++ ) {

		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos->vx = work->hlf_width;
		pos->vy = work->height;
		pos++;
		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos->vx =-work->hlf_width;
		pos->vy = work->height;
		pos++;
		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos->vx = work->hlf_width;
		pos->vy = 0.0f;
		pos++;
		DG_COPY_VEC( pos, &DG_ZeroVector );
		pos->vx =-work->hlf_width;
		pos->vy = 0.0f;
		pos++;


		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x8fff;
		uvrgb->r = 128;
		uvrgb->g = 128;
		uvrgb->b = 128;
		uvrgb->a = 0;
		uvrgb++;
	
		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x8fff;
		uvrgb->r = 128;
		uvrgb->g = 128;
		uvrgb->b = 128;
		uvrgb->a = 0;
		uvrgb++;
	
		uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = 128;
		uvrgb->g = 128;
		uvrgb->b = 128;
		uvrgb->a = 0;
		uvrgb++;
	
		uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb->q = 4096;
		uvrgb->f = 0x0fff;
		uvrgb->r = 128;
		uvrgb->g = 128;
		uvrgb->b = 128;
		uvrgb->a = 0;
		uvrgb++;

    }	

    
    OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),        8 );
    OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),        8 );
    OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), 8 );
    OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), 8 );
 
    return 1;


}




static int GetResources( Work *work, FVECTOR *pole_pos, SVECTOR *pole_rot, float radius, float offset_y,
						 float width, float height,
						 int flame_tex, int scroll_tex, int noise_tex,
						 float scroll_x, float scroll_y, float scroll_w, float scroll_h,
						 float v_start, float v_end, float v_width, float v_time, int *mode )
{

	FVECTOR 	shift;
	
	work->mode = mode;

	DG_SetPos2( pole_pos, pole_rot );
	DG_GetPos( &work->mat );

	shift.vx = 0.0f;
	shift.vy = (float)offset_y;
	shift.vz = (float)radius;
	shift.vw = 1.0f;

	DG_PutVector( &shift, (FVECTOR *)work->mat.m[3], 1 );


	work->flame_tex = flame_tex;
	work->scroll_tex = scroll_tex;
	work->noise_tex = noise_tex;

	printf("%d %d %d\n", work->flame_tex, work->scroll_tex, work->noise_tex );
	
	/* 枠サイズ取得 */
	work->hlf_width  = (float)width / 2.0f;
	work->height     = (float)height;

	work->scroll_x = (float)scroll_x / SIZE_PARAM;
	work->scroll_y = (float)scroll_y / SIZE_PARAM;
	work->scroll_w = (float)scroll_w / SIZE_PARAM;
	work->scroll_h = (float)scroll_h / SIZE_PARAM;

	printf("-------------------------%f %f %f %f\n",work->scroll_x,work->scroll_y,work->scroll_w,work->scroll_h);
	
	work->v_start = v_start / SIZE_PARAM;
	work->v_end   = v_end   / SIZE_PARAM;
	work->v_width = v_width / SIZE_PARAM;
	printf("start %f, end %f, width %f\n", work->v_start, work->v_end, work->v_width );

	work->scroll_cnt_max = v_time;
	work->scroll_cnt = 0;


	work->noise_width = 0.0f;
	
	if ( !( InitMonitor( work ) ) ) {
		return -1;
	}

	if ( !( InitScroll( work ) ) ) {
		return -1;
	}

	if ( !( InitNoise( work ) ) ) {
		return -1;
	}

	return 0;

}



void *NewTengStageMonitor( FVECTOR *pole_pos, SVECTOR *pole_rot, float radius, float offset_y,
						   float width, float height,
						   int flame_tex, int scroll_tex, int noise_tex,
						   float scroll_x, float scroll_y, float scroll_w, float scroll_h,
						   float v_start, float v_end, float v_width, float v_time, int *mode )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) );

	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die );
		GV_ActorEX( &work->actor );
		if ( GetResources( work, pole_pos, pole_rot, radius, offset_y, width, height,
						   flame_tex, scroll_tex, noise_tex,
						   scroll_x, scroll_y, scroll_w, scroll_h,
						   v_start, v_end, v_width, v_time, mode ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

