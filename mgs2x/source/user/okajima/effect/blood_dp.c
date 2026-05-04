//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	blood.c
	落ちる血
	1999/09/01 S.Okajima
	$Id: blood_dp.c,v 1.1.1.3 2002/11/19 11:46:58 Yoshizawa1 Exp $
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

/*----------------------------------------------------------------*/

extern void	BloodSpread( FVECTOR *pos, SVECTOR *rot, int decay, float size, int addr );
extern void	*OK_BLOOD_SPREAD_WORK;
extern FVECTOR G_wind;

/* 固定 */
#define	N_PRIMS		(1)
#define	N_VERTS		(1)

//#define	LIFE_LIMIT	(240)
#define	LIFE_LIMIT	(15*60)

#define	FROM_UNDER_LIMIT	(-32000.0f)

#define	SIZE			(64)
//#define	GRAVITY			(5.0f)
#define	MAX_ALPHA		(128)

#define	NARR_LIMIT	(400.0f)
#define	STEP_LIMIT	(150.0f)

typedef	struct	{
	GV_ACT_EX		actor ;
	int			map ;

	int			addr ;

	DG_PRIM2	*prim ;

	int			floor_flag;
	float		under_limit;
	SVECTOR		floor_rot;

	int			white;
	int			life;
	int			decay;

	float		spread_size;

	FVECTOR		pos ;
	FVECTOR		vec ;
} Work ;


/*----------------------------------------------------------------*/
/* floor についての rotを得る */
static	void	CalcLocalRot_Floor( HZX_FLR *floor, SVECTOR *rot )
{
	FVECTOR	to;

	/* 法線収得 */
	to.vx=floor->p1.h;
	to.vy=floor->p3.h;
	to.vz=floor->p2.h;

	OK_DirVecXY( &DG_ZeroVector, &to, rot );
}

/*----------------------------------------------------------------*/
static	int	UpdateVectors( Work *work )
{
	work->vec.vy += P_GRAVITY;
	if(work->vec.vy < -300.0f) work->vec.vy = -300.0f;
	work->pos.vy += work->vec.vy;

	DG_SwitchBuffPrim2( work->prim );
	DG_COPY_VEC( work->prim->pos[ work->prim->buffer_clock ], &work->pos );


	if( work->floor_flag
	 && (work->pos.vy < work->under_limit)  ){
		work->pos.vy = work->under_limit;

//AN_Test_Eye2( &work->pos, 2 );
		if( work->spread_size > 0.0f ){
			BloodSpread( &work->pos, &work->floor_rot, work->decay, work->spread_size, work->addr );
		}

		return 0;
	}
	return 1;
}

/*----------------------------------------------------------------*/

static	void	Act( Work *work )
{
	if(work->life-- > 0){
		if( work->life > LIFE_LIMIT-10 ){
			DG_InvisiblePrim2( work->prim ) ;
		}else{
			DG_VisiblePrim2( work->prim ) ;
		}
		GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
		if( !UpdateVectors(work) ) GV_DestroyActor( work ) ;
	}else{
		GV_DestroyActor( work ) ;
	}
}

static void Die( Work *work )
{
	work->prim = OK_FreePrim2( work->prim );
}

/* ---------------------------------------------------------------- */
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */

	DG_ConfigPrim2Tex( prim, tex );
	if( work->white ){
//		prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) ;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}else{
		if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
		{
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
		}
		else
		{
//		prim->prim_env.alpha.data = SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) ;
		DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 2, 0, 0, 1, 0x00 ) );
		}
	}

	DG_COPY_VEC( prim->pos[0], &work->pos );
	DG_COPY_VEC( prim->pos[1], &work->pos );

	//-------------------------------
	uvrgbwh = prim->uvrgb[ 0 ] ;
	uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
	uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
	uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
	uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
	uvrgbwh->q0 = 4096 ;
	uvrgbwh->q1 = 4096 ;
	uvrgbwh->f0 = 0x0fff ;
	uvrgbwh->f1 = 0x0fff ;

	uvrgbwh->w = SIZE ;
	uvrgbwh->h = SIZE ;

	uvrgbwh->r = COLOR_R ;
	uvrgbwh->g = COLOR_G ;
	uvrgbwh->b = COLOR_B ;
	uvrgbwh->a = MAX_ALPHA ;

	//-------------------------------
	uvrgbwh = prim->uvrgb[ 1 ] ;
	uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
	uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
	uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
	uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
	uvrgbwh->q0 = 4096 ;
	uvrgbwh->q1 = 4096 ;
	uvrgbwh->f0 = 0x0fff ;
	uvrgbwh->f1 = 0x0fff ;

	uvrgbwh->w = SIZE ;
	uvrgbwh->h = SIZE ;

	uvrgbwh->r = COLOR_R ;
	uvrgbwh->g = COLOR_G ;
	uvrgbwh->b = COLOR_B ;
	uvrgbwh->a = MAX_ALPHA ;

}


