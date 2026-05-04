//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　全画面_水カメラ滴り　のランチャー関数

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
	int     map ;
} Work ;

void *NewScrWaterFilm_0026Launch( int id, void *argv )
{
	Work  work ;

	extern void *NewScrWaterFilm( int ,int  ) ;


	work.name = ( int ) 0.000000  ;
	work.map = ( int ) 0.000000  ;

	return NewScrWaterFilm( work.name,
	                        work.map
) ;
}


/*********************************************

	エフェクト　全画面_水カメラ滴り　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
