//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*

	bomb_in_water.c
	水中爆炎エフェクト
	2001/05/01 S.Okajima
	$Id: bomb_in_water.c,v 1.1.1.3 2002/11/19 11:47:28 Yoshizawa1 Exp $

*/


#ifdef PSX2
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <eekernel.h>
#include <eeregs.h>
#include <libgraph.h>
#include <libdma.h>
#include <libdev.h>
#include <sifdev.h>
#include <libvu0.h>
#endif

#include <libutl.h>
#include	"libdg.h"
#include	"libdg.cnf"
#include	"libmt.h"
#include	"gameheader.h"
#include	"utl_dma.h"
#include	"../etc/ok_util.h"

#define	FOG_R_MAX	(255)
#define	FOG_G_MAX	(255)
#define	FOG_B_MAX	(255)
#define	FOG_LIFE0	(2)
#define	FOG_LIFE1	(8)
#define	FOG_LIFE2	(16)

#define	LIFE		(8)


#define	ROTATE_ANGLE_RND	(512-1)
#define	DECLAINE_ANGLE		(64-1)

#define	LIMIT_WATER_MOUNTAIN	(3000.0f)

/*----------------------------------------------------------------*/
extern float GM_WaterLevel;

extern CVECTOR OK_FogColorBase;
extern float   OK_FogNear;
extern float   OK_FogFar;

extern void *NewFogSet_Demo( int col_r, int col_g, int col_b, float near, float far, int time );
extern void *NewBombSphere( FMATRIX *world, float radius );
extern void *NewSmokeInWater( FVECTOR *pos, float size, int life );
extern void *NewBombBubble( FVECTOR *center, float radius, int life );
extern int	OK_FogDataInsert( CVECTOR col, float near, float far, int phase0, int phase1, int phase2 );
extern void *NewWaterSurfaceMountain( FVECTOR *center, float radius, float intense, int flag );
extern void *NewWaterSurfaceMountain2( FVECTOR *center, float radius, float intense, int flag );
// 固定生成波紋

extern void* NewOneRipple( FVECTOR*	pvecPos,	// 中心位置
		    float	fRadius,	// 半径
		    int		nTime,		// 時間
		    int		nAddNum,	// 同時生成数	
		    int		nIntvMin,	// 波紋発生間隔(最小)	
		    int		nIntvMax,	// 波紋発生間隔(最大)	
		    float	fSizeMin,	// 波紋サイズ(最小)	
		    float	fSizeMax);	// 波紋サイズ(最大)	
// 水飛抹

extern void* NewOneSpread( FMATRIX*	pmatOrg, 		// 出現座標系
		    int		nPrims, 		// 出すプリミティブ数 ( 1prim == 16飛抹)
		    int		nAddNum, 		// 1フレームで生成するプリミティブ数
		    float	fRadMin, 		// 発生半径（最小）
		    float	fRadMax, 		// 発生半径（最大）
		    int		nLifeMin, 		// 生存時間（最小）
		    int		nLifeMax, 		// 生存時間（最大）
		    float	fSizeBase, 		// 基本サイズ
		    float	fSizeAdd, 		// ランダム加算サイズ
		    float	fGravity, 		// 重力 1.f で -P_GRAVITY
		    int		nRGBA,	 		// RGBA値
		    float	fSpreadRadBase,		// 移動ベクトルXZ成分:基本	
		    float	fSpreadRadAdd,		// 移動ベクトルXZ成分:ランダム幅
		    float	fSpreadHeightBase,	// 移動ベクトルY成分:基本
		    float	fSpreadHeightAdd);      	// 移動ベクトルY成分:ランダム幅

/* ---------------------------------------------------------------- */
typedef	struct	{
	GV_ACT_EX	actor ;

	int			life;

	FVECTOR		center;
} Work ;

/* ---------------------------------------------------------------- */
static void CalcSets( Work *work, float radius )
{
	FMATRIX	mat0;
	FMATRIX	mat1;
	SVECTOR	dir_cam;
	SVECTOR	svtemp;

	OK_DirVecXY( &work->center, (FVECTOR *)DG_Chanls->eye.m[3], &dir_cam );
	svtemp.vy = 0;
	svtemp.vz = 0;

	dir_cam.vz = ((irnd()>>8)&4095);
	DG_SetPos2( &work->center, &dir_cam );
	DG_GetPos( &mat0 );
	svtemp.vx = ((irnd()>>8)&DECLAINE_ANGLE);
	DG_SetPos2( &DG_ZeroVector, &svtemp );
	DG_GetPos( &mat1 );
	_sceVu0MulMatrix( &mat1, &mat0, &mat1 );
	NewBombSphere( &mat1, radius*(0.75f + rnd()*0.25f) );


	dir_cam.vz+= 1500 + ((irnd()>>8)&ROTATE_ANGLE_RND);
	DG_SetPos2( &work->center, &dir_cam );
	DG_GetPos( &mat0 );
	svtemp.vx = ((irnd()>>8)&DECLAINE_ANGLE);
	DG_SetPos2( &DG_ZeroVector, &svtemp );
	DG_GetPos( &mat1 );
	_sceVu0MulMatrix( &mat1, &mat0, &mat1 );
	NewBombSphere( &mat1, radius*(0.75f + rnd()*0.25f) );

	dir_cam.vz+= 2300 + ((irnd()>>8)&ROTATE_ANGLE_RND);
	DG_SetPos2( &work->center, &dir_cam );
	DG_GetPos( &mat0 );
	svtemp.vx = ((irnd()>>8)&DECLAINE_ANGLE);
	DG_SetPos2( &DG_ZeroVector, &svtemp );
	DG_GetPos( &mat1 );
	_sceVu0MulMatrix( &mat1, &mat0, &mat1 );
	NewBombSphere( &mat1, radius*(0.75f + rnd()*0.25f) );

}

