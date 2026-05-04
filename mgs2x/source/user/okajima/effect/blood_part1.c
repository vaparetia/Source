//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*
	blood.c
	血飛沫
	1999/09/01 S.Okajima
	$Id: blood_part1.c,v 1.1.1.3 2002/11/19 11:46:58 Yoshizawa1 Exp $
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
extern void PutWallBlood( FVECTOR *pos, SVECTOR *rot, int white );
extern void *NewBlood2( FMATRIX *world, FVECTOR *pos, FVECTOR *force, int white );
extern void *NewBloodPart2( FVECTOR *pos, SVECTOR *rot, int white );
extern void *NewBlood_M9( FMATRIX *world, FVECTOR *mov, FVECTOR *norm );

/*----------------------------------------------------------------*/
//#define	BLOOD_GRAVITY	(5.0f)

#define	BLOOD_LENGTH	(150.0f)
#define	LENGTH_RAND		(BLOOD_LENGTH * 0.9f)
#define	ANGLE_RAND		(256)

#define	WIDTH_RATIO		(0.7f)

#define	SCR_LENGTH		( 0x4000 )
#define	SCR_WALL		( SCR_LENGTH / 3 /16 * 16)

#define	SCR_POS		(SCRPAD_ADDR)
#define	SCR_UVS		(SCRPAD_ADDR + SCR_WALL)

#define	N_VERTS		(64)
#define	N_POLYS		(N_VERTS/4)
#define	N_PRIMS		(SCR_WALL / 16 / N_VERTS)
/* ｖｗ は 速度として使用 */

#ifdef ENGLISH
#define	MAX_ALPHA		(64)
#else
#define	MAX_ALPHA		(BLOOD_ALPHA_0)
#endif

#define	SKIP			( 4 )
#define	LIFE_TIME		( N_PRIMS * SKIP )

typedef	struct	{
	GV_ACT_EX		actor ;

	DG_PRIM2	*prim ;

	FMATRIX		*world;		/* 被付随行列 */
	FVECTOR		force;		/* インバース後 */
	FVECTOR		center;		/* インバース後 */

	int			count;
	int			life;
	int			white;
	int			prim_num;
	int			prim_count[N_PRIMS];
} Work ;


