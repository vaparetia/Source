//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	drops_many_cm.c
	カメラ前に大量水玉

	2001/06/29 S.Okajima
	$Id: drops_many_cm.c,v 1.1.1.3 2002/11/19 11:47:30 Yoshizawa1 Exp $
*/


#ifdef PSX2	///
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
#define	RAISE_ADD	( -20000 )
#define	RAISE_SUB	( -10000 )

#define	COL_R_ADD	(64)
#define	COL_G_ADD	(64)
#define	COL_B_ADD	(64)
#define	COL_A_ADD	(96)

#define	COL_R_SUB	(64)
#define	COL_G_SUB	(64)
#define	COL_B_SUB	(64)
#define	COL_A_SUB	(96)


#define	SCREEN_NEAR			( 51.0f )

#define	N_VERTS			(64)
#define	N_POLYS			(N_VERTS/4)
#define	N_PRIMS			(16)


#define	WIDTH_MIN		(0.01f * SCREEN_NEAR)
#define	WIDTH_RND		(0.02f  * SCREEN_NEAR)
#define	RATIO_MAX		( (WIDTH_MIN+WIDTH_RND)*4.0f/SCREEN_NEAR )
#define	SHIFT_WIDTH		(WIDTH_MIN * 0.3f)

typedef	struct	{
	GV_ACT_EX	actor ;

	int			life;

	float		angle_vec;
	float		angle;

	float		ratio[N_PRIMS*N_POLYS*2];
	DG_PRIM2	*prim_add ;
	DG_PRIM2	*prim_sub ;
} Work ;

static Work *OK_DROPS_MANY_CM_WORK = NULL;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2	*prim_add;
	DG_PRIM2	*prim_sub;
	FVECTOR		*pos_add;
	FVECTOR		*pos_sub;
	FVECTOR		*shift;
	FVECTOR		center;
	float		angle;
	float		ftemp;
	float		screen_near_x;
	float		screen_near_y;
	float		*ratio;
	int			clock;
	int			i;

	prim_add = work->prim_add;
	prim_sub = work->prim_sub;

	if( !GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE) ){
		DG_InvisiblePrim2( prim_add );
		DG_InvisiblePrim2( prim_sub );
		GV_DestroyActor( work ) ;
		return;
	}

	DG_VisiblePrim2( prim_add );
	GM_GroupPrim2( prim_add, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim_add );
	clock = prim_add->buffer_clock;

	DG_VisiblePrim2( prim_sub );
//	DG_InvisiblePrim2( prim_sub );
	GM_GroupPrim2( prim_sub, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim_sub );


	if(GV_Time%16==0) work->angle = TPI*0.25f*frnd();

	work->angle_vec-= work->angle*0.2f;
	work->angle+= work->angle_vec;
	work->angle*= 0.9f;

//printf("%f:%f:\n",work->angle,work->angle_vec);

	screen_near_x = SCREEN_NEAR / (ASPECT_X()                                       ) / DG_Chanls->screen;
	screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / DG_Chanls->screen;

	ratio = work->ratio;
	angle = work->angle - PI*0.5f;
	shift = SCRPAD_ADDR;
	shift->vx = SHIFT_WIDTH*sinf( work->angle );
	shift->vy = SHIFT_WIDTH*cosf( work->angle );
	shift->vz = 0.0f;
	pos_add = prim_add->pos[clock];
	pos_sub = prim_sub->pos[clock];
	for ( i = 0 ; i < N_PRIMS*N_POLYS ; i++ ){
		center.vx = screen_near_x * (*ratio++);
		center.vy = screen_near_y * (*ratio++);

		ftemp = pos_add->vw;
		pos_add->vx = center.vx;
		pos_add->vy = center.vy + ftemp;
		pos_add->vz = SCREEN_NEAR;
		_sceVu0AddVector( pos_sub++, pos_add++, shift );

		pos_add->vx = center.vx - ftemp;
		pos_add->vy = center.vy;
		pos_add->vz = SCREEN_NEAR;
		_sceVu0AddVector( pos_sub++, pos_add++, shift );
 
		pos_add->vx = center.vx + ftemp;
		pos_add->vy = center.vy;
		pos_add->vz = SCREEN_NEAR;
		_sceVu0AddVector( pos_sub++, pos_add++, shift );

		pos_add->vx = center.vx;
		pos_add->vy = center.vy - ftemp;
		pos_add->vz = SCREEN_NEAR;
		_sceVu0AddVector( pos_sub++, pos_add++, shift );
	}

	if( work->life > 0 ){
		work->life--;
		if( work->life <= 0 ){
			GV_DestroyActor( work ) ;
		}
	}


}

