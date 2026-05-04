/*********************************************

	エフェクト　水飛沫 汎用水飛沫　のランチャー関数

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
	int     name ;
	int     con_name ;
	int     nPrims ;
	int     nAddNum ;
	float   fRadMin ;
	float   fRadMax ;
	int     LifeMin ;
	int     LifeMax ;
	float   fSizeBase ;
	float   fSizeAdd ;
	float   fGravity ;
	int     color ;
	float   fSprdRadBase ;
	float   fSprdRadAdd ;
	float   fHeightBase ;
	float   fHeightAdd ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
	int     d2 ;
	int     d3 ;
	float   d4 ;
	float   d5 ;
	int     d6 ;
	int     d7 ;
	float   d8 ;
	float   d9 ;
	float   d10 ;
	IVECTOR d11 ;
	float   d12 ;
	float   d13 ;
	float   d14 ;
	float   d15 ;
} Data ;

void *NewWaterSpreadDemo_010bLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewWaterSpreadDemo( int ,int ,int ,int ,float ,float ,int ,int ,float ,float ,float ,int ,float ,float ,float ,float  ) ;

	IVECTOR * col ;

	work.name = data->d0 ;
	work.con_name = data->d1 ;
	work.nPrims = data->d2 ;
	work.nAddNum = data->d3 ;
	work.fRadMin = data->d4 ;
	work.fRadMax = data->d5 ;
	work.LifeMin = data->d6 ;
	work.LifeMax = data->d7 ;
	work.fSizeBase = data->d8 ;
	work.fSizeAdd = data->d9 ;
	work.fGravity = data->d10 ;
	col = &data->d11 ;
	work.color = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;
	work.fSprdRadBase = data->d12 ;
	work.fSprdRadAdd = data->d13 ;
	work.fHeightBase = data->d14 ;
	work.fHeightAdd = data->d15 ;

	return NewWaterSpreadDemo( work.name,
	                           work.con_name,
	                           work.nPrims,
	                           work.nAddNum,
	                           work.fRadMin,
	                           work.fRadMax,
	                           work.LifeMin,
	                           work.LifeMax,
	                           work.fSizeBase,
	                           work.fSizeAdd,
	                           work.fGravity,
	                           work.color,
	                           work.fSprdRadBase,
	                           work.fSprdRadAdd,
	                           work.fHeightBase,
	                           work.fHeightAdd
) ;
}


/*********************************************

	エフェクト　水飛沫 汎用水飛沫　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
