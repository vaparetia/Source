//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　物理シム・チェーン　のランチャー関数

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
	int     model_name ;
	int     sample_num ;
	FVECTOR *pos ;
	SVECTOR *rot ;
	int     collision_flag ;
} Work ;

typedef struct data_t {
	int     d0 ;
	FVECTOR d1 ;
	SVECTOR d2 ;
} Data ;

#include "BP_EndianSupport.h"

void *NewRopeModel2_called_3005Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewRopeModel2_called( int ,int ,FVECTOR *,SVECTOR *,int  ) ;

   BP_LE_SwapSIntArray_Inp(&data->d2, 2);
   BP_LE_SwapSShortArray_Inp(&data->d2, 4);

	work.model_name = data->d0 ;
	work.sample_num = ( int ) 1.000000  ;
	work.pos = &data->d1 ;
	work.rot = &data->d2 ;
	work.collision_flag = ( int ) 1.000000  ;

	return NewRopeModel2_called( work.model_name,
	                             work.sample_num,
	                             work.pos,
	                             work.rot,
	                             work.collision_flag
) ;
}


/*********************************************

	エフェクト　物理シム・チェーン　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
