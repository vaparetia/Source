//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　カメラレンズ面効果　のランチャー関数

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
	int     time ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
} Data ;

void *NewRainCamera_Demo_0004Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewRainCamera_Demo( int  ) ;

	int      max ;
	int      min ;

	max = data->d0 ;
	min = data->d1 ;
	work.time = (max - min) ;

	return NewRainCamera_Demo( work.time
) ;
}


/*********************************************

	エフェクト　カメラレンズ面効果　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
