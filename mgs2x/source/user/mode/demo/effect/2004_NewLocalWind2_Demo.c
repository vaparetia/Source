//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　風 ローカル２　のランチャー関数

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
	int     con_name ;
	FVECTOR *size ;
	float   intense ;
	int     name ;
} Work ;

typedef struct data_t {
	int     d0 ;
	FVECTOR d1 ;
	float   d2 ;
	int     d3 ;
} Data ;

void *NewLocalWind2_Demo_2004Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewLocalWind2_Demo( int ,FVECTOR *,float ,int  ) ;


	work.con_name = data->d0 ;
	work.size = &data->d1 ;
	work.intense = data->d2 ;
	work.name = data->d3 ;

	return NewLocalWind2_Demo( work.con_name,
	                           work.size,
	                           work.intense,
	                           work.name
) ;
}


/*********************************************

	エフェクト　風 ローカル２　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
