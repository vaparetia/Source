//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　スローパラメータ制御　のランチャー関数

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
	int     param ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
} Data ;

void *NewSlowParamMan_prog_2003Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewSlowParamMan_prog( int ,int  ) ;


	work.name = data->d0 ;
	work.param = data->d1 ;

	return NewSlowParamMan_prog( work.name,
	                             work.param
) ;
}


/*********************************************

	エフェクト　スローパラメータ制御　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
