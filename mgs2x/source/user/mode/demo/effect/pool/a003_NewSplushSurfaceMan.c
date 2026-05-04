/*********************************************

	エフェクト　常駐 水飛沫　のランチャー関数

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

void *NewSplushSurfaceMan_a003Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewSplushSurfaceMan( int ,int  ) ;


	work.name = data->d0 ;
	work.map = DM_GetCurrentMap(  ) ;

	return NewSplushSurfaceMan( work.name,
	                            work.map
) ;
}


/*********************************************

	エフェクト　常駐 水飛沫　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
