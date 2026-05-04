//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　効果 中煙　のランチャー関数

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
	SVECTOR *pole_rot ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	SVECTOR d1 ;
} Data ;

#include "BP_EndianSupport.h"

void *NewBombGasEffect_600bLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewBombGasEffect( FVECTOR *,SVECTOR * ) ;

   BP_LE_SwapSIntArray_Inp(&data->d1, 2);
   BP_LE_SwapSShortArray_Inp(&data->d1, 4);

	work.pos = &data->d0 ;
	work.pole_rot = &data->d1 ;

	return NewBombGasEffect( work.pos,
	                         work.pole_rot
) ;
}


/*********************************************

	エフェクト　効果 中煙　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
