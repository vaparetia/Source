//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　Ｊ・Ｗ橋の交通　のランチャー関数

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
	float   z_crip ;
	int     max_car ;
	int     f_speed ;
} Work ;

typedef struct data_t {
	int     d0 ;
	float   d1 ;
	int     d2 ;
	int     d3 ;
} Data ;

void *NewTraffic_Demo_07d1Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewTraffic_Demo( int ,float ,int ,int  ) ;


	work.name = data->d0 ;
	work.z_crip = data->d1 ;
	work.max_car = data->d2 ;
	work.f_speed = data->d3 ;

	return NewTraffic_Demo( work.name,
	                        work.z_crip,
	                        work.max_car,
	                        work.f_speed
) ;
}


/*********************************************

	エフェクト　Ｊ・Ｗ橋の交通　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
