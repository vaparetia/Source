//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	bubble_motion.c
	細かい泡でラインを生成

	2001/03/16 S.Okajima
	$Id: bubble_motion.c,v 1.1.1.3 2002/11/19 11:47:18 Yoshizawa1 Exp $
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

#include	"camera.h"
#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"
#include	"./bubble.h"

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;

/*----------------------------------------------------------------*/
#define	RAISE			(0)

//#define	SCALE_DOWN		(0.85f)
#define	SCALE_DOWN		(1.5f)
#define	DECAY_VEC		(0.80f)

#define	SCREEN_NEAR			( 51.0f )

//スクラッチパッドの一部をランダムテーブルに使用
#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_VEC		(SCRPAD_ADDR + 0x2000)
#define	SCR_RND		(SCRPAD_ADDR + 0x3000)
#define N_PRIMS		(1)
#define N_VERTS		(32)
#define	RANDAM_FIELD_NUM	(0x1000/4)

#define PENETRATE_MUL	(4.0f)


#define	P_ALPHA_MAX		(128)
#define	P_RGB_MAX		(128)
#define	SIZE			(8)
#define	SIZE_MIN		(5)


typedef	struct	{
	GV_ACT_EX		actor ;
	int			name ;
	int			map ;

	int			life;
	int			life_max;

	int		top_num;

	FVECTOR dummy_center;
	FVECTOR pos;
	FVECTOR vec;

	FVECTOR		prim_vec[N_PRIMS*N_VERTS];
	DG_PRIM2	*prim ;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	clock;
	int	i;
	int	alpha;
	int	activ_count;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	FVECTOR	*pos;
	FVECTOR	*vec;
	float	*p_randam;

	OK_frnd_to_scr( SCR_RND, RANDAM_FIELD_NUM );
	p_randam = (float *)SCR_RND;

	DG_VisiblePrim2( work->prim );

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;

	//AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;

	OK_Mem_Scr( SCR_POS, work->prim->pos[1-clock], sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
	OK_Mem_Scr( SCR_VEC, work->prim_vec,           sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;

	activ_count = 0;
	alpha = P_ALPHA_MAX  * work->life / work->life_max;
//	alpha = BUBBLE_ALPHA * work->life / work->life_max;
	pos = SCR_POS;
	vec = SCR_VEC;
	uvrgbwh        = work->prim->uvrgb[  clock];
	for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){

//if(i==0)printf("%f %f %f\n",vec->vx,vec->vy,vec->vz);

		pos->vx += pos->vw * SCALE_DOWN * (*(p_randam++)) + vec->vx;
		pos->vy += pos->vw * BUBBLE_SPEED                 + vec->vy;
		pos->vz += pos->vw * SCALE_DOWN * (*(p_randam++)) + vec->vz;

		if( pos->vy >= GM_WaterLevel ){
			pos->vy = GM_WaterLevel;
			uvrgbwh->a = 0;
			activ_count++;
		}else{
			_sceVu0ScaleVector( vec, vec, DECAY_VEC );
		}

		uvrgbwh->a = alpha;

		pos++;
		vec++;
		uvrgbwh++;
	}
	OK_Scr_Mem( work->prim->pos[  clock], SCR_POS, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;
	OK_Scr_Mem( work->prim_vec,           SCR_VEC, sizeof(FVECTOR), N_VERTS*N_PRIMS ) ;


	work->life--;
	if( work->life <= 0  ||  activ_count >= N_PRIMS*N_VERTS ){
		GV_DestroyActor( work ) ;
	}

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	FVECTOR		*vec;
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i;
	float	size;
	float	ratio;
	int		isize;


	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	pos0 = prim->pos[ 0 ];
	pos1 = prim->pos[ 1 ];
	vec = work->prim_vec;
	uvrgbwh0   = prim->uvrgb[ 0 ];
	uvrgbwh1   = prim->uvrgb[ 1 ];
	for ( i = 0 ; i < N_PRIMS*N_VERTS ; i++ ){
		size = rnd();
		size*= size*size;
//		pos0->vw = BUBBLE_SIZE_MIN + size*BUBBLE_SIZE_RAND;
		pos0->vw =        SIZE_MIN + size*       SIZE;
		isize  = (int)pos0->vw;


		ratio = rnd();
		pos0->vx  = work->pos.vx -  work->vec.vx                           *ratio;
		pos0->vy  = work->pos.vy - (work->vec.vy + pos0->vw * BUBBLE_SPEED)*ratio;
		pos0->vz  = work->pos.vz -  work->vec.vz                           *ratio;

		if( pos0->vy > GM_WaterLevel ){
			pos0->vy = GM_WaterLevel;
		}

		ratio = pos0->vw * SCALE_DOWN;
		pos0->vx += ratio * frnd();
		pos0->vy += ratio * frnd();
		pos0->vz += ratio * frnd();
		DG_COPY_VEC( pos1, pos0 );
//		_sceVu0ScaleVector( vec, &work->vec, rnd()*1.5f );
		_sceVu0ScaleVector( vec, &work->vec, 1.0f - size );


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
		uvrgbwh1->r  = uvrgbwh0->r  = P_RGB_MAX ;
		uvrgbwh1->g  = uvrgbwh0->g  = P_RGB_MAX ;
		uvrgbwh1->b  = uvrgbwh0->b  = P_RGB_MAX ;
//		uvrgbwh1->r  = uvrgbwh0->r  = BUBBLE_COL_R ;
//		uvrgbwh1->g  = uvrgbwh0->g  = BUBBLE_COL_G ;
//		uvrgbwh1->b  = uvrgbwh0->b  = BUBBLE_COL_B ;
		uvrgbwh1->a  = uvrgbwh0->a  = 0 ;

		vec++;
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

	work->top_num = 0;


	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );

	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}

	if( !InitPacket2( work, prim, tex ) ) return -1;
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);

	return 0 ;
}


/*
pos から vec の間に泡を出す
*/
void *NewBubbleMotion( FVECTOR *pos, FVECTOR *vec, int life )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->pos, pos );
		DG_COPY_VEC( &work->vec, vec );
		if( life <= 0 ) life = 1;
		work->life = work->life_max = life;

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
