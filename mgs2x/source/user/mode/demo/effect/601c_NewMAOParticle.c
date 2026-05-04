//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　効果 汎用パーティクル　のランチャー関数

	Created by DEMO Composer.
 *********************************************/


#include <stdio.h>
#include <stdlib.h>
#ifdef PSX2
#include <libgraph.h>
#include <libdma.h>
#include <libvu0.h>
#endif

#include "gameheader.h"
#include "libutl.h"
#include "../libdemo.h"



typedef struct work_t {
	int     nRandSeed ;
	FMATRIX *world ;
	int     nRrimNum ;
	int     nVertNum ;
	int     nLifeMin ;
	int     nLifeMax ;
	int     nAng ;
	float   fInitSpd ;
	float   fAimSpd ;
	float   fGravity ;
	float   fInitSize ;
	float   fAimSize ;
	int     tex_code ;
	int     nInitRGBA ;
	int     nAimRGBA ;
	float   fRandRate ;
	int     nFlag ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
	int     d4 ;
	int     d5 ;
	int     d6 ;
	int     d7 ;
	int     d8 ;
	float   d9 ;
	float   d10 ;
	float   d11 ;
	float   d12 ;
	float   d13 ;
	int     d14 ;
	IVECTOR d15 ;
	IVECTOR d16 ;
	float   d17 ;
	int     d18 ;
} Data ;

void *NewMAOParticle_601cLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewMAOParticle( int ,FMATRIX *,int ,int ,int ,int ,int ,float ,float ,float ,float ,float ,int ,int ,int ,float ,int  ) ;

	IVECTOR * link ;
	void    * handle ;
	IVECTOR * col ;

	work.nRandSeed = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.world = DM_GetObjectMatrix( handle, link->vy ) ;
	work.nRrimNum = data->d4 ;
	work.nVertNum = data->d5 ;
	work.nLifeMin = data->d6 ;
	work.nLifeMax = data->d7 ;
	work.nAng = data->d8 ;
	work.fInitSpd = data->d9 ;
	work.fAimSpd = data->d10 ;
	work.fGravity = data->d11 ;
	work.fInitSize = data->d12 ;
	work.fAimSize = data->d13 ;
	work.tex_code = data->d14 ;
	col = &data->d15 ;
	work.nInitRGBA = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;
	col = &data->d16 ;
	work.nAimRGBA = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;
	work.fRandRate = data->d17 ;
	work.nFlag = data->d18 ;

	return NewMAOParticle( work.nRandSeed,
	                       work.world,
	                       work.nRrimNum,
	                       work.nVertNum,
	                       work.nLifeMin,
	                       work.nLifeMax,
	                       work.nAng,
	                       work.fInitSpd,
	                       work.fAimSpd,
	                       work.fGravity,
	                       work.fInitSize,
	                       work.fAimSize,
	                       work.tex_code,
	                       work.nInitRGBA,
	                       work.nAimRGBA,
	                       work.fRandRate,
	                       work.nFlag
) ;
}


/*********************************************

	エフェクト　効果 汎用パーティクル　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
