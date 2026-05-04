//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　効果 メッセージ起動Ｂ　のランチャー関数

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

void *NewMesgBomb2_6010Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewMesgBomb2( int ,int  ) ;


	work.name = data->d0 ;
	work.con_name = data->d1 ;

	return NewMesgBomb2( work.name,
	                     work.con_name
) ;
}


/*********************************************

	エフェクト　効果 メッセージ起動Ｂ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