/*----------------------------------------------------------------*/
static void InitPartVector( Work *work, FVECTOR *pos, int part_verts )
{
	FVECTOR			center ;
	FVECTOR			fvtemp0 ;
	FVECTOR			fvtemp1 ;
	SVECTOR			rot ;
	SVECTOR			svtemp ;
	FVECTOR			*pos0 ;
	FVECTOR			*pos1 ;
	FVECTOR			*pos2 ;
	FVECTOR			*pos3 ;
	int		k ;

	/* 付随する関節から出すべき位置・角度 */
	DG_SetPos( work->world );
	DG_RotVector( &work->force, &fvtemp0, 1 );
	DG_PutVector( &work->center, &center, 1 );
//printf("%f %f %f\n",center.vx,center.vy,center.vz);
	OK_DirVecXY( &DG_ZeroVector, &fvtemp0, &rot );

	pos0 = pos1 = pos2 = pos3 = pos ;
	pos1+=1;
	pos2+=2;
	pos3+=3;
	for ( k = 0 ; k < N_POLYS ; k++ ){
		/* 初期頂点 */
		svtemp.vx  = rot.vx + irnd()%ANGLE_RAND - ANGLE_RAND/2;
		svtemp.vy  = rot.vy + irnd()%ANGLE_RAND - ANGLE_RAND/2;
		svtemp.vz  = irnd()%4096;

		fvtemp0.vx = 0.0f;
		fvtemp0.vy = 0.0f;
		fvtemp0.vz =  BLOOD_LENGTH - rnd()*LENGTH_RAND;

		fvtemp1.vx = (BLOOD_LENGTH - rnd()*LENGTH_RAND)*WIDTH_RATIO;
		fvtemp1.vy = (BLOOD_LENGTH - rnd()*LENGTH_RAND)*WIDTH_RATIO;
		fvtemp1.vz = 0.0f;

		DG_SetPos2( &DG_ZeroVector, &svtemp );
		DG_RotVector( &fvtemp0, &fvtemp0, 1 );	/* 主軸 */
		DG_RotVector( &fvtemp1, &fvtemp1, 1 );	/* 幅 */

		_sceVu0CopyVector( pos0, &center ) ;
		_sceVu0CopyVector( pos1, pos0 ) ;
		_sceVu0CopyVector( pos2, pos0 ) ;
		_sceVu0CopyVector( pos3, pos0 ) ;

		/* 速度を保持 */
		pos0->vw = fvtemp0.vx;
		pos1->vw = fvtemp0.vy;
		pos2->vw = fvtemp0.vz;

		/* 遠方の頂点二つ */
		pos0->vx += fvtemp0.vx + fvtemp1.vx;
		pos0->vy += fvtemp0.vy + fvtemp1.vy;
		pos0->vz += fvtemp0.vz + fvtemp1.vz;

		pos1->vx += fvtemp0.vx - fvtemp1.vx;
		pos1->vy += fvtemp0.vy - fvtemp1.vy;
		pos1->vz += fvtemp0.vz - fvtemp1.vz;

		/* 近傍の頂点二つ */
		fvtemp0.vx /= 0.2f;
		fvtemp0.vy /= 0.2f;
		fvtemp0.vz /= 0.2f;
		pos2->vx += fvtemp0.vx + fvtemp1.vx;
		pos2->vy += fvtemp0.vy + fvtemp1.vy;
		pos2->vz += fvtemp0.vz + fvtemp1.vz;

		pos3->vx += fvtemp0.vx - fvtemp1.vx;
		pos3->vy += fvtemp0.vy - fvtemp1.vy;
		pos3->vz += fvtemp0.vz - fvtemp1.vz;

		pos0 += 4;
		pos1 += 4;
		pos2 += 4;
		pos3 += 4;

	}
}



