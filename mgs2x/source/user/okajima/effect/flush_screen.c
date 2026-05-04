//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	flush_screen.c
	画面フラッシュ：ポリゴン版
	2000/11/10 S.Okajima
	$Id: flush_screen.c,v 1.1.1.3 2002/11/19 11:47:05 Yoshizawa1 Exp $
*/

#ifdef PSX2
#include <sys/types.h>
#endif
#ifdef PSX2
#include <eekernel.h>
#endif
#ifdef PSX2
#include <eeregs.h>
#endif
#ifdef PSX2
#include <libgraph.h>
#endif
#ifdef PSX2
#include <libdma.h>
#endif
#ifdef PSX2
#include <libdev.h>
#endif
#ifdef PSX2
#include <math.h>
#endif
#ifdef PSX2
#include <stdio.h>
#endif
#ifdef PSX2
#include <stdlib.h>
#endif
#ifdef PSX2
#include <sifdev.h>
#endif
#ifdef PSX2
#include <libvu0.h>
#endif
#include <libutl.h>

#include	"camera.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

/*----------------------------------------------------------------*/

#define	N_VERTS			(4)

#define	SCREEN_NEAR			( 51.0f )

#define	P_RGB_MAX		(128)
#define	P_ALP_MAX		(255)

typedef	struct	{
	GV_ACT_EX		actor ;

	int			life_max;
	int			life;
	DG_PRIM2	*prim ;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	FVECTOR	*pos;
	float	screen_near_x;
	float	screen_near_y;
	int		clock,i;


//	GM_GroupPrim2( work->prim, GM_PlayerMap ) ;

	DG_VisiblePrim2( work->prim );

	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	screen_near_x = SCREEN_NEAR / (ASPECT_X()                                           ) / DG_Chanls->screen;
	screen_near_y = SCREEN_NEAR / (ASPECT_Y() * DG_Chanls->width / DG_Chanls->height) / DG_Chanls->screen;

	screen_near_x*= 1.5f;
	screen_near_y*= 1.5f;

	pos = work->prim->pos[ clock ];
	uvrgbwh = work->prim->uvrgb[ clock ];
	for(i=0; i<N_VERTS; i++ ){
		pos->vz = SCREEN_NEAR;
		pos->vx = screen_near_x * frnd();
		pos->vy = screen_near_y * frnd();
//printf("%f %f %f\n",pos->vx,pos->vy,pos->vz);
		uvrgbwh->a = P_ALP_MAX * work->life / work->life_max;
		uvrgbwh->h = uvrgbwh->w = SCREEN_NEAR;	//画面の幅と合わせる
		uvrgbwh++;
		pos++;
	}

	work->life--;
	if( work->life < 0 ) GV_DestroyActor( work ) ;
}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for(i=0; i<N_VERTS; i++ ){
		uvrgbwh0->u0 = uvrgbwh1->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh0->v0 = uvrgbwh1->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh0->u1 = uvrgbwh1->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh0->v1 = uvrgbwh1->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh0->q0 = uvrgbwh1->q0 = 4096 ;
		uvrgbwh0->q1 = uvrgbwh1->q1 = 4096 ;
		uvrgbwh0->f0 = uvrgbwh1->f0 = 0x0fff ;
		uvrgbwh0->f1 = uvrgbwh1->f1 = 0x0fff ;
		uvrgbwh0->w  = uvrgbwh1->w  = 0;
		uvrgbwh0->h  = uvrgbwh1->h  = 0 ;
		uvrgbwh0->r  = uvrgbwh1->r  = P_RGB_MAX ;
		uvrgbwh0->g  = uvrgbwh1->g  = P_RGB_MAX ;
		uvrgbwh0->b  = uvrgbwh1->b  = P_RGB_MAX ;
		uvrgbwh0->a  = uvrgbwh1->a  = P_ALP_MAX ;
		uvrgbwh0++;
		uvrgbwh1++;
	}

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * N_VERTS );

	return 1;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;


	tex = DG_GetTexture( 12244206 /*"fog02_msk"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_ON_CAMERA, 1, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	if( !InitPacket2( work, prim, tex ) ) return -1;
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

void *NewFlushOnCamera( int life )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->life = work->life_max = life;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
