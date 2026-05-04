//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　レイ_吹き出し血　のランチャー関数

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
	int     con_name ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
} Data ;

void *NewDemoRayFallBlood_d00aLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoRayFallBlood( int ,int  ) ;


	work.name = data->d0 ;
	work.con_name = data->d1 ;

	return NewDemoRayFallBlood( work.name,
	                            work.con_name
) ;
}


/*********************************************

	エフェクト　レイ_吹き出し血　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