/*----------------------------------------------------------------*/
static	void	Act( Work *work )
{
	FVECTOR			fvtemp0 ;
	FVECTOR			fvtemp1 ;
	FVECTOR	*pos0;
	FVECTOR	*pos1;
	FVECTOR	*pos2;
	FVECTOR	*pos3;
	SVECTOR	rot;
	DG_PRIM2_UVRGB	*uvrgb ;
	int	i,j;
	int	clock;
	int	*prim_count;
	int	alpha;

	GM_GroupPrim2( work->prim, GM_CurrentStageMap ) ;
   //AR_PARTICLE_HALF
   if( !DG_SwitchBuffPrim2( work->prim ) )
   {
      return;
   }
	clock = work->prim->buffer_clock;

	alpha = MAX_ALPHA * work->life / LIFE_TIME;
	if(alpha<0)alpha=0;

	//--------------------------------------------------------------------------
	// 標準の血
	OK_Mem_Scr( SCR_POS, work->prim->pos[   1-clock ], sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Mem_Scr( SCR_UVS, work->prim->uvrgb[ 1-clock ], sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);

	pos0 = pos1 = pos2 = pos3 = SCR_POS;
	pos1 += 1;
	pos2 += 2;
	pos3 += 3;
	uvrgb = SCR_UVS;
	prim_count = work->prim_count;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		if( (*prim_count) >= 0 ){
			for ( j = 0 ; j < N_POLYS ; j++ ){
				pos0->vx += pos0->vw;
				pos0->vy += pos1->vw;
				pos0->vz += pos2->vw;
				pos1->vx += pos0->vw;
				pos1->vy += pos1->vw;
				pos1->vz += pos2->vw;
				pos2->vx += pos0->vw;
				pos2->vy += pos1->vw;
				pos2->vz += pos2->vw;
				pos3->vx += pos0->vw;
				pos3->vy += pos1->vw;
				pos3->vz += pos2->vw;
				pos1->vw += P_GRAVITY;
				pos0+=4;
				pos1+=4;
				pos2+=4;
				pos3+=4;

				uvrgb->a = alpha;	uvrgb++;
				uvrgb->a = alpha;	uvrgb++;
				uvrgb->a = alpha;	uvrgb++;
				uvrgb->a = alpha;	uvrgb++;
			}
		}else{
			if( i == work->count/SKIP*SKIP ){
				*prim_count =0;
				InitPartVector( work, pos0, i*N_VERTS );
			}
			pos0  += N_VERTS;
			pos1  += N_VERTS;
			pos2  += N_VERTS;
			pos3  += N_VERTS;
			uvrgb += N_VERTS;
		}
		prim_count++;
	}

	OK_Scr_Mem( work->prim->pos[   clock ], SCR_POS, sizeof(FVECTOR),        N_PRIMS * N_VERTS);
	OK_Scr_Mem( work->prim->uvrgb[ clock ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);


	if( work->life % (LIFE_TIME/2)==0 ){
		/* 壁血（内部でスクラッチパッド使用しているのでループの外） */
		DG_SetPos( work->world );
		DG_RotVector( &work->force, &fvtemp0, 1 );
		DG_PutVector( &work->center, &fvtemp1, 1 );
		OK_DirVecXY( &DG_ZeroVector, &fvtemp0, &rot );

		rot.vx += irnd()%1024;
		rot.vy += irnd()%256-128;
		rot.vz += irnd()%256-128;
		PutWallBlood( &fvtemp1, &rot, work->white );
	}

	if( work->life < 0 ) GV_DestroyActor( work ) ;
	work->life--;
	work->count++;

}

static void Die( Work *work )
{
//printf("end\n");
	work->prim = OK_FreePrim2( work->prim );
}

/*----------------------------------------------------------------*/
static void InitPacket2( Work *work, DG_PRIM2 *prim, DG_TEX *tex )
{
	DG_PRIM2_UVRGB	*uvrgb ;
	int		i, k ;

	DG_ConfigPrim2Tex( prim, tex );
	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/03 */
	{
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 2, 0, 1, 0x00 ) );
	}
	else
	{
	DG_SetPrim2Alpha( prim, SCE_GS_SET_ALPHA( 0, 1, 0, 1, 0x00 ) );
	}

	/* 固定値と非表示（α＝０）セット */
	uvrgb = SCR_UVS ;
	for ( i = 0 ; i < N_PRIMS ; i++ ){
		for ( k = 0 ; k < N_POLYS ; k++ ){
			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = COLOR_R ;
			uvrgb->g = COLOR_G ;
			uvrgb->b = COLOR_B ;
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 0.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x8fff ;
			uvrgb->r = COLOR_R ;
			uvrgb->g = COLOR_G ;
			uvrgb->b = COLOR_B ;
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 0.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = COLOR_R ;
			uvrgb->g = COLOR_G ;
			uvrgb->b = COLOR_B ;
			uvrgb->a = 0 ;
			uvrgb++;

			uvrgb->u = FTOI12( 1.0F * tex->u_scale + tex->u_offset ) ;
			uvrgb->v = FTOI12( 1.0F * tex->v_scale + tex->v_offset ) ;
			uvrgb->q = 4096 ;
			uvrgb->f = 0x0fff ;
			uvrgb->r = COLOR_R ;
			uvrgb->g = COLOR_G ;
			uvrgb->b = COLOR_B ;
			uvrgb->a = 0 ;
			uvrgb++;

			if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/04/02 */
			{
				(uvrgb - 4)->r = (uvrgb - 3)->r = (uvrgb - 2)->r = (uvrgb - 1)->r = 180;
				(uvrgb - 4)->g = (uvrgb - 3)->g = (uvrgb - 2)->g = (uvrgb - 1)->g =
				(uvrgb - 4)->b = (uvrgb - 3)->b = (uvrgb - 2)->b = (uvrgb - 1)->b = 0;
			}
		}
	}

	OK_Scr_Mem( prim->uvrgb[ 0 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);
	OK_Scr_Mem( prim->uvrgb[ 1 ], SCR_UVS, sizeof(DG_PRIM2_UVRGB), N_PRIMS * N_VERTS);

	GV_ZeroMemory( prim->pos[0], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );
	GV_ZeroMemory( prim->pos[1], sizeof( FVECTOR ) * N_PRIMS * N_VERTS );

}
/*----------------------------------------------------------------*/
static int GetResources( Work *work, FMATRIX *world, FVECTOR *pos, FVECTOR *force, int mode, int white )
{
	int	i;
	float	len;
	FMATRIX		world_inv;
	FVECTOR		fvtemp;
	FVECTOR		pos_pers;
	SVECTOR		rot;
	DG_PRIM2	*prim ;
	DG_TEX		*tex ;


//AN_Test_Eye2( pos, 2 );
//printf("%f %f %f\n",pos->vx,pos->vy,pos->vz);


	work->world = world;

	FastInverseMatrix( &world_inv, world );

	DG_SetPos( &world_inv );
	DG_RotVector( force, &work->force, 1 );
	DG_PutVector( pos,   &work->center, 1 );

	work->white=white;

	for( i=0; i<N_PRIMS; i++ ){
		work->prim_count[i]=-100;
	}

//	tex = DG_GetTexture( 13872561 /*"blood_1e_msk"*/ );
//	tex = DG_GetTexture( 12823985 /*"blood_1d_msk"*/ );

//	tex = DG_GetTexture( 11775409 /*"blood_1c_msk"*/ );
	tex = DG_GetTexture( 15638432 /*"blood_1bw_msk"*/ );

	if(GM_VRStatus & GM_VR_EFFECT)	/* ＶＲエフェクト modified by S.Yamashita 2002/06/25 */
	{
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA, N_PRIMS, N_VERTS );
	}
	else
	{
	prim = work->prim = GM_MakePrim2( DG_PRIM2_POLY|DG_PRIM2_SHADE|DG_PRIM2_TEX|DG_PRIM2_ALPHA|DG_PRIM2_FOG, N_PRIMS, N_VERTS );
	}
	if(prim==NULL){
		printf("null prim\n");
		return -1;
	}
	InitPacket2( work, prim, tex );


	work->life = LIFE_TIME;
	work->count=0;

	_sceVu0SubVector( &fvtemp, pos, (FVECTOR *)DG_Chanls->eye.m[3] ) ;
	len = GV_VecLen3F( &fvtemp );

