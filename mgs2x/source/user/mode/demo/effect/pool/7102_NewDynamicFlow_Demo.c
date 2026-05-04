/*********************************************

	エフェクト　煙 フロースモーク　のランチャー関数

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
	FVECTOR *bound0 ;
	FVECTOR *bound1 ;
	float   size ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	FVECTOR d1 ;
	float   d2 ;
} Data ;

void *NewDynamicFlow_Demo_7102Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDynamicFlow_Demo( FVECTOR *,FVECTOR *,float  ) ;


	work.bound0 = &data->d0 ;
	work.bound1 = &data->d1 ;
	work.size = data->d2 ;

	return NewDynamicFlow_Demo( work.bound0,
	                            work.bound1,
	                            work.size
) ;
}


/*********************************************

	エフェクト　煙 フロースモーク　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
