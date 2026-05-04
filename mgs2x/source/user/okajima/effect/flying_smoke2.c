//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	flying_smoke2.c
	煙の尾を引く飛翔物（薄い煙）
	2000/05/19 S.Okajima
	$Id: flying_smoke2.c,v 1.1.1.3 2002/11/19 11:47:05 Yoshizawa1 Exp $

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

#include	"gameheader.h"
#include	"libmt.h"
#include	"../etc/ok_util.h"

#define	RGB_MIN			(196)
#define	RGB_RND_MAX		(255)
#define	RGB_RND_MIN		(230)
#define	RGB_SUB_STEP	(2)
#define	ALPHA			(32)
#define	TEX_PER			(4.0f)
#define	WIDTH_ADD_PER	(0.003f)

#define N_PRIMS		(4)
#define N_VERTS		(16)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x2000)


#define	DECAY_RATIO		(0.995f)

#define	WIN_MIN				(10.0f)
#define	WIN_MAX				(100.0f)

#define	RAISE			(0)


extern SVECTOR G_wind_rot;	/* 風向 */
extern int G_wind_intense;	/* 強さ */

/*----------------------------------------------------------------*/
typedef	struct	{
	GV_ACT_EX		actor ;

	DG_PRIM2	*prim ;

	int			now_num;
	int			size_add;
	float		size;
	float		size_randam;
	float		angle[N_PRIMS*N_VERTS];
	FVECTOR		pos;
	FVECTOR		vec;

	SVECTOR		wind_rot;
	FVECTOR		wind;
	FVECTOR		wind_before;

	int			alpha_step;

	int			life;

	HZX_GROUP_ID	map_id;

} Work ;
/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh;
	DG_PRIM2_UVRGBWH	*uvrgbwh_before;
	int	i;
	int		clock;
	int		count;
	float	ftemp;
	float	*angle=NULL;
	FVECTOR	local_wind;
	FVECTOR	*pos;
	FVECTOR	*pos_before;

   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;

	work->wind.vx=0.0f;
	work->wind.vz=0.0f;
	ftemp = G_wind_intense;
	if( ftemp < WIN_MIN ){
		ftemp = -WIN_MIN;
	}else if( ftemp > WIN_MAX ){
		ftemp = -WIN_MAX;
	}else{
		ftemp = -ftemp;
	}
	OK_FloatSmoother( &work->wind.vy, &ftemp, 0.95f );
	OK_DirectionSmoother( &work->wind_rot, &G_wind_rot, 0.999f );
	DG_SetPos2( &DG_ZeroVector, &work->wind_rot );
	DG_RotVector( &work->wind, &local_wind, 1 );
	local_wind.vy = 0.0f;

	count          = 0;
	angle          = work->angle;
	pos            = work->prim->pos[clock];
	pos_before     = work->prim->pos[1-clock];
	uvrgbwh        = work->prim->uvrgb[clock];
	uvrgbwh_before = work->prim->uvrgb[1-clock];
	for( i=0; i<N_PRIMS*N_VERTS; i++ ){
		pos->vx = pos_before->vx + (*angle)*local_wind.vx;
		pos->vy = pos_before->vy;
		pos->vz = pos_before->vz + (*angle)*local_wind.vz;
		if(work->life%4==0){
			uvrgbwh->a = ( uvrgbwh_before->a < work->alpha_step )? 0: uvrgbwh_before->a - work->alpha_step;
		}else{
			uvrgbwh->a = uvrgbwh_before->a;
		}
		if( uvrgbwh->a == 0 ){
			uvrgbwh->w = 0;
			uvrgbwh->h = 0;
			uvrgbwh->r  = uvrgbwh->g = uvrgbwh->b = 0 ;
			count++;
		}else{
			/* 本来の指定方法と矛盾するが採用 */
			uvrgbwh->w = uvrgbwh_before->w + work->size_add;
			uvrgbwh->h = uvrgbwh_before->h + work->size_add;
			uvrgbwh->b = ( uvrgbwh_before->b < RGB_MIN + RGB_SUB_STEP )? RGB_MIN: uvrgbwh_before->b - RGB_SUB_STEP;
			uvrgbwh->r  = uvrgbwh->g = uvrgbwh->b ;
		}

		angle++;
		pos++;
		pos_before++;
		uvrgbwh++;
		uvrgbwh_before++;
	}

	/* 先頭初期化 */
	if( work->life > 0 ){
		//---------
		angle   = &work->angle[work->now_num];
		pos     = &work->prim->pos[clock][work->now_num];
		uvrgbwh = ((DG_PRIM2_UVRGBWH *)work->prim->uvrgb[clock]) + work->now_num;
		uvrgbwh->a = (int)((float)ALPHA * (rnd() + 1.0f) * 0.5f) ;

		uvrgbwh->r  = uvrgbwh->g = uvrgbwh->b = irnd() % (RGB_RND_MAX - RGB_RND_MIN) + RGB_RND_MIN;

		(*angle) = rnd();
		if( irnd()%8!=0 ){
			uvrgbwh->w = (int)(cosf( (*angle)*TPI ) * work->size);
			uvrgbwh->h = (int)(sinf( (*angle)*TPI ) * work->size);
		}else{
			ftemp = work->size * (1.0f + rnd());
			uvrgbwh->w = (int)(cosf( (*angle)*TPI ) * ftemp );
			uvrgbwh->h = (int)(sinf( (*angle)*TPI ) * ftemp);
		}

		work->life--;

		//---------

		work->vec.vx *= DECAY_RATIO;
		work->vec.vy *= DECAY_RATIO;
		work->vec.vz *= DECAY_RATIO;

		work->vec.vy += P_GRAVITY*0.8f;

		OK_StepCheckHzd( &work->pos, &work->pos, &work->vec, 0.9f, work->size, work->map_id );
//		OK_StepCheckHzd( &work->pos, &work->vec, 0.9f, work->size, work->map_id );
//		_sceVu0AddVector( &work->pos, &work->pos, &work->vec );

		pos->vx = work->pos.vx + work->size_randam * frnd();
		pos->vy = work->pos.vy + work->size_randam * frnd();
		pos->vz = work->pos.vz + work->size_randam * frnd();
//		DG_COPY_VEC( pos, &work->pos );
	}else if( count >= N_PRIMS*N_VERTS ){
		GV_DestroyActor( work ) ;
	}

	work->now_num++;
	if( work->now_num >= N_PRIMS*N_VERTS ) work->now_num = 0;
}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

