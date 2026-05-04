/*********************************************

	エフェクト　常駐 ＡＫＳ薬莢確保　のランチャー関数

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
	int     map ;
} Work ;

typedef struct data_t {
	int     d0 ;
} Data ;

void *NewCartridgeAksALL_a001Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewCartridgeAksALL( int ,int  ) ;


	work.name = data->d0 ;
	work.map = DM_GetCurrentMap(  ) ;

	return NewCartridgeAksALL( work.name,
	                           work.map
) ;
}


/*********************************************

	エフェクト　常駐 ＡＫＳ薬莢確保　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
