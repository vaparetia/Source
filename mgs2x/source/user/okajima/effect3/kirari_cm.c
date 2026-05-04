//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	kirari_cm.c
	バンプ戦ナイフ投げナイフ強調（透視変換バージョン）

	2001/06/03 S.Okajima
	$Id: kirari_cm.c,v 1.1.1.3 2002/11/19 11:47:32 Yoshizawa1 Exp $
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
extern void *NewOnlineCheck_Static( int map_id,
					 FVECTOR *from, FVECTOR *to,
					 int chk_flag, int seg_flag, int flr_flag,
					 int *result,
					 int turn, int phase, float max_len  );

#define	RAISE			( 0 )

#define	COL_R			(32)
#define	COL_G			(64)
#define	COL_B			(90)
#define	COL_A			(24)

#define	COL_R_SHADOW	(128)
#define	COL_G_SHADOW	(64)
#define	COL_B_SHADOW	(32)
#define	COL_A_SHADOW	(64)


//#define	SCREEN_NEAR			( 51.0f )
#define	SCREEN_NEAR			( 1000.0f )

#define	N_VERTS			(64)
#define	N_POLYS			(N_VERTS/4)
#define	N_PRIMS			(1)

#define	WIDTH_PARTS				(SCREEN_NEAR * 0.35f)
#define	WIDTH_PARTS_SHADOW		(SCREEN_NEAR * 0.25f)

#define	SPEED_MIN	(5.0f)
#define	SPEED_RND	(5.0f)

#define	SHORT_RATIO	(0.01f)

typedef	struct	{
	GV_ACT_EX	actor ;

	int			shadow_knife;
	int			result;
	float		ratio[N_PRIMS*N_POLYS];
	float		angle[N_PRIMS*N_POLYS];
	float		blink_angle;
	FMATRIX		*world;
	FVECTOR		from;
	FVECTOR		to;

	DG_PRIM2	*prim ;
} Work ;


/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	clock;
	int	i;
	DG_PRIM2		*prim;
	FVECTOR	*pos;
	FVECTOR	diff;
	FVECTOR	center;
	float	screen_near_x;
	float	screen_near_y;
	float	rs;
	float	rc;
	float	radius_local;
	float	ftemp;
	float	width_parts;
	float	*ratio;
	float	*angle;

	prim = work->prim;

//AN_Test_Eye2( (FVECTOR *)work->world->m[3], 2 );

//if(GV_Time%120==0)printf("%d\n",work->result);
	DG_COPY_VEC( &work->from, (FVECTOR *)work->world->m[3] );
	DG_COPY_VEC( &work->to, (FVECTOR *)DG_Chanls->eye.m[3] );

	switch( work->result ){
	  case -1:	// 中心見えない
		DG_InvisiblePrim2( prim );
		return;
	  case 0:	// わかんない
		if( (irnd()>>8)&1 ){
			DG_VisiblePrim2( prim );
		}else{
			DG_InvisiblePrim2( prim );
			return;
		}
		break;
	  case 1:	// 中心見える
		DG_VisiblePrim2( prim );
		break;
	}

	_sceVu0SubVector( &diff, &work->to, &work->from );

	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( prim );
	clock = prim->buffer_clock;

	DG_SetPos( &DG_Chanls->eye_pers );
	DG_PutVector( &work->from, &center, 1 );

	center.vx/= center.vw;
	center.vy/= center.vw;

	pos = prim->pos[clock];
	if( center.vz > center.vw
	 || DG_FABS(center.vx) > 1.0f
	 || DG_FABS(center.vy) > 1.0f
	  ){
		DG_InvisiblePrim2( prim );
		return;
	}

	screen_near_x = SCREEN_NEAR / (ASPECT_X()                                       ) / DG_Chanls->screen;
	screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / DG_Chanls->screen;

	center.vw = (center.vw  > 0.0f)? center.vw: -center.vw;
	center.vx*= screen_near_x;
	center.vy*= screen_near_y;

	ratio = work->ratio;
	angle = work->angle;
	if( work->shadow_knife ){
		work->blink_angle+= PI*0.03f;
		width_parts = WIDTH_PARTS_SHADOW * (1.0f + sinf( work->blink_angle )*0.25f);
		for ( i = 0 ; i < N_PRIMS*N_POLYS ; i++ ){
			ftemp = (*angle++);
			radius_local = (*ratio++)*width_parts;
			rs = radius_local * sinf( ftemp );
			rc = radius_local * cosf( ftemp );

			pos->vx = center.vx + rs;
			pos->vy = center.vy + rc;
			pos->vz = SCREEN_NEAR;
			pos++;

			pos->vx = center.vx - rc*SHORT_RATIO;
			pos->vy = center.vy + rs*SHORT_RATIO;
			pos->vz = SCREEN_NEAR;
			pos++;

			pos->vx = center.vx + rc*SHORT_RATIO;
			pos->vy = center.vy - rs*SHORT_RATIO;
			pos->vz = SCREEN_NEAR;
			pos++;

			pos->vx = center.vx - rs;
			pos->vy = center.vy - rc;
			pos->vz = SCREEN_NEAR;
			pos++;
		}
	}else{
		for ( i = 0 ; i < N_PRIMS*N_POLYS ; i++ ){
			ftemp = (*angle++);
			radius_local = (*ratio++)*WIDTH_PARTS;
			rs = radius_local * sinf( ftemp );
			rc = radius_local * cosf( ftemp );

			pos->vx = center.vx + rs;
			pos->vy = center.vy + rc;
			pos->vz = SCREEN_NEAR;
			pos++;

			pos->vx = center.vx - rc*SHORT_RATIO;
			pos->vy = center.vy + rs*SHORT_RATIO;
			pos->vz = SCREEN_NEAR;
			pos++;

			pos->vx = center.vx + rc*SHORT_RATIO;
			pos->vy = center.vy - rs*SHORT_RATIO;
			pos->vz = SCREEN_NEAR;
			pos++;

			pos->vx = center.vx - rs;
			pos->vy = center.vy - rc;
			pos->vz = SCREEN_NEAR;
			pos++;
		}
	}



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

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
//	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 1, 2, 2, 1, 0xff ) );

	prim->raise = RAISE;

	if( work->shadow_knife ){
		col_r = COL_R_SHADOW;
		col_g = COL_G_SHADOW;
		col_b = COL_B_SHADOW;
		col_a = COL_A_SHADOW;
	}else{
		col_r = COL_R;
		col_g = COL_G;
		col_b = COL_B;
		col_a = COL_A;
	}

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
	float		*ratio;
	float		*angle;
	int			i;

	work->blink_angle = 0;
	ratio = work->ratio;
	angle  = work->angle;
	for( i=0; i<N_PRIMS*N_POLYS;i ++ ){
		(*ratio++) = 0.1f + rnd();
		(*angle++) = TPI*rnd();
	}


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

	if( !InitPacket2( work, prim, tex ) ) return -1;
	DG_InvisiblePrim2( prim );


	GV_SetActorChild( work,
		NewOnlineCheck_Static(
			GM_CurrentStageMap,
			&work->from,
			&work->to,
			HZX_CHK_ALL,
			HZX_SEG_NO_RECOIL | HZX_SEG_RECOIL_TYPE,
			HZX_FLOOR_NO_PLAYER,
			&work->result,
			4, 4, 1.0f )
	);

	return 0 ;
}

void *NewKirari( FMATRIX *world )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->shadow_knife = 0;
		work->world = world;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

void *NewKirariShadowKnife( FMATRIX *world )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->shadow_knife = 1;
		work->world = world;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

