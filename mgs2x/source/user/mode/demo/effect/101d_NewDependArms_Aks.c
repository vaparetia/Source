//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　火器_状況ＡＫＳ　のランチャー関数

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
	int     flags ;
} Work ;

typedef struct data_t {
	int     d0 ;
} Data ;

void *NewDependArms_Aks_101dLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDependArms_Aks( int ,int  ) ;


	work.name = data->d0 ;
	work.flags = ( int ) 0.000000  ;

	return NewDependArms_Aks( work.name,
	                          work.flags
) ;
}


/*********************************************

	エフェクト　火器_状況ＡＫＳ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