static int InitPacket( Work *work, DG_PRIM2 *prim, DG_TEX *tex, FVECTOR *center )
{
	FVECTOR		*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		i, k ;

	prim->raise = RAISE;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );

	pos        = SCR_POS;
	uvrgbwh    = SCR_UVS;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			DG_COPY_VEC( pos, center );

			uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh->q0 = 4096 ;
			uvrgbwh->q1 = 4096 ;
			uvrgbwh->f0 = 0x0fff ;
			uvrgbwh->f1 = 0x0fff ;

			uvrgbwh->h  = uvrgbwh->w  = 0;
			uvrgbwh->r  = uvrgbwh->g = uvrgbwh->b = 0;
			uvrgbwh->a  = 0 ;

			pos ++;
			uvrgbwh ++ ;
		}
	}


	OK_Scr_Mem( prim->pos[ 0 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->pos[ 1 ],   SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS );

	return 1;
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, FVECTOR *pos, FVECTOR *vector, int life )
{
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	work->map_id = GM_GetHzxGroupID( GM_CurrentStageMap );
	work->now_num = 0;

	work->life = life;
	DG_COPY_VEC( &work->pos, pos );
	DG_COPY_VEC( &work->vec, vector );

	work->size        = GV_VecLen3F( vector ) * TEX_PER;
	work->size_add    = (int)(work->size * WIDTH_ADD_PER);
	work->size_randam = work->size * 0.25f;
	work->alpha_step  = ALPHA / (N_PRIMS * N_VERTS) + 1;

	tex = DG_GetTexture( 9868771 /*"bombgas6_alp"*/ );

	/* 回転スプライト */
	prim = work->prim = GM_MakePrim2(
				DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA,
				N_PRIMS,
				N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket( work, prim, tex, pos );
	prim->flag |= (DG_PRIM2_INVISIBLE1|DG_PRIM2_INVISIBLE2|DG_PRIM2_INVISIBLE3);


#if 0
	work->wind_rot.vx = G_wind_rot.vx;
	work->wind_rot.vy = G_wind_rot.vy;
	work->wind_rot.vz = G_wind_rot.vz;
#else
	work->wind_rot.vx = -irnd()%2048;
	work->wind_rot.vy =  irnd()%4096;
	work->wind_rot.vz =  0;
	OK_DirectionSmoother( &work->wind_rot, &G_wind_rot, 0.8f );
#endif

	return 0 ;
}


/* [薄い煙表現]
pos    :発生位置（へのポインタ）
vector :飛翔方向ベクトル（へのポインタ）
life   :消滅までのフレーム数
*/
void *NewFlyingSmoke2( FVECTOR *pos, FVECTOR *vector, int life )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, pos, vector, life ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
