//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　モデル 影追加と削除　のランチャー関数

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
	int     data0 ;
	int     data1 ;
	int     data2 ;
	int     data3 ;
	int     data4 ;
	int     data5 ;
	int     data6 ;
	int     data7 ;
	int     command ;
	int     num ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
	int     d2 ;
	int     d3 ;
} Data ;

void *DM_SendEffectMessage_4003Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *DM_SendEffectMessage( int ,int ,int ,int ,int ,int ,int ,int ,int ,int ,int  ) ;


	work.name = data->d0 ;
	work.data0 = data->d1 ;
	work.data1 = data->d2 ;
	work.data2 = ( int ) 0.000000  ;
	work.data3 = ( int ) 0.000000  ;
	work.data4 = ( int ) 0.000000  ;
	work.data5 = ( int ) 0.000000  ;
	work.data6 = ( int ) 0.000000  ;
	work.data7 = ( int ) 0.000000  ;
	work.command = data->d3 ;
	work.num = ( int ) 2.000000  ;

	return DM_SendEffectMessage( work.name,
	                             work.data0,
	                             work.data1,
	                             work.data2,
	                             work.data3,
	                             work.data4,
	                             work.data5,
	                             work.data6,
	                             work.data7,
	                             work.command,
	                             work.num
) ;
}


/*********************************************

	エフェクト　モデル 影追加と削除　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
