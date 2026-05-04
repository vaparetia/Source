//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	kirari_water_cm.c
	バンプ戦きらきら水飛沫（透視変換バージョン）

	2001/06/05 S.Okajima
	$Id: kirari_water_cm.c,v 1.1.1.3 2002/11/19 11:47:32 Yoshizawa1 Exp $
*/

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
#define	RAISE			( 0 )

#define	COL_R	(48)
#define	COL_G	(48)
#define	COL_B	(48)
#define	COL_A	(32)


#define	SCREEN_NEAR			( 51.0f )
//#define	SCREEN_NEAR			( 5000.0f )

#define	SCR_POS		(SCRPAD_ADDR)

#define	N_NUM			(2)				/* ２枚で十字を作る */
#define	N_VERTS			(64)
#define	N_PRIMS			(8)
#define	N_POLYS			(N_VERTS/4)
#define	N_PARTS			(N_PRIMS*N_POLYS/N_NUM)	/* 十字の数 */

#define	MAX_NUM			(N_PARTS)

#define	WIDTH_PARTS		(1.0f * SCREEN_NEAR)

#define	SPEED_MIN	(5.0f)
#define	SPEED_RND	(5.0f)

#define	WIDTH_RATIO	(0.01f)
#define	SHORT_RATIO	(0.4f)

typedef	struct	{
	GV_ACT_EX	actor ;

	int			now_num;
	int			result;
	FVECTOR		pos_list[MAX_NUM];
	FVECTOR		pos;
	FVECTOR		from;
	FVECTOR		to;

	DG_PRIM2	*prim ;
} Work ;

static Work *OK_KIRARI_WATER_WORK = NULL;


/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	clock;
	int	i;
	DG_PRIM2	*prim;
	DG_PRIM2_UVRGB		*uvrgb ;
	FVECTOR		*pos_list;
	FVECTOR		*pos;
	FVECTOR		*center;
	FVECTOR		fvtemp0;
	FVECTOR		fvtemp1;
	float	param_x;
	float	param_y;
	float	screen_near_x;
	float	screen_near_y;
	float	x_rs;
	float	x_rc;
	float	x_rs_s;
	float	x_rc_s;
	float	y_rs;
	float	y_rc;
	float	y_rs_s;
	float	y_rc_s;
	float	ftemp0;
	float	ftemp1;

	if( work->now_num >= MAX_NUM ) work->now_num = MAX_NUM-1;

	prim = work->prim;
	DG_VisiblePrim2( prim );
	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	DG_COPY_VEC( &work->to, (FVECTOR *)DG_Chanls->eye.m[3] );

	param_x = ASPECT_X() *                    DG_Chanls->screen;
	param_y = ASPECT_Y() * DG_Chanls->width * DG_Chanls->screen / DG_Chanls->height;

	DG_SetPos( &DG_Chanls->eye_pers );
	DG_PutVector( work->pos_list, SCR_POS, work->now_num );

	center = SCR_POS;
	pos_list = work->pos_list;
	pos = prim->pos[clock];

	fvtemp0.vx = 0.0f;
	fvtemp0.vy = 0.0f;
	fvtemp0.vz = 1.0f;
	DG_SetPos( &DG_Chanls->eye );
	DG_RotVector( &fvtemp0, &fvtemp1, 1 );

//	ftemp1 = _sceVu0InnerProduct( &fvtemp0, &fvtemp1 )*PI + PI*0.25f +
//	         DG_Chanls->screen*0.08f;	/* 対象はなんでもいい？ */
	ftemp1 = PI*0.25f + DG_Chanls->screen*0.08f;	/* 対象はなんでもいい？ */

	ftemp0 = sinf( ftemp1 );
	ftemp1 = cosf( ftemp1 );

	uvrgb = prim->uvrgb[clock];
	for ( i=0; i<work->now_num; i++ ){
		center->vx/= center->vw;
		center->vy/= center->vw;
		if( center->vz < center->vw
		 && DG_FABS(center->vx) < 1.0f
		 && DG_FABS(center->vy) < 1.0f
		  ){

			center->vz =-center->vz;

			screen_near_x = center->vz / param_x;
			screen_near_y = center->vz / param_y;

			center->vx   *= screen_near_x;
			center->vy   *= screen_near_y;

			screen_near_x*= pos_list->vw;
			screen_near_y*= pos_list->vw;

			x_rs   = ftemp0 * screen_near_x;
			x_rc   = ftemp1 * screen_near_x;
			x_rs_s = x_rs * WIDTH_RATIO;
			x_rc_s = x_rc * WIDTH_RATIO;
			y_rs   = ftemp0 * screen_near_y;
			y_rc   = ftemp1 * screen_near_y;
			y_rs_s = y_rs * WIDTH_RATIO;
			y_rc_s = y_rc * WIDTH_RATIO;

			x_rs  *= SHORT_RATIO;
			y_rc  *= SHORT_RATIO;

			pos->vx = center->vx + x_rs;
			pos->vy = center->vy + y_rc;
			pos->vz = center->vz;
			pos++;

			pos->vx = center->vx - x_rc_s;
			pos->vy = center->vy + y_rs_s;
			pos->vz = center->vz;
			pos++;

			pos->vx = center->vx + x_rc_s;
			pos->vy = center->vy - y_rs_s;
			pos->vz = center->vz;
			pos++;

			pos->vx = center->vx - x_rs;
			pos->vy = center->vy - y_rc;
			pos->vz = center->vz;
			pos++;


			pos->vx = center->vx + x_rs_s;
			pos->vy = center->vy + y_rc_s;
			pos->vz = center->vz;
			pos++;

			pos->vx = center->vx - x_rc;
			pos->vy = center->vy + y_rs;
			pos->vz = center->vz;
			pos++;

			pos->vx = center->vx + x_rc;
			pos->vy = center->vy - y_rs;
			pos->vz = center->vz;
			pos++;

			pos->vx = center->vx - x_rs_s;
			pos->vy = center->vy - y_rc_s;
			pos->vz = center->vz;
			pos++;
		}else{
			DG_COPY_VEC( pos++, &DG_ZeroVector );
			DG_COPY_VEC( pos++, &DG_ZeroVector );
			DG_COPY_VEC( pos++, &DG_ZeroVector );
			DG_COPY_VEC( pos++, &DG_ZeroVector );
			DG_COPY_VEC( pos++, &DG_ZeroVector );
			DG_COPY_VEC( pos++, &DG_ZeroVector );
			DG_COPY_VEC( pos++, &DG_ZeroVector );
			DG_COPY_VEC( pos++, &DG_ZeroVector );
		}

		pos_list++;
		center++;
	}
	for (    ; i<MAX_NUM; i++ ){
		DG_COPY_VEC( pos++, &DG_ZeroVector );
		DG_COPY_VEC( pos++, &DG_ZeroVector );
		DG_COPY_VEC( pos++, &DG_ZeroVector );
		DG_COPY_VEC( pos++, &DG_ZeroVector );
		DG_COPY_VEC( pos++, &DG_ZeroVector );
		DG_COPY_VEC( pos++, &DG_ZeroVector );
		DG_COPY_VEC( pos++, &DG_ZeroVector );
		DG_COPY_VEC( pos++, &DG_ZeroVector );
	}


	work->now_num = 0;
}