/* ---------------------------------------------------------------- */
static void Act( Work *work )
{
	FVECTOR	fvtemp1;
	float	flr_height[2];
	float	ftemp1;

	if( work->life == LIFE   ) CalcSets( work, 1700.0f );
	if( work->life == LIFE   ) NewSmokeInWater( &work->center, 800.0f, 300 );
	if( work->life == LIFE   ){
		NewOneRipple( &work->center,  500.f, 60, 2, 4, 8,  800.f, 1000.f);
		NewOneRipple( &work->center, 1000.f, 60, 1, 1, 2,  800.f, 1000.f);
	}
	if( work->life == LIFE-1 ){
		float ftemp;
		ftemp = work->center.vy - (GM_WaterLevel - LIMIT_WATER_MOUNTAIN);
		if( ftemp > 0.0f ){
			ftemp = (LIMIT_WATER_MOUNTAIN-ftemp)*0.5f;
			if( !GM_CheckGameStatus( STATE_DEMO ) ){
				fvtemp1.vx = work->center.vx;
				fvtemp1.vy = GM_WaterLevel;
				fvtemp1.vz = work->center.vz;
				if( HZX_LevelHazardCheck(
						GM_GetHzxGroupID( GM_CurrentStageMap ),
						&fvtemp1,
						HZX_CHK_ALL,
						0 )
					 & 2 ){
					HZX_GetLevelHeight( flr_height );
					ftemp1 = (flr_height[1]-GM_WaterLevel)*0.5f;
					if( ftemp > ftemp1 ) ftemp = ftemp1;
				}
			}
//			NewWaterSurfaceMountain( &work->center, 500.0f, ftemp, 0x03 );
			NewWaterSurfaceMountain2( &work->center, 500.0f, ftemp, 0x03 );
#ifdef PSX2	///
			GM_SeSetMode( SD_A_INWBOMS1, &work->center, GM_SEMODE_BOMB );
#endif
		}
	}
	if( work->life == LIFE-2 ) NewBombBubble( &work->center, 1500.0f, 100 );
	if( work->life == LIFE-4 ) NewBombBubble( &work->center, 1000.0f, 250 );
#if 0
	if( work->life == LIFE-6 ){
		FMATRIX	fmat;
		DG_COPY_MAT( &fmat, &DG_UnitMatrix );
		DG_COPY_VEC( (FVECTOR *)fmat.m[3], &work->center );
		NewOneSpread( &fmat, 16, 32, 200.f, 300.f, 40, 80, 100.f, 200.f, 1.0f/8.0f, 0x808080ff, 4.f, 8.f, 5.f, 10.f);
	}
#endif

	if( work->life-- <= 0) GV_DestroyActor( work ) ;
}

/* ---------------------------------------------------------------- */
static void Die( Work *work )
{
}

static int GetResources( Work *work )
{
	FVECTOR	fvtemp;
//	CVECTOR	col;
	float	len;

	work->life = LIFE;


	_sceVu0SubVector( &fvtemp, &work->center, (FVECTOR *)DG_Chanls->eye.m[3] );
	len = GV_VecLen3F( &fvtemp );

#if 0
	col.r = FOG_R_MAX;
	col.g = FOG_G_MAX;
	col.b = FOG_B_MAX;
	OK_FogDataInsert( col, 0.0f, len + 1000.0f, FOG_LIFE0, FOG_LIFE1, FOG_LIFE2 );
#endif

	return 0 ;
}

void *NewBombWaterEffect( FVECTOR *center )
{
	Work		*work ;

	OPERATOR() ;

	work = (Work *)GV_NewEffect( GV_ACTOR_USER, sizeof( Work ) ) ;
	if ( work != NULL ) {
		GV_SetActor( &( work->actor ), Act, Die ) ;
		GV_ActorEX( &work->actor )

		DG_COPY_VEC( &work->center, center );

		if ( GetResources( work ) < 0 ) {
			GV_DestroyActor( work ) ;
			return NULL ;
		}
	}
	return (void *)work ;
}

