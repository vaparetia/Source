//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　効果 フォーチュンバリアー　のランチャー関数

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
	void    *objs ;
	float   length ;
	float   front ;
	float   vertical ;
	float   size ;
	int     where ;
	int     flag ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	float   d3 ;
	float   d4 ;
	float   d5 ;
	float   d6 ;
	int     d8 ;
} Data ;

void *NewFortBarrierDemo_6014Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewFortBarrierDemo( void *,float ,float ,float ,float ,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.objs = DM_GetObjectDgObjs( handle ) ;
	work.length = data->d3 ;
	work.front = data->d4 ;
	work.vertical = data->d5 ;
	work.size = data->d6 ;
	work.where = DM_GetCurrentMap(  ) ;
	work.flag = data->d8 ;

	return NewFortBarrierDemo( work.objs,
	                           work.length,
	                           work.front,
	                           work.vertical,
	                           work.size,
	                           work.where,
	                           work.flag
) ;
}


/*********************************************

	エフェクト　効果 フォーチュンバリアー　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