/*----------------------------------------------------------------*/
static int GetResources( Work *work, FVECTOR *pos, int decay, int white, int spread_size, int addr )
{
	float		flr_height[2];

	DG_PRIM2	*prim ;
	DG_TEX		*tex ;
	HZX_FLR		flr[2];
	int			flr_atrs[2];

	DG_COPY_VEC( &work->pos, pos );
	DG_COPY_VEC( &work->vec, &DG_ZeroVector );

	work->addr = addr;


	work->spread_size = (float)spread_size;

	work->white=white;

	work->floor_flag = HZX_LevelHazardCheck(
	                         GM_GetHzxGroupID( GM_CurrentStageMap ),
	                         pos,
	                         HZX_CHK_ALL, 
	                         HZX_FLOOR_NO_BLOOD );


	work->under_limit = FROM_UNDER_LIMIT;
	if( work->floor_flag & 1 ){
		HZX_GetLevelHazard( flr, flr_atrs );
		if( !(flr[ 0 ].attribute & (HZX_FLOOR_NO_OBJECT|HZX_FLOOR_STEP) )  ){
			float	bound_min_x;
			float	bound_min_z;
			float	bound_max_x;
			float	bound_max_z;
			float	ftemp, ftemp2;

			CalcLocalRot_Floor( flr, &work->floor_rot );
			HZX_GetLevelHeight( flr_height );
			work->under_limit = flr_height[0] + 2.0f;

			bound_min_x = DG_MIN( flr->p1.x,   flr->p2.x );
			bound_min_x = DG_MIN( bound_min_x, flr->p3.x );
			bound_min_x = DG_MIN( bound_min_x, flr->p4.x );
			bound_min_z = DG_MIN( flr->p1.z,   flr->p2.z );
			bound_min_z = DG_MIN( bound_min_z, flr->p3.z );
			bound_min_z = DG_MIN( bound_min_z, flr->p4.z );
			bound_max_x = DG_MAX( flr->p1.x,   flr->p2.x );
			bound_max_x = DG_MAX( bound_max_x, flr->p3.x );
			bound_max_x = DG_MAX( bound_max_x, flr->p4.x );
			bound_max_z = DG_MAX( flr->p1.z,   flr->p2.z );
			bound_max_z = DG_MAX( bound_max_z, flr->p3.z );
			bound_max_z = DG_MAX( bound_max_z, flr->p4.z );

			// 階段等の狭い場所では狭い幅の真ん中に置く
			ftemp  = bound_max_x - bound_min_x;
			ftemp2 = bound_max_z - bound_min_z;
			if( DG_MIN( ftemp, ftemp2 ) < NARR_LIMIT ){
#if 0
				if( ftemp > ftemp2 ){
					work->pos.vz = ( bound_max_z + bound_min_z ) * 0.5f;
					if( spread_size > ftemp2*0.25f ) spread_size = ftemp2*0.25f;
				}else{
					work->pos.vx = ( bound_max_x + bound_min_x ) * 0.5f;
					if( spread_size > ftemp*0.25f )  spread_size = ftemp *0.25f;
				}
#else
				work->floor_flag = 0;
#endif
			}else{
				// はみ出しチェック
				if( work->pos.vx > bound_max_x - spread_size ){
					work->pos.vx = bound_max_x - spread_size;
				}else if( work->pos.vx < bound_min_x + spread_size ){
					work->pos.vx = bound_min_x + spread_size;
				}
				if( work->pos.vz > bound_max_z - spread_size ){
					work->pos.vz = bound_max_z - spread_size;
				}else if( work->pos.vz < bound_min_z + spread_size ){
					work->pos.vz = bound_min_z + spread_size;
				}
			}

		}
	}


//printf("%f\n",work->under_limit);

	work->decay=decay;
	work->life = DIRECT_TICK( LIFE_LIMIT );
	if( work->life <= 0 ) work->life = 1;


//printf("HZX_FLOOR_NO_BLOOD::%d\n",flr_atrs[ 0 ] & HZX_FLOOR_NO_BLOOD);
	if( work->floor_flag ){
		if( !( flr_atrs[ 0 ] & HZX_FLOOR_NO_BLOOD ) ){
			work->pos.vy = work->under_limit;
//			BloodSpread( &work->pos, &work->floor_rot, work->decay, work->spread_size, work->addr );
			BloodSpread( &work->pos, &work->floor_rot, 3000, work->spread_size, work->addr );
		}
	}
	return -1;


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



	return 0 ;
}

/*

pos   : 絶対座標で与える。
decay : 床に当たると広がる血を発生し、これが消滅するまでのフレーム数

*/
//    int addr;		/* コントロール構造体のグループ内アドレス */
void *NewBloodDrops( FVECTOR *pos, int decay, int white, int addr )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, pos, decay, white, 128.0f, addr ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

/*

pos         : 絶対座標で与える。
decay       : 床に当たると広がる血を発生し、これが消滅するまでのフレーム数
white       : ０）赤 １）未対応
spread_size : 床に到達した後に広がる血の大きさ

*/
//    int addr;		/* コントロール構造体のグループ内アドレス */
void *NewBloodDropsSize( FVECTOR *pos, int decay, int white, int spread_size, int addr )
{
	Work		*work ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, pos, decay, white, spread_size, addr ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
