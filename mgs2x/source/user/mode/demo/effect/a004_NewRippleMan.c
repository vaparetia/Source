//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　常駐 波紋　のランチャー関数

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

typedef struct data_t {
	int     d0 ;
} Data ;

void *NewRippleMan_a004Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewRippleMan( int ,int  ) ;


	work.name = data->d0 ;
	work.map = DM_GetCurrentMap(  ) ;

	return NewRippleMan( work.name,
	                     work.map
) ;
}


/*********************************************

	エフェクト　常駐 波紋　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
