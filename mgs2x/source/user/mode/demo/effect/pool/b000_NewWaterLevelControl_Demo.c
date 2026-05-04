/*********************************************

	エフェクト　水効果 水位制御　のランチャー関数

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
	float   init_level ;
} Work ;

typedef struct data_t {
	int     d0 ;
	float   d1 ;
} Data ;

void *NewWaterLevelControl_Demo_b000Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewWaterLevelControl_Demo( int ,float  ) ;


	work.name = data->d0 ;
	work.init_level = data->d1 ;

	return NewWaterLevelControl_Demo( work.name,
	                                  work.init_level
) ;
}


/*********************************************

	エフェクト　水効果 水位制御　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
