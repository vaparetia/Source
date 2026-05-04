/*********************************************

	エフェクト　全画面_デジカメシャッター　のランチャー関数

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
} Work ;

typedef struct data_t {
	int     d0 ;
} Data ;

void *NewDEMODigitalCamera_001aLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDEMODigitalCamera( int  ) ;


	work.name = data->d0 ;

	return NewDEMODigitalCamera( work.name
) ;
}


/*********************************************

	エフェクト　全画面_デジカメシャッター　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
