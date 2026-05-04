//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　全画面 CLUTフラッシュ　のランチャー関数

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
	int     decay ;
	int     mode ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
	int     d2 ;
} Data ;

void *NewFlush_0005Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewFlush( int ,int  ) ;

	int      max ;
	int      min ;

	max = data->d0 ;
	min = data->d1 ;
	work.decay = (max - min) ;
	work.mode = data->d2 ;

	return NewFlush( work.decay,
	                 work.mode
) ;
}


/*********************************************

	エフェクト　全画面 CLUTフラッシュ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
