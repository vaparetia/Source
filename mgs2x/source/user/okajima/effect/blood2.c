//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	blood2.c
	血飛沫（激しい出血）
	1999/09/01 S.Okajima
	$Id: blood2.c,v 1.1.1.3 2002/11/19 11:46:57 Yoshizawa1 Exp $
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

extern void	*OK_BLOOD_WALL_WORK;
extern void OK_DirVecXY( FVECTOR *from, FVECTOR *to, SVECTOR *rot );
extern void PutCameraBlood( FVECTOR *pos_pers, int white );

/*----------------------------------------------------------------*/
#define	MAX_TEX		( 5 )

#define	SCR_LENGTH		( 0x4000 )
#define	FVEC_NUM		(SCR_LENGTH/16)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + 0x1000)
#define	SCR_VEC		(SCRPAD_ADDR + 0x3000)

#define	N_VERTS		(32)
#define	N_PRIMS		(1)

#define	SCALE			(80.0f)
#define	LIFE_TIME		(N_PRIMS * N_VERTS )
//#define	GRAVITY			(4.0f)

#ifdef ENGLISH
#define	MAX_ALPHA		(128)
#else
#define	MAX_ALPHA		(BLOOD_ALPHA_0)
#endif

#define	PRIM_SIZE		(30.0f)
#define	PRIM_SIZE_MIN	(50.0f)

#define	RAND_ROT		(128)

typedef	struct	{
	DG_PRIM2	*prim ;
	FVECTOR		vec[N_PRIMS * N_VERTS] ;
	float		alpha[N_PRIMS * N_VERTS] ;
} Unit ;

typedef	struct	{
	GV_ACT_EX		actor ;
	int			map ;

	int			white;
	FMATRIX		*world;		/* 被付随行列 */
	FVECTOR		center;		/* インバース後 */
	FVECTOR		force;		/* インバース後 */

	int			life;

	Unit		unit[MAX_TEX];
} Work ;

Work	*BLOOD2_WORK = NULL;

/* ---------------------------------------------------------------- */
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	FVECTOR				*pos;
	DG_PRIM2_UVRGBWH	*uvrgbwh ;	/* スプライト用 */
	int		k ;

	DG_ConfigPrim2Tex( prim, tex );
	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
	{
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}
	else
	{
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	}

	//-------------------------------
	pos     = SCR_POS ;
	uvrgbwh = SCR_UVS ;
	for ( k = 0 ; k < N_PRIMS * N_VERTS ; k++ ){
		pos->vx = 0.0f;
		pos->vy = 0.0f;
		pos->vz = 0.0f;

		uvrgbwh->u0 = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;/* 左上 */
		uvrgbwh->v0 = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;/* 左上 */
		uvrgbwh->u1 = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;/* 右下 */
		uvrgbwh->v1 = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;/* 右下 */
		uvrgbwh->q0 = 4096 ;
		uvrgbwh->q1 = 4096 ;
		uvrgbwh->f0 = 0x0fff ;
		uvrgbwh->f1 = 0x0fff ;

		uvrgbwh->h = uvrgbwh->w = PRIM_SIZE_MIN + PRIM_SIZE*rnd();

		uvrgbwh->r = COLOR_R ;
		uvrgbwh->g = COLOR_G ;
		uvrgbwh->b = COLOR_B ;
		uvrgbwh->a = 0 ;

		pos++;
		uvrgbwh ++ ;
	}

	OK_Scr_Mem( prim->pos[   0 ], SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->pos[   1 ], SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS);
}

/* ポリゴンの座標データを更新する */
static	void	UpdateVectors( Work *work, FVECTOR *pos, DG_PRIM2_UVRGBWH *uvrgbwh, FVECTOR *vec )
{
	int j;
	int	alpha;

	for( j=0; j<N_PRIMS * N_VERTS; j++ ){
		_sceVu0AddVector( pos, pos, vec );
		vec->vy += P_GRAVITY;
		alpha = uvrgbwh->a - (MAX_ALPHA / LIFE_TIME);
		if(alpha < 0) alpha=0;
		uvrgbwh->a = alpha;

		uvrgbwh++;
		pos++;
		vec++;
	}

}

