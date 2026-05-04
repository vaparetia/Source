//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　血 血煙　のランチャー関数

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
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
} Data ;

void *AN_Blood_Mist_7003Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *AN_Blood_Mist( FVECTOR * ) ;


	work.pos = &data->d0 ;

	return AN_Blood_Mist( work.pos
) ;
}


/*********************************************

	エフェクト　血 血煙　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
