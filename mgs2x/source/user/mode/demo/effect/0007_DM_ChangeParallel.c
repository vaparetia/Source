//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　全画面 平行光　のランチャー関数

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
	IVECTOR *rgb ;
	FVECTOR *dir ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	FVECTOR d1 ;
} Data ;

void *DM_ChangeParallel_0007Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *DM_ChangeParallel( IVECTOR *,FVECTOR * ) ;


	work.rgb = &data->d0 ;
	work.dir = &data->d1 ;

	return DM_ChangeParallel( work.rgb,
	                          work.dir
) ;
}


/*********************************************

	エフェクト　全画面 平行光　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
