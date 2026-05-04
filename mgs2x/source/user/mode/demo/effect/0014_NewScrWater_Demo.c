//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　全画面_水中歪み　のランチャー関数

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
	float   scale ;
	int     color ;
	float   time_scl ;
} Work ;

typedef struct data_t {
	float   d0 ;
	IVECTOR d1 ;
	float   d2 ;
} Data ;

void *NewScrWater_Demo_0014Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewScrWater_Demo( float ,int ,float  ) ;

	IVECTOR * col ;

	work.scale = data->d0 ;
	col = &data->d1 ;
	work.color = (((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) ;
	work.time_scl = data->d2 ;

	return NewScrWater_Demo( work.scale,
	                         work.color,
	                         work.time_scl
) ;
}


/*********************************************

	エフェクト　全画面_水中歪み　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
