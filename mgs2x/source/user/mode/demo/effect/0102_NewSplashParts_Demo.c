//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　水飛沫 単発　のランチャー関数

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
	SVECTOR *rot ;
	float   intense ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	SVECTOR d1 ;
	float   d2 ;
} Data ;

#include "BP_EndianSupport.h"

void *NewSplashParts_Demo_0102Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewSplashParts_Demo( FVECTOR *,SVECTOR *,float  ) ;

   BP_LE_SwapSIntArray_Inp(&data->d1, 2);
   BP_LE_SwapSShortArray_Inp(&data->d1, 4);

	work.center = &data->d0 ;
	work.rot = &data->d1 ;
	work.intense = data->d2 ;

	return NewSplashParts_Demo( work.center,
	                            work.rot,
	                            work.intense
) ;
}


/*********************************************

	エフェクト　水飛沫 単発　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
