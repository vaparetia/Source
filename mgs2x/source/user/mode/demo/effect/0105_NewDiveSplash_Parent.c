//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　水飛沫 落下　のランチャー関数

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
	FVECTOR *center ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
} Data ;

void *NewDiveSplash_Parent_0105Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDiveSplash_Parent( FVECTOR * ) ;


	work.center = &data->d0 ;

	return NewDiveSplash_Parent( work.center
) ;
}


/*********************************************

	エフェクト　水飛沫 落下　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
