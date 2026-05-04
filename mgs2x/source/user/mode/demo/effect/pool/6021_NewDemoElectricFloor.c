/*********************************************

	エフェクト　効果 電撃床プラズマ　のランチャー関数

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

void *NewDemoElectricFloor_6021Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoElectricFloor( int  ) ;


	work.name = data->d0 ;

	return NewDemoElectricFloor( work.name
) ;
}


/*********************************************

	エフェクト　効果 電撃床プラズマ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
