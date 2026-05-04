//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	tear_drops.c
	おつる涙
	2000/04/27 S.Okajima
	$Id: tear_drops.c,v 1.1.1.3 2002/11/19 11:47:15 Yoshizawa1 Exp $
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

/*----------------------------------------------------------------*/
#define	SPEED_MAX		(5.0f)
#define	SPEED_MIN		(3.0f)
#define	ANGLE_MAX		(512)
#define	ANGLE_ADD		(64)

#define	BLOOD_M9_GRAVITY	(-0.5f)

#define	N_VERTS		(16)
#define	N_PRIMS		(4)


#define	LIFE_TIME		( N_VERTS * N_PRIMS )

#define	SIZE		( 15 )

#define	MAX_CHECK		( 4 )

#define	COLOR_R			( 255 )
#define	COLOR_G			( 255 )
#define	COLOR_B			( 255 )
#define	MAX_ALPHA		( 32 )

typedef	struct	{
	GV_ACT_EX		actor ;
	int			map ;

	FMATRIX		*world;
	FVECTOR		mov;

	DG_PRIM2	*prim ;
	DG_PRIM2	*prim_inter ;

	FVECTOR		vec[N_PRIMS*N_VERTS];

	float		speed;
	SVECTOR		rot;

	int			count;
	int			life;
	int			white;
	int			prim_num;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	i,j;
	SVECTOR		rot;
	FVECTOR		mov;
	FVECTOR		norm;
	FVECTOR		keep_pos;
	FVECTOR		*vec;
	FVECTOR		*prim_pos_inter;
	FVECTOR		*prim_pos;
	FVECTOR		*prim_pos_before;
	DG_PRIM2_UVRGBWH	*uvrgbwh_inter ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int			clock;
	int			flag;
	int			alpha;

	GM_GroupPrim2( work->prim,       GM_CurrentStageMap ) ;
	GM_GroupPrim2( work->prim_inter, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	DG_SwitchBuffPrim2( work->prim_inter );
	clock = work->prim->buffer_clock;

	norm.vx = 0.0f;
	norm.vy = 0.0f;
	norm.vz = work->speed;
	DG_SetPos( work->world );
	DG_PutVector( &work->mov,  &mov,  1 );
	DG_RotVector( &norm, &norm, 1 );

	//---------------------------
	if( work->life < MAX_ALPHA ){
		alpha = work->life ;
		if( alpha<0 ){
			alpha=0;
		}
	}else{
		alpha = MAX_ALPHA;
	}

	uvrgbwh_inter   = work->prim_inter->uvrgb[clock];
	uvrgbwh         = work->prim->uvrgb[clock];
	prim_pos_inter  = work->prim_inter->pos[clock];
	prim_pos        = work->prim->pos[  clock];
	prim_pos_before = work->prim->pos[1-clock];
	DG_COPY_VEC( &keep_pos, prim_pos );
	vec  = work->vec;
	flag = 0;
	for( i=0; i<N_PRIMS; i++ ){
		for( j=0; j<N_VERTS; j++ ){
			if( vec->vw==0.0f ){
				if( flag==0 && irnd()%8!=0){
					flag = 1;
					DG_COPY_VEC( vec, &norm );
					rot.vx = work->rot.vx + irnd()%ANGLE_ADD;
					rot.vy = work->rot.vy + irnd()%ANGLE_ADD;
					rot.vz = work->rot.vz + irnd()%ANGLE_ADD;
					DG_SetPos2( &DG_ZeroVector, &rot );
					DG_RotVector( vec, vec, 1 );
					DG_COPY_VEC( prim_pos, &mov );
					vec->vw = 1.0f;
				}
			}else if( vec->vw==1.0f ){
				prim_pos->vx = prim_pos_before->vx + vec->vx;
				prim_pos->vy = prim_pos_before->vy + vec->vy;
				prim_pos->vz = prim_pos_before->vz + vec->vz;

				prim_pos_inter->vx = (prim_pos->vx + keep_pos.vx)*0.5f;
				prim_pos_inter->vy = (prim_pos->vy + keep_pos.vy)*0.5f;
				prim_pos_inter->vz = (prim_pos->vz + keep_pos.vz)*0.5f;

				DG_COPY_VEC( &keep_pos, prim_pos );
				vec->vy += BLOOD_M9_GRAVITY;

				uvrgbwh_inter->a = alpha;
				uvrgbwh->a = alpha;
			}else{
				uvrgbwh_inter->a = 0;
				uvrgbwh->a = 0;
			}

			uvrgbwh_inter++;
			uvrgbwh++;
			prim_pos_inter ++;
			prim_pos ++;
			prim_pos_before ++;
			vec ++;
		}
	}
	if( irnd()%4==0 || work->speed<SPEED_MIN ){
		work->rot.vx = irnd()%ANGLE_MAX - ANGLE_MAX/2;
		work->rot.vy = irnd()%ANGLE_MAX - ANGLE_MAX/2;
		work->rot.vz = irnd()%ANGLE_MAX - ANGLE_MAX/2;
		work->speed  = SPEED_MAX*(rnd()+1.0f)*0.5f;
	}else{
		work->rot.vx += irnd()%ANGLE_ADD - ANGLE_ADD/2;
		work->rot.vy += irnd()%ANGLE_ADD - ANGLE_ADD/2;
		work->rot.vz += irnd()%ANGLE_ADD - ANGLE_ADD/2;
		work->speed  *= 0.97f;
	}


	if( --work->life < 0){
		GV_DestroyActor( work ) ;
	}

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
	work->prim_inter = OK_FreePrim2( work->prim_inter );
}

/* ---------------------------------------------------------------- */
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		i,k ;
	float	angle;
	float	size_rnd;

	DG_ConfigPrim2Tex( prim, tex );
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );

	//-------------------------------
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for( i=0; i<N_PRIMS; i++ ){
		for ( k = 0 ; k < N_VERTS ; k++ ){
			uvrgbwh1->u0 = uvrgbwh0->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
			uvrgbwh1->v0 = uvrgbwh0->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
			uvrgbwh1->u1 = uvrgbwh0->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
			uvrgbwh1->v1 = uvrgbwh0->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
			uvrgbwh1->q0 = uvrgbwh0->q0 = 4096 ;
			uvrgbwh1->q1 = uvrgbwh0->q1 = 4096 ;
			uvrgbwh1->f0 = uvrgbwh0->f0 = 0x0fff ;
			uvrgbwh1->f1 = uvrgbwh0->f1 = 0x0fff ;

			angle = rnd();
			size_rnd = SIZE * (rnd() + 1.0f) * 0.5f;
			uvrgbwh1->w = uvrgbwh0->w = (int)(cosf( angle*TPI ) * size_rnd);
			uvrgbwh1->h = uvrgbwh0->h = (int)(sinf( angle*TPI ) * size_rnd);

			uvrgbwh1->r = uvrgbwh0->r = COLOR_R ;
			uvrgbwh1->g = uvrgbwh0->g = COLOR_G ;
			uvrgbwh1->b = uvrgbwh0->b = COLOR_B ;
			uvrgbwh1->a = uvrgbwh0->a = 0 ;

			uvrgbwh0 ++ ;
			uvrgbwh1 ++ ;
		}
	}

}