static void Die( Work *work )
{
	work->prim_add = OK_FreePrim2( work->prim_add );
	work->prim_sub = OK_FreePrim2( work->prim_sub );
	OK_DROPS_MANY_CM_WORK = NULL;
}

/*----------------------------------------------------------------*/
static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex, int flag )
{
	DG_PRIM2_UVRGB		*uvrgb0 ;
	DG_PRIM2_UVRGB		*uvrgb1 ;
	int					i;
	int					col_r;
	int					col_g;
	int					col_b;
	int					col_a;

	if( flag ){
		col_r = COL_R_ADD;
		col_g = COL_G_ADD;
		col_b = COL_B_ADD;
		col_a = COL_A_ADD;
	}else{
		col_r = COL_R_SUB;
		col_g = COL_G_SUB;
		col_b = COL_B_SUB;
		col_a = COL_A_SUB;
	}

	DG_ConfigPrim2Tex( prim, tex );

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
	DG_PRIM2	*prim_add ;
	DG_PRIM2	*prim_sub ;
	DG_TEX		*tex ;
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	float		*ratio;
	float		angle0;
	float		angle1;
	float		x0,y0;
	float		x1,y1;
	int	i;

	work->angle = PI*0.25f;


//	tex = DG_GetTexture( 15625989 /*"blood_2bw_alp"*/ );
//	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );
//	tex = DG_GetTexture( 12244206 /*"fog02_msk"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );

	prim_add = work->prim_add = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, N_PRIMS, N_VERTS );
	if(prim_add==NULL){
		printf("null prim\n");
		return -1;
	}
	DG_InvisiblePrim2( prim_add );
	if( !InitPacket2( work, prim_add, tex, 1 ) ) return -1;
	DG_SetPrim2Alpha( prim_add, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	prim_add->raise = RAISE_ADD;

	prim_sub = work->prim_sub = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, N_PRIMS, N_VERTS );
	if(prim_sub==NULL){
		printf("null prim\n");
		return -1;
	}
	DG_InvisiblePrim2( prim_sub );
	if( !InitPacket2( work, prim_sub, tex, 0 ) ) return -1;
	DG_SetPrim2Alpha( prim_sub, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	prim_sub->raise = RAISE_SUB;


	// カメラ前水玉の座標を決定
	x0 = frnd();
	y0 = frnd();
	x1 = frnd();
	y1 = frnd();
	ratio = work->ratio;
	pos0 = work->prim_add->pos[0];
	pos1 = work->prim_add->pos[1];
	angle0 = TPI*rnd();
	angle1 = TPI*rnd();
	for ( i = 0 ; i < N_PRIMS*N_POLYS ; i++ ){
		angle0+= PI*0.25f*frnd();
		x0+= RATIO_MAX*sinf( angle0 );
		y0+= RATIO_MAX*cosf( angle0 );
		angle1+= PI*0.25f*frnd();
		x1+= RATIO_MAX*sinf( angle1 );
		y1+= RATIO_MAX*cosf( angle1 );
		if( DG_FABS(x0) > 1.0f ) x0 = frnd();
		if( DG_FABS(y0) > 1.0f ) y0 = frnd();
		if( DG_FABS(x1) > 1.0f ) x1 = frnd();
		if( DG_FABS(y1) > 1.0f ) y1 = frnd();
//		(*ratio++) = (x0 - x1)*rnd() + x1;
		(*ratio++) = x0;
		(*ratio++) = (y0 - y1)*rnd()*rnd() + y1;
		pos0->vw = pos1->vw = WIDTH_MIN + WIDTH_RND*rnd();
		pos0+= 4;
		pos1+= 4;
	}

	return 0 ;
}

void *NewWaterDropsMany( int life )
{
	Work		*work ;

	OPERATOR() ;

	if( OK_DROPS_MANY_CM_WORK != NULL ) return NULL;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->life = life;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}

	OK_DROPS_MANY_CM_WORK = work;

	return (void *)work ;
}

