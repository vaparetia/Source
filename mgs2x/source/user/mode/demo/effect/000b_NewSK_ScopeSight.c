//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　全画面　双眼鏡　のランチャー関数

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
} Work ;

typedef struct data_t {
	float   d0 ;
} Data ;

void *NewSK_ScopeSight_000bLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewSK_ScopeSight( float  ) ;


	work.scale = data->d0 ;

	return NewSK_ScopeSight( work.scale
) ;
}


/*********************************************

	エフェクト　全画面　双眼鏡　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
