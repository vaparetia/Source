//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	bubble_bullet.c
	水中弾道の泡
	2001/06/22 S.Okajima
	$Id: bubble_bullet.c,v 1.1.1.3 2002/11/19 11:47:29 Yoshizawa1 Exp $

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
#endif	///

#include <libutl.h>

#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"
#include	"../effect2/bubble.h"

//スクラッチパッドの一部をランダムテーブルに使用
//#define	SCR_POS		(SCRPAD_ADDR)
//#define	SCR_RND		(SCRPAD_ADDR + 0x3000)
//#define	SCR_UVR		(SCRPAD_ADDR + 0x3000)
#define N_PRIMS		(2)
#define N_VERTS		(32)
#define	RANDAM_FIELD_NUM	(0x1000/4)

#define	RAISE				(0)

#define	FREQUENCY_RATIO_MASK	(7)

//#define	LIFE	(60)
#define	LIFE	(8)

#define LOCAL_ALPHA	(64)

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;

	DG_PRIM2	*prim ;
	int			total_num;
	int			life;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int		i;
	int		clock;
	int		i_alpha;
	float	alpha;
	float	const *p_randam;
	FVECTOR			const *pos_prev;
   FVECTOR				*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	FVECTOR	fvtemp;
	DG_PRIM2	*prim ;

	prim = work->prim;

	DG_VisiblePrim2( prim );

	p_randam = OK_frnd_to_ptr( RANDAM_FIELD_NUM );

	GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( prim ) )
   {
      return;
   }
	clock = prim->buffer_clock;
	// update
	if( work->life < LOCAL_ALPHA ){
		alpha = (float)work->life;
	}else{
		alpha = (float)LOCAL_ALPHA;
	}
	if( alpha < 0.0f ) alpha = 0.0f;

	i_alpha = (int)alpha;

	uvrgbwh        = prim->uvrgb[clock];
   pos_prev       = prim->pos[1-clock];
   pos            = prim->pos[clock];

	for( i=0; i<work->total_num; i++ ){
      *pos = *pos_prev;
#if BP_VITA
      __builtin_pld( pos_prev + 1 );
#endif
		if( pos->vw!=0.0f ){
			pos->vx += pos->vw * (*(p_randam++));
			pos->vy += pos->vw * BUBBLE_SPEED;
			pos->vz += pos->vw * (*(p_randam++));

			uvrgbwh->a = i_alpha;

			if( (pos->vy + pos->vw) > GM_WaterLevel ){
				uvrgbwh->a = 0;
				uvrgbwh->w = uvrgbwh->h = 0;
				pos->vw = 0.0f;
				if( (i&FREQUENCY_RATIO_MASK)==0 && gAS_DG_HackArea != kDGHA_w32a ){
					if( pos->vy - (BUBBLE_SIZE_MIN + BUBBLE_SIZE_RAND) < GM_WaterLevel ){
						extern int OK_PutRipple( FVECTOR *center );
						DG_COPY_VEC( &fvtemp, pos );
						fvtemp.vy = GM_WaterLevel;
						OK_PutRipple( &fvtemp );
					}
				}
			}
		}else{
			uvrgbwh->a = 0;
			uvrgbwh->w = uvrgbwh->h = 0;
		}

		pos++;
      pos_prev++;
		uvrgbwh++;
	}

	if( work->life-- < 0) GV_DestroyActor( work ) ;

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *from, FVECTOR *to, float ratio )
{
	FVECTOR		diff;
	FVECTOR		*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i;
	int		isize;
	float	size;
	float	ftemp;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );

	_sceVu0SubVector( &diff, to, from ) ;

	pos       = prim->pos[0];
	uvrgbwh   = prim->uvrgb[0];
	for ( i = 0 ; i < work->total_num ; i++ ){
		size  = rnd();
		size *= size*size;
		ftemp = rnd();
		pos->vw = (BUBBLE_SIZE_MIN + size*BUBBLE_SIZE_RAND);
//		pos->vw = 100.0f;
#if 1
		pos->vx  = from->vx + diff.vx*ftemp;
		pos->vy  = from->vy + diff.vy*ftemp + pos->vw*BUBBLE_SPEED*(1.0f - ftemp);
		pos->vz  = from->vz + diff.vz*ftemp;
#else
		pos->vx  = from->vx;
		pos->vy  = from->vy;
		pos->vz  = from->vz;
#endif

		isize  = (int)pos->vw;
		uvrgbwh->w  = isize;
		uvrgbwh->h  = isize;
		uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh->q0 = 4096 ;
		uvrgbwh->q1 = 4096 ;
		uvrgbwh->f0 = 0x0fff ;
		uvrgbwh->f1 = 0x0fff ;
		uvrgbwh->r  = BUBBLE_COL_R ;
		uvrgbwh->g  = BUBBLE_COL_G ;
		uvrgbwh->b  = BUBBLE_COL_B ;
		uvrgbwh->a  = LOCAL_ALPHA ;
		uvrgbwh ++ ;
		pos ++;
	}
	for (       ; i < N_PRIMS*N_VERTS ; i++ ){
		uvrgbwh->w  = 0;
		uvrgbwh->h  = 0;
		uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh->q0 = 4096 ;
		uvrgbwh->q1 = 4096 ;
		uvrgbwh->f0 = 0x0fff ;
		uvrgbwh->f1 = 0x0fff ;
		uvrgbwh->r  = 0;
		uvrgbwh->g  = 0;
		uvrgbwh->b  = 0;
		uvrgbwh->a  = 0;
		uvrgbwh ++ ;
	}
//	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVR, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS);
//	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVR, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS*N_VERTS);
//	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS*N_VERTS);
//	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS*N_VERTS);
   memcpy( prim->uvrgb[1], prim->uvrgb[0], sizeof(DG_PRIM2_UVRGBWH) * N_PRIMS*N_VERTS);
   memcpy( prim->pos[1], prim->pos[0], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );

	return 1;
}
/*----------------------------------------------------------------*/
static int GetResources( Work *work, FVECTOR *from, FVECTOR *to, float ratio )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->total_num = (int)((float)(N_PRIMS*N_VERTS)*ratio);
	if( work->total_num > N_PRIMS*N_VERTS ){
		work->total_num = N_PRIMS*N_VERTS;
	}else if( work->total_num < 0 ){
		work->total_num = 0;
	}

//printf("work->total_num:%d\n",work->total_num);

	work->life = DIRECT_TICK( LIFE + LOCAL_ALPHA );

	tex = DG_GetTexture( 3594043 /*"drop01_msk"*/ );
//	tex = DG_GetTexture( 6715088 /*"rcm_l_msk"*/ );

	/* 回転スプライト */
	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_FOG, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);
	InitPacket( work, prim, tex, from, to, ratio );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	return 0 ;
}

void *NewBulletBubble( FVECTOR *from, FVECTOR *to, float ratio )
{
	Work		*work ;

	OPERATOR() ;
								
	// bullet.c よりも後

	
	work = (Work *)GV_NewEffect( GV_ACTOR_EFFECT, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		if ( GetResources( work, from, to, ratio ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

