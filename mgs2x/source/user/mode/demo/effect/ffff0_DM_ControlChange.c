//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　使用禁止 ダミーポイント　のランチャー関数

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
	float   rot_x ;
	float   rot_y ;
	float   rot_z ;
	float   pos_x ;
	float   pos_y ;
	float   pos_z ;
} Work ;

typedef struct data_t {
	int     d0 ;
	float   d1 ;
	float   d2 ;
	float   d3 ;
	float   d4 ;
	float   d5 ;
	float   d6 ;
} Data ;

void *DM_ControlChange_ffff0Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *DM_ControlChange( int ,float ,float ,float ,float ,float ,float  ) ;


	work.name = data->d0 ;
	work.rot_x = data->d1 ;
	work.rot_y = data->d2 ;
	work.rot_z = data->d3 ;
	work.pos_x = data->d4 ;
	work.pos_y = data->d5 ;
	work.pos_z = data->d6 ;

	return DM_ControlChange( work.name,
	                         work.rot_x,
	                         work.rot_y,
	                         work.rot_z,
	                         work.pos_x,
	                         work.pos_y,
	                         work.pos_z
) ;
}


/*********************************************

	エフェクト　使用禁止 ダミーポイント　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
