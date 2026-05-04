//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　風 ローカル　のランチャー関数

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
	FVECTOR *pos ;
	FVECTOR *size ;
	float   rot_y ;
	float   rot_diff ;
	float   intense ;
	int     name ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	FVECTOR d1 ;
	float   d2 ;
	float   d3 ;
	float   d4 ;
	int     d5 ;
} Data ;

void *NewLocalWind_Demo_2002Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewLocalWind_Demo( FVECTOR *,FVECTOR *,float ,float ,float ,int  ) ;


	work.pos = &data->d0 ;
	work.size = &data->d1 ;
	work.rot_y = ((data->d2 * ( float ) 2048.000000 ) / ( float ) 3.141593 ) ;
	work.rot_diff = data->d3 ;
	work.intense = data->d4 ;
	work.name = data->d5 ;

	return NewLocalWind_Demo( work.pos,
	                          work.size,
	                          work.rot_y,
	                          work.rot_diff,
	                          work.intense,
	                          work.name
) ;
}


/*********************************************

	エフェクト　風 ローカル　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
