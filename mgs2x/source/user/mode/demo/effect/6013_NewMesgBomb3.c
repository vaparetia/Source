//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　効果 簡易エフェクトＢ　のランチャー関数

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
	int     conname ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
} Data ;

void *NewMesgBomb3_6013Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewMesgBomb3( int ,int  ) ;


	work.name = data->d0 ;
	work.conname = data->d1 ;

	return NewMesgBomb3( work.name,
	                     work.conname
) ;
}


/*********************************************

	エフェクト　効果 簡易エフェクトＢ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
