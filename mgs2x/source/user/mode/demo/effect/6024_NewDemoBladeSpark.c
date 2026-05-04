//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　効果 ソリダス刀火花　のランチャー関数

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

void *NewDemoBladeSpark_6024Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoBladeSpark( FVECTOR * ) ;


	work.pos = &data->d0 ;

	return NewDemoBladeSpark( work.pos
) ;
}


/*********************************************

	エフェクト　効果 ソリダス刀火花　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
