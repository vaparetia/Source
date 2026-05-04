//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　効果 ソリダス蛇手エフェクト　のランチャー関数

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
	void    *evmobj ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
} Data ;

void *NewDemoSolidusSnakearmFlow_6025Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoSolidusSnakearmFlow( int ,void * ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.evmobj = DM_GetEvmObjObject( handle ) ;

	return NewDemoSolidusSnakearmFlow( work.name,
	                                   work.evmobj
) ;
}


/*********************************************

	エフェクト　効果 ソリダス蛇手エフェクト　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
