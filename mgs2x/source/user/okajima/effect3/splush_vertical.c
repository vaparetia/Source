//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	splush_vertical.c
	垂直水飛沫

	2001/05/30 S.Okajima
	$Id: splush_vertical.c,v 1.1.1.3 2002/11/19 11:47:37 Yoshizawa1 Exp $

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
extern float GM_WaterLevel;

/*----------------------------------------------------------------*/
#define	RAISE			(0)

//スクラッチパッドの一部をランダムテーブルに使用
#define	SCR_POS		(SCRPAD_ADDR + 0x0000)
#define	SCR_TMP		(SCRPAD_ADDR + 0x2000)

#define	COL_R		(255)
#define	COL_G		(255)
#define	COL_B		(255)
#define	COL_A		(255)


//#define N_VERTS			(32)
#define N_VERTS			(16)	/* rspr */
#define N_PRIMS			(0x2000 / 16 / N_VERTS)
#define N_LOOPS			(8)

#define	RANDAM_FIELD_NUM	(0x2000 / 4)

#define SIZE		(40)

#define SPEED_RND	(1.0f)

/* 画面前バンダリ */
#define	BOUND_WIDTH	(800.0f)
#define	BOUND_WIDTH_DOUBLE	(BOUND_WIDTH*2.0f)
/* バウンドの中心と、視点との距離 */
#define	CENTER_DISTANCE	(1000.0f)

typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;
	int			map ;

	DG_PRIM2	*prim ;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2 *prim;
	int	count;
	int	clock;
	int	i, j;
	FVECTOR	*before_pos;
	FVECTOR	*pos;
	FVECTOR	*sc_pos;
	FVECTOR	*sc_vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh;	/* スプライト用 */

	prim = work->prim;

	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;

	count = N_LOOPS*N_PRIMS*N_VERTS;
	before_pos = prim->pos[1-clock];
	pos        = prim->pos[  clock];
	uvrgbwh    = prim->uvrgb[  clock];
	for ( j=0 ; j < N_LOOPS ; j++ ){
		OK_Mem_Scr( SCR_POS, before_pos, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
		sc_pos = SCR_POS;
		sc_vec = SCR_TMP;
		for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
			sc_pos->vy += sc_pos->vw;

			if( sc_pos->vy < GM_WaterLevel ){
				uvrgbwh->w = uvrgbwh->h = 0;
				count--;
			}

			sc_pos++;
			uvrgbwh++;
		}
		OK_Scr_Mem( pos, SCR_POS, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
		before_pos += N_VERTS*N_PRIMS;
		pos        += N_VERTS*N_PRIMS;
	}

	if( count <= 0 ) GV_DestroyActor( work );

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos0       = prim->pos[ 0 ];
	pos1       = prim->pos[ 1 ];
	uvrgbwh0   = prim->uvrgb[ 0 ];
	uvrgbwh1   = prim->uvrgb[ 1 ];
	for ( i = 0 ; i < N_LOOPS*N_PRIMS*N_VERTS ; i++ ){

		uvrgbwh1->w  = uvrgbwh0->w  = SIZE;
		uvrgbwh1->h  = uvrgbwh0->h  = SIZE;
		uvrgbwh1->u0 = uvrgbwh0->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh1->v0 = uvrgbwh0->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh1->u1 = uvrgbwh0->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh1->v1 = uvrgbwh0->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh1->q0 = uvrgbwh0->q0 = 4096 ; 
		uvrgbwh1->q1 = uvrgbwh0->q1 = 4096 ;
		uvrgbwh1->f0 = uvrgbwh0->f0 = 0x0fff ;
		uvrgbwh1->f1 = uvrgbwh0->f1 = 0x0fff ;
		uvrgbwh1->r  = uvrgbwh0->r  = COL_R ;
		uvrgbwh1->g  = uvrgbwh0->g  = COL_G ;
		uvrgbwh1->b  = uvrgbwh0->b  = COL_B ;
		uvrgbwh1->a  = uvrgbwh0->a  = COL_A ;
		pos0 ++;
		pos1 ++;
		uvrgbwh0 ++ ;
		uvrgbwh1 ++ ;
	}

	return 1;
}

static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;


//	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
//	tex = DG_GetTexture( 13081966 /*"chi03_msk"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_LOOPS*N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	if( !InitPacket2( work, prim, tex ) ) return -1;
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

void *NewVerticalSplush( int name, int map )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor );

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

