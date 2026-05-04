//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　水飛沫 ハリアー　のランチャー関数

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
	float   inf_height ;
	float   max_width ;
} Work ;

typedef struct data_t {
	int     d0 ;
	float   d1 ;
	float   d2 ;
} Data ;

void *NewWaterWindSplush_DEMO_0109Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewWaterWindSplush_DEMO( int ,float ,float  ) ;


	work.name = data->d0 ;
	work.inf_height = data->d1 ;
	work.max_width = data->d2 ;

	return NewWaterWindSplush_DEMO( work.name,
	                                work.inf_height,
	                                work.max_width
) ;
}


/*********************************************

	エフェクト　水飛沫 ハリアー　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
