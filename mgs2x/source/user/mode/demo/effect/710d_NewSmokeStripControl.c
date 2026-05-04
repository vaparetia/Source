//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　煙 乱流　のランチャー関数

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
	FVECTOR *pos0 ;
	FVECTOR *pos1 ;
	float   size ;
	int     num ;
	int     color ;
	int     seed ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	FVECTOR d1 ;
	float   d2 ;
	int     d3 ;
	IVECTOR d4 ;
	int     d5 ;
} Data ;

void *NewSmokeStripControl_710dLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewSmokeStripControl( FVECTOR *,FVECTOR *,float ,int ,int ,int  ) ;

	IVECTOR * col ;

	work.pos0 = &data->d0 ;
	work.pos1 = &data->d1 ;
	work.size = data->d2 ;
	work.num = data->d3 ;
	col = &data->d4 ;
	work.color = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;
	work.seed = data->d5 ;

	return NewSmokeStripControl( work.pos0,
	                             work.pos1,
	                             work.size,
	                             work.num,
	                             work.color,
	                             work.seed
) ;
}


/*********************************************

	エフェクト　煙 乱流　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
