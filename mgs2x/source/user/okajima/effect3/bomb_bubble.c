//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	bomb_bubble.c
	爆発泡
	2001/05/05 S.Okajima
	$Id: bomb_bubble.c,v 1.1.1.3 2002/11/19 11:47:28 Yoshizawa1 Exp $

*/


#ifdef PSX2
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
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"
#include	"../effect2/bubble.h"

//スクラッチパッドの一部をランダムテーブルに使用
#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_RND		(SCRPAD_ADDR + 0x3000)
#define N_PRIMS		(16)
#define N_VERTS		(32)
#define	RANDAM_FIELD_NUM	(0x1000/4)

#define	RAISE				(0)

#define	INIT_PART_NUM		(16)

#define	FREQUENCY_RATIO_MASK	(0x07)


#define	LOCAL_ALPHA	(16)


/*----------------------------------------------------------------*/
extern float GM_WaterLevel;

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;

	DG_PRIM2	*prim ;
	FVECTOR		center;

	float		radius;
	int			life;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int		i;
	int		clock;
	float	surface;
	float	alpha;
	float	*p_randam;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	FVECTOR	*pos;


	surface = GM_WaterLevel;

	OK_frnd_to_scr( SCR_RND, RANDAM_FIELD_NUM );
	p_randam = (float *)SCR_RND;

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;



	// update
	if( work->life < LOCAL_ALPHA ){
		alpha = work->life;
	}else{
		alpha = LOCAL_ALPHA;
	}
	if( alpha < 0 ) alpha = 0;

	OK_Mem_Scr( SCR_POS, work->prim->pos[1-clock], sizeof(FVECTOR), N_PRIMS*N_VERTS );
	uvrgbwh        = work->prim->uvrgb[clock];
	pos            = SCR_POS;
	for( i=0; i<N_PRIMS*N_VERTS; i++ ){
		if( pos->vw!=0.0f ){
			pos->vx += pos->vw * (*(p_randam++));
			pos->vy += pos->vw * BUBBLE_SPEED;
			pos->vz += pos->vw * (*(p_randam++));

			uvrgbwh->a = alpha;
			if( (pos->vy + pos->vw) > surface ){
				uvrgbwh->a = 0;
				uvrgbwh->w = uvrgbwh->h = 0;
				pos->vw = 0.0f;
			}
		}else{
			uvrgbwh->a = 0;
			uvrgbwh->w = uvrgbwh->h = 0;
		}

		pos++;
		uvrgbwh++;
	}

	OK_Scr_Mem( work->prim->pos[  clock], SCR_POS, sizeof(FVECTOR), N_PRIMS*N_VERTS );


	if( work->life-- < 0) GV_DestroyActor( work ) ;

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i;
	float	size;
	int		isize;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos0       = prim->pos[ 0 ];
	pos1       = prim->pos[ 1 ];
	uvrgbwh0   = prim->uvrgb[ 0 ];
	uvrgbwh1   = prim->uvrgb[ 1 ];
	for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
		size = rnd();
		size*= size*size;
		pos1->vx = pos0->vx = work->center.vx + work->radius*frnd();
		pos1->vy = pos0->vy = work->center.vy + work->radius*frnd();;
		pos1->vz = pos0->vz = work->center.vz + work->radius*frnd();;
		pos1->vw = pos0->vw = (BUBBLE_SIZE_MIN + size*BUBBLE_SIZE_RAND)*4.0f;
		isize  = (int)pos0->vw;

		uvrgbwh1->w  = uvrgbwh0->w  = isize;
		uvrgbwh1->h  = uvrgbwh0->h  = isize;
		uvrgbwh1->u0 = uvrgbwh0->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh1->v0 = uvrgbwh0->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh1->u1 = uvrgbwh0->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh1->v1 = uvrgbwh0->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh1->q0 = uvrgbwh0->q0 = 4096 ;
		uvrgbwh1->q1 = uvrgbwh0->q1 = 4096 ;
		uvrgbwh1->f0 = uvrgbwh0->f0 = 0x0fff ;
		uvrgbwh1->f1 = uvrgbwh0->f1 = 0x0fff ;
		uvrgbwh1->r  = uvrgbwh0->r  = BUBBLE_COL_R ;
		uvrgbwh1->g  = uvrgbwh0->g  = BUBBLE_COL_G ;
		uvrgbwh1->b  = uvrgbwh0->b  = BUBBLE_COL_B ;
		uvrgbwh1->a  = uvrgbwh0->a  = 0 ;

		pos0 ++;
		pos1 ++;
		uvrgbwh0 ++ ;
		uvrgbwh1 ++ ;
	}

	return 1;
}
/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	prim = work->prim = GM_MakePrim2(
		DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_FOG, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );
	InitPacket( work, prim, tex );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}

void *NewBombBubble( FVECTOR *center, float radius, int life )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->center, center );
		work->radius = radius;
		work->life = DIRECT_TICK( life + LOCAL_ALPHA );

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

