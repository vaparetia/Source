//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　水飛沫 キャラ付随　のランチャー関数

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
	int     nPrims ;
	int     nVerts ;
	float   fSizeBase ;
	float   fSizeAdd ;
	FMATRIX *matRefer ;
	int     colAdd ;
	int     colSub ;
	int     LifeMin ;
	int     LifeMax ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
	int     d2 ;
	float   d3 ;
	float   d4 ;
	IVECTOR d5 ;
	IVECTOR d8 ;
	IVECTOR d9 ;
	int     d10 ;
	int     d11 ;
} Data ;

void *NewConcSpread_010aLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewConcSpread( int ,int ,int ,float ,float ,FMATRIX *,int ,int ,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;
	IVECTOR * col ;

	work.name = data->d0 ;
	work.nPrims = data->d1 ;
	work.nVerts = data->d2 ;
	work.fSizeBase = data->d3 ;
	work.fSizeAdd = data->d4 ;
	link = &data->d5 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.matRefer = DM_GetObjectMatrix( handle, link->vy ) ;
	col = &data->d8 ;
	work.colAdd = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;
	col = &data->d9 ;
	work.colSub = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;
	work.LifeMin = data->d10 ;
	work.LifeMax = data->d11 ;

	return NewConcSpread( work.name,
	                      work.nPrims,
	                      work.nVerts,
	                      work.fSizeBase,
	                      work.fSizeAdd,
	                      work.matRefer,
	                      work.colAdd,
	                      work.colSub,
	                      work.LifeMin,
	                      work.LifeMax
) ;
}


/*********************************************

	エフェクト　水飛沫 キャラ付随　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
