/*********************************************

	エフェクト　稲光　のランチャー関数

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
	float   sx ;
	float   sy ;
} Work ;

typedef struct data_t {
	float   d0 ;
	float   d1 ;
} Data ;

void *NewThunder_Demo_2000Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewThunder_Demo( float ,float  ) ;


	work.sx = data->d0 ;
	work.sy = data->d1 ;

	return NewThunder_Demo( work.sx,
	                        work.sy
) ;
}


/*********************************************

	エフェクト　稲光　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
