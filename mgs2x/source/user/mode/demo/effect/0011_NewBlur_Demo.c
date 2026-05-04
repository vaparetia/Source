//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　ブラー　のランチャー関数

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
	float   intense ;
} Work ;

typedef struct data_t {
	int     d0 ;
	float   d1 ;
} Data ;

void *NewBlur_Demo_0011Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewBlur_Demo( int ,float  ) ;


	work.name = data->d0 ;
	work.intense = data->d1 ;

	return NewBlur_Demo( work.name,
	                     work.intense
) ;
}


/*********************************************

	エフェクト　ブラー　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