// width_ratio : 光線の長い方の、画面幅に対する割合
//
void OK_PutKirariCross( FVECTOR *pos, float width_ratio )
{
	Work *work;
	FVECTOR	fvtemp;

	if( OK_KIRARI_WATER_WORK == NULL ){
		void *NewKirariWater( void );
		NewKirariWater();
	}
	if( OK_KIRARI_WATER_WORK == NULL ) return; // それでもやっぱり起動できなかった
	work = OK_KIRARI_WATER_WORK;

	if( work->now_num >= MAX_NUM ) return;

	fvtemp.vx = pos->vx;
	fvtemp.vy = pos->vy;
	fvtemp.vz = pos->vz;
	fvtemp.vw = width_ratio;
	DG_COPY_VEC( &work->pos_list[work->now_num], &fvtemp );
	work->now_num++;
}

/*----------------------------------------------------------------*/
static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
	OK_KIRARI_WATER_WORK = NULL;
}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB		*uvrgb0 ;
	DG_PRIM2_UVRGB		*uvrgb1 ;
	int		i ;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	prim->raise = RAISE;

	uvrgb0 = prim->uvrgb[0] ;
	uvrgb1 = prim->uvrgb[1] ;

	for ( i = 0 ; i < N_PRIMS*N_POLYS ; i++ ){
		uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x8fff ;
		uvrgb0->r = uvrgb1->r = COL_R ;
		uvrgb0->g = uvrgb1->g = COL_G ;
		uvrgb0->b = uvrgb1->b = COL_B ;
		uvrgb0->a = uvrgb1->a = COL_A ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x8fff ;
		uvrgb0->r = uvrgb1->r = COL_R ;
		uvrgb0->g = uvrgb1->g = COL_G ;
		uvrgb0->b = uvrgb1->b = COL_B ;
		uvrgb0->a = uvrgb1->a = COL_A ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x0fff ;
		uvrgb0->r = uvrgb1->r = COL_R ;
		uvrgb0->g = uvrgb1->g = COL_G ;
		uvrgb0->b = uvrgb1->b = COL_B ;
		uvrgb0->a = uvrgb1->a = COL_A ;
		uvrgb0++;
		uvrgb1++;

		uvrgb0->u = uvrgb1->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
		uvrgb0->v = uvrgb1->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
		uvrgb0->q = uvrgb1->q = 4096 ;
		uvrgb0->f = uvrgb1->f = 0x0fff ;
		uvrgb0->r = uvrgb1->r = COL_R ;
		uvrgb0->g = uvrgb1->g = COL_G ;
		uvrgb0->b = uvrgb1->b = COL_B ;
		uvrgb0->a = uvrgb1->a = COL_A ;
		uvrgb0++;
		uvrgb1++;
	}

}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->now_num = 0;

//	tex = DG_GetTexture( 15625989 /*"blood_2bw_alp"*/ );
//	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
//	tex = DG_GetTexture( 12244206 /*"fog02_msk"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	InitPacket2( work, prim, tex );
	DG_InvisiblePrim2( prim );

	return 0 ;
}

void *NewKirariWater( void )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewActorPrio( GV_ACTOR_EFFECT, sizeof( Work ), GV_PRIO_MAX ) ;
//	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	OK_KIRARI_WATER_WORK = work;
	return (void *)work ;
}

