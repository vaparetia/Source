//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　全画面 クロスフェード　のランチャー関数

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
	int     d2 ;
} Data ;

void *NewCrossFadeEffect_0012Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewCrossFadeEffect( int  ) ;

	int      max ;
	int      min ;
	float    fps ;

	max = data->d0 ;
	min = data->d1 ;
	fps = (( float ) 300.000000  / data->d2) ;
	work.time = ((max - min) * fps) ;

	return NewCrossFadeEffect( work.time
) ;
}


/*********************************************

	エフェクト　全画面 クロスフェード　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
