//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　レイ_ステージ床フォグ消去　のランチャー関数

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
	int     size ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	int     d1 ;
} Data ;

void *PDRAY_CLOUD_DeleteDensity_d009Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *PDRAY_CLOUD_DeleteDensity( FVECTOR *,int  ) ;


	work.pos = &data->d0 ;
	work.size = data->d1 ;

	return PDRAY_CLOUD_DeleteDensity( work.pos,
	                                  work.size
) ;
}


/*********************************************

	エフェクト　レイ_ステージ床フォグ消去　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