/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	int	i;
	DG_PRIM2		*prim ;
	int				clock;
	Unit			*unit;
	DG_PRIM2_UVRGBWH *uvrgbwh;
	FVECTOR			fvtemp0;
	FVECTOR			center;
	FVECTOR			force;
	SVECTOR			rot;

   //AR_PARTICLE_HALF
   if(!AS_WillPrimBuffSwitch())
   {
      return;
   }
	clock = 1 - work->unit[0].prim->buffer_clock;

	fvtemp0.vx = 0.0f;
	fvtemp0.vy = 0.0f;
	if( work->life < LIFE_TIME ){
		/* 関節位置追従 */
		DG_SetPos( work->world );
		DG_PutVector( &work->center, &center, 1 );
		DG_RotVector( &work->force, &force, 1 );
		OK_DirVecXY( &DG_ZeroVector, &force, &rot );

		unit = work->unit;
		for( i=0; i<MAX_TEX; i++ ){
			fvtemp0.vz = SCALE * rnd() * (float)(work->life) / (float)(LIFE_TIME) + SCALE * 0.1f;
			DG_SetPos2( &center, &rot );
			/* 使用するのは前のフレームなので、このデータを書き換える */
			DG_PutVector( &fvtemp0, &unit->prim->pos[1-clock][work->life], 1 );
			DG_RotVector( &fvtemp0, &unit->vec[work->life], 1 );
			uvrgbwh    = unit->prim->uvrgb[1-clock];
			uvrgbwh   += work->life;
			uvrgbwh->a = MAX_ALPHA;
			unit++;
		}

		/* カメラに飛ばす血（起動チェック有り） */
		if( ((irnd()>>8)&15)==0 && GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE) ){
			if( OK_RectLen( &center, (FVECTOR *)DG_Chanls->eye.m[3] ) < 1500.0f ){
				fvtemp0.vx = rnd()*2.0f-1.0f;
				fvtemp0.vy = rnd()*2.0f-1.0f;
				PutCameraBlood( &fvtemp0, work->white );
			}
		}

	}

	unit = work->unit;
	for( i=0; i<MAX_TEX; i++ ){
		prim = unit->prim;
		GM_GroupPrim2( prim, GM_CurrentStageMap ) ;
		DG_SwitchBuffPrim2( prim );
		OK_Mem_Scr( SCR_POS, prim->pos[  1-clock], sizeof(FVECTOR),          N_PRIMS * N_VERTS);
		OK_Mem_Scr( SCR_UVS, prim->uvrgb[1-clock], sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS);
		OK_Mem_Scr( SCR_VEC, unit->vec,            sizeof(FVECTOR),          N_PRIMS * N_VERTS);

		UpdateVectors( work, SCR_POS, SCR_UVS, SCR_VEC );

		OK_Scr_Mem( prim->pos[   clock ], SCR_POS, sizeof(FVECTOR),          N_PRIMS * N_VERTS);
		OK_Scr_Mem( prim->uvrgb[ clock ], SCR_UVS, sizeof(DG_PRIM2_UVRGBWH), N_PRIMS * N_VERTS);
		OK_Scr_Mem( unit->vec,            SCR_VEC, sizeof(FVECTOR),          N_PRIMS * N_VERTS);
		unit++;
	}

	if( work->life >= LIFE_TIME * 2) GV_DestroyActor( work ) ;
	work->life++;
}

static void Die( Work *work )
{
	int	k;
	for( k=0; k<MAX_TEX; k++ ){
		work->unit[k].prim = OK_FreePrim2( work->unit[k].prim );
	}
}

/*----------------------------------------------------------------*/
static int GetResources( Work *work, FMATRIX *world, FVECTOR *pos, FVECTOR *force, int white )
{
	int	k;
	FMATRIX		mat;
	DG_PRIM2	*prim ;
	DG_TEX		*tex[5] ;

	work->white=white;
	work->life = 0;
	work->world = world;

	FastInverseMatrix( &mat, world );

	DG_SetPos( &mat );
	DG_PutVector( pos,   &work->center, 1 );
	DG_RotVector( force, &work->force, 1 );

#if 0
	tex[0] = DG_GetTexture( 10984814 /*"chi01_msk"*/ );
	tex[1] = DG_GetTexture( 12033390 /*"chi02_msk"*/ );
	tex[2] = DG_GetTexture( 13081966 /*"chi03_msk"*/ );
	tex[3] = DG_GetTexture( 14130542 /*"chi04_msk"*/ );
	tex[4] = DG_GetTexture( 15179118 /*"chi05_msk"*/ );
#else
	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
	{
	tex[0] = DG_GetTexture(  9373287 /*"vr2_chi01_alp"*/ );
	tex[1] = DG_GetTexture( 10421863 /*"vr2_chi02_alp"*/ );
	tex[2] = DG_GetTexture( 11470439 /*"vr2_chi03_alp"*/ );
	tex[3] = DG_GetTexture( 12519015 /*"vr2_chi04_alp"*/ );
	tex[4] = DG_GetTexture( 13567591 /*"vr2_chi05_alp"*/ );
	}
	else
	{
	tex[0] = DG_GetTexture( 10972307 /*"chi01_alp"*/ );
	tex[1] = DG_GetTexture( 12020883 /*"chi02_alp"*/ );
	tex[2] = DG_GetTexture( 13069459 /*"chi03_alp"*/ );
	tex[3] = DG_GetTexture( 14118035 /*"chi04_alp"*/ );
	tex[4] = DG_GetTexture( 15166611 /*"chi05_alp"*/ );
	}
#endif

	for( k=0; k<MAX_TEX; k++ ){
		prim = work->unit[k].prim = GM_MakePrim2( DG_PRIM2_SPRT|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
		if(prim==NULL){
			printf("null prim\n");
			return -1;
		}
		InitPacket2( work, prim, tex[k] );
	}

	GM_SeSetMode( SD_E_BLOOD_L1 , pos, GM_SEMODE_NORMAL );	/* 血発生音 */

	return 0 ;
}


/*

[world]:付随させる行列（人型の関節など）
[pos  ]:発生時の絶対座標での初期位置
[force]:吹き出しベクトル
[white]:0:赤 1:白

*/
void *NewBlood2( FMATRIX *world, FVECTOR *pos, FVECTOR *force, int white )
{
	Work		*work ;

	if( GM_Configuration & GM_CONFIG_BLOOD_OFF ) return NULL;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, world, pos, force, white ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

