//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	blood_part2.c
	主観カメラ用・びっくり血
	2000/01/26 S.Okajima
	$Id: blood_part2.c,v 1.1.1.3 2002/11/19 11:46:59 Yoshizawa1 Exp $
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
#include	"blood.h"
#include	"../etc/ok_util.h"

extern void PutCameraBlood( FVECTOR *pos_pers, int white );

/*----------------------------------------------------------------*/
//#define	BLOOD_GRAVITY	(5.0f)

#define	SPEED			(200.0f)
#define	SPEED_RAND		(SPEED * 0.8f)
#define	ANGLE_RAND		(128)

#define	WIDTH_RATIO		(0.7f)

#define	N_VERTS		(32)
#define	N_PRIMS		(1)

#ifdef ENGLISH
#define	MAX_ALPHA		(128)
#else
#define	MAX_ALPHA		(BLOOD_ALPHA_0)
#endif

#define	LIFE_TIME		( 15 )

#define	SIZE_MIN		( 20 )
#define	SIZE_RND		( 50 )


typedef	struct	{
	GV_ACT_EX		actor ;
	int			map ;

	DG_PRIM2	*prim ;

	FMATRIX		*world;		/* 被付随行列 */
	FVECTOR		pos[N_PRIMS*N_VERTS];
	FVECTOR		vec[N_PRIMS*N_VERTS];
	int			flag[N_PRIMS*N_VERTS];

	int			count;
	int			life;
	int			white;
	int			prim_num;
} Work ;

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	i,j;
	FVECTOR		*vec;
	FVECTOR		*pos;
	FVECTOR		*prim_pos;
	DG_PRIM2_UVRGBWH	*uvrgb;
	FVECTOR		pers;
	FVECTOR		cam;
	int			*flag;
	int			alpha;
	int			clock;

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
	DG_SwitchBuffPrim2( work->prim );
	clock = work->prim->buffer_clock;

	DG_SetPos( &DG_Chanls->eye_pers );

	alpha = MAX_ALPHA * work->life / LIFE_TIME;

	DG_COPY_VEC( &cam, (FVECTOR *)DG_Chanls->eye.m[3] ) ;

	vec  = work->vec;
	pos  = work->pos;
	flag = work->flag;
	prim_pos = work->prim->pos[clock];
	uvrgb    = work->prim->uvrgb[clock];
	for( i=0; i<N_PRIMS; i++ ){
		for( j=0; j<N_VERTS; j++ ){
			vec->vy += P_GRAVITY;
			_sceVu0AddVector( pos, pos, vec );
			DG_COPY_VEC( prim_pos, pos );
//AN_Test_Eye2( pos, 2 );

			if( (*flag)!=0 ){
				DG_PutVector( pos, &pers, 1 );
				if( pers.vz >  pers.vw ){	/* 画面奥にきた */

					if( OK_RectLen( pos, &cam ) < 1500.0f ){
						pers.vw = (pers.vw  > 0.0f)? pers.vw: -pers.vw;

						/* -1 ～ 1 */
						pers.vx /= pers.vw;
						pers.vy /= pers.vw;
						pers.vz /= pers.vw;
						if( pers.vy > -1.1f && pers.vy < 1.1f
						 && pers.vx > -1.1f && pers.vx < 1.1f ){
							(*flag)=0;
//printf("%f %f %f\n",pers.vx,pers.vy,pers.vz);
							PutCameraBlood( &pers, work->white );
						}
					}
				}
			}
			uvrgb->a = alpha;

			uvrgb++;
			flag++;
			prim_pos++;
			pos ++;
			vec ++;
		}
	}

	if( --work->life < 0) GV_DestroyActor( work ) ;

}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/* ---------------------------------------------------------------- */
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh0 ;	/* スプライト用 */
	DG_PRIM2_UVRGBWH	*uvrgbwh1 ;	/* スプライト用 */
	int		k ;

	DG_ConfigPrim2Tex( prim, tex );
#if 0
	if( work->white ){
//		prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}else{
//		prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) ;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
	}
#else
	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
	{
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}
	else
	{
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	}
#endif

	//-------------------------------
	uvrgbwh0 = prim->uvrgb[ 0 ] ;
	uvrgbwh1 = prim->uvrgb[ 1 ] ;
	for ( k = 0 ; k < N_VERTS ; k++ ){
		uvrgbwh0->u0 = uvrgbwh1->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh0->v0 = uvrgbwh1->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh0->u1 = uvrgbwh1->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh0->v1 = uvrgbwh1->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh0->q0 = uvrgbwh1->q0 = 4096 ;
		uvrgbwh0->q1 = uvrgbwh1->q1 = 4096 ;
		uvrgbwh0->f0 = uvrgbwh1->f0 = 0x0fff ;
		uvrgbwh0->f1 = uvrgbwh1->f1 = 0x0fff ;

		uvrgbwh0->h  = uvrgbwh0->w  = uvrgbwh1->h  = uvrgbwh1->w = SIZE_MIN + (irnd()>>8)%SIZE_RND ;

		uvrgbwh0->r  = uvrgbwh1->r  = COLOR_R ;
		uvrgbwh0->g  = uvrgbwh1->g  = COLOR_G ;
		uvrgbwh0->b  = uvrgbwh1->b  = COLOR_B ;
		uvrgbwh0->a  = uvrgbwh1->a  = MAX_ALPHA ;

		uvrgbwh0 ++ ;
		uvrgbwh1 ++ ;
	}

}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, FVECTOR *center, SVECTOR *rot, int white )
{
	int	i,j;
	FVECTOR		*vec;
	FVECTOR		*pos;
	FVECTOR		*pos0;
	FVECTOR		*pos1;
	SVECTOR		local_rot;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	int			*flag;

	work->white=white;

	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
	{
	tex = DG_GetTexture( 9373287 /*"vr2_chi01_alp"*/ );
	}
	else
	{
//	tex = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
	tex = DG_GetTexture( 10972307 /*"chi01_alp"*/ );
	}

	prim = work->prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );

	vec = work->vec;
	pos  = work->pos;
	pos0 = work->prim->pos[0];
	pos1 = work->prim->pos[1];
	flag = work->flag;
	for( i=0; i<N_PRIMS; i++ ){
		for( j=0; j<N_VERTS; j++ ){
			DG_COPY_VEC( pos,  center );
			DG_COPY_VEC( pos0, center );
			DG_COPY_VEC( pos1, center );

			vec->vx = 0;
			vec->vy = 0;
			vec->vz = -(SPEED - rnd() * SPEED_RAND);
			local_rot.vx = rot->vx + irnd() % ANGLE_RAND - ANGLE_RAND/2;
			local_rot.vy = rot->vy + irnd() % ANGLE_RAND - ANGLE_RAND/2;
			local_rot.vz = irnd() % 4096;
			DG_SetPos2( center, &local_rot );
			DG_RotVector( vec, vec, 1 );

			(*flag)=1;

			flag++;
			pos ++;
			pos0++;
			pos1++;
			vec ++;
		}
	}

	work->life = LIFE_TIME;
	work->count=0;

	return 0 ;
}

/*
[pos  ]:発生位置
[rot  ]:発生角度
[white]:0:赤 1:白
*/
void *NewBloodPart2( FVECTOR *center, SVECTOR *rot, int white )
{
	Work		*work ;

	if( GM_Configuration & GM_CONFIG_BLOOD_OFF ) return NULL;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, center, rot, white ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