//printf("len:::%f\n",len);

	if(mode!=0){
		/* カメラに飛ばす血（起動チェック有り） */
		if( GM_CheckPlayerStatus( PLAYER_WATCH | PLAYER_INTRUDE) ){
			if( len < 1500.0f ){
				for( i=0; i<4; i++ ){
					pos_pers.vx = frnd();
					pos_pers.vy = frnd();
					PutCameraBlood( &pos_pers, white );
				}
			}
			OK_DirVecXY( &DG_ZeroVector, (FVECTOR *)DG_Chanls->eye.m[3], &rot );
			NewBloodPart2( pos, &rot, white );
		}

		NewBlood2( world, pos, force, white );
		NewBlood_M9( world, pos, force );

	}

	if( len > 500.0f ){
		extern void  *NewBlood_2D( FVECTOR *pos );
		NewBlood_2D(pos);
	}

	return 0 ;
}

/*

[world]:付随させる行列（人型の関節など）
[pos  ]:発生時の絶対座標での初期位置
[force]:吹き出しベクトル
[mode ]:０：通常、 １：大量出血
[white]:0:赤 1:白

*/
void *NewBlood( FMATRIX *world, FVECTOR *pos, FVECTOR *force, int mode, int white )
{
	Work		*work ;

	if( GM_Configuration & GM_CONFIG_BLOOD_OFF ) return NULL;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )
		if ( GetResources( work, world, pos, force, mode, white ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}
