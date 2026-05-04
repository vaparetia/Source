/*********************************************

	エフェクト　システム エフェクト初期化　のランチャー関数

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

void *NewDemoEffectInitialize_fff05Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoEffectInitialize( int  ) ;


	work.name = data->d0 ;

	return NewDemoEffectInitialize( work.name
) ;
}


/*********************************************

	エフェクト　システム エフェクト初期化　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