/*----------------------------------------------------------------*/
static int GetResources( Work *work )
{
	int	i,j;
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	FVECTOR		*vec;
	FVECTOR		center;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;

	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
//	tex = DG_GetTexture( 10972307 /*"chi01_alp"*/ );


	/* 落ちる血 */
	prim = work->prim = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );
	DG_VisiblePrim2( prim ) ;

	prim = work->prim_inter = GM_MakePrim2( DG_PRIM2_RSPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );
	DG_VisiblePrim2( prim ) ;


	DG_COPY_VEC( &center, (FVECTOR *)work->world->m[3] );
	pos0 = work->prim->pos[0];
	pos1 = work->prim->pos[1];
	vec  = work->vec;
	for( i=0; i<N_PRIMS; i++ ){
		for( j=0; j<N_VERTS; j++ ){
			DG_COPY_VEC( pos0, &center );
			DG_COPY_VEC( pos1, &center );
			vec->vw = 0.0f;	/* フラグ */
			pos0++;
			pos1++;
			vec++;
		}
	}
	pos0 = work->prim_inter->pos[0];
	pos1 = work->prim_inter->pos[1];
	for( i=0; i<N_PRIMS; i++ ){
		for( j=0; j<N_VERTS; j++ ){
			DG_COPY_VEC( pos0, &center );
			DG_COPY_VEC( pos1, &center );
			pos0++;
			pos1++;
		}
	}

	work->life = LIFE_TIME;
	work->count=0;

	work->rot.vx = irnd()%ANGLE_MAX;
	work->rot.vy = irnd()%ANGLE_MAX;
	work->rot.vz = irnd()%ANGLE_MAX;
	work->speed  = SPEED_MAX*rnd();

	return 0 ;
}

/*
[world]:[常に参照する]発生位置の座標系
[mov  ]:world からの相対発生位置
*/
void *NewTearDrops( FMATRIX *world, FVECTOR *mov )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		work->world = world;
		DG_COPY_VEC( &work->mov, mov );

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
