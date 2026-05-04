/*********************************************

	エフェクト　効果 亀甲エフェクト　のランチャー関数

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
	FMATRIX *root ;
	FVECTOR *RegionCenter ;
	int     color ;
	float   HexRad ;
	float   RegionRad ;
	float   floor_y ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
	FVECTOR d4 ;
	IVECTOR d5 ;
	float   d6 ;
	float   d7 ;
	float   d8 ;
} Data ;

void *NewHexagonalPattern_601fLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewHexagonalPattern( int ,FMATRIX *,FVECTOR *,int ,float ,float ,float  ) ;

	IVECTOR * link ;
	void    * handle ;
	IVECTOR * col ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.root = DM_GetObjectMatrix( handle, link->vy ) ;
	work.RegionCenter = &data->d4 ;
	col = &data->d5 ;
	work.color = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;
	work.HexRad = data->d6 ;
	work.RegionRad = data->d7 ;
	work.floor_y = data->d8 ;

	return NewHexagonalPattern( work.name,
	                            work.root,
	                            work.RegionCenter,
	                            work.color,
	                            work.HexRad,
	                            work.RegionRad,
	                            work.floor_y
) ;
}


/*********************************************

	エフェクト　効果 亀甲エフェクト　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
