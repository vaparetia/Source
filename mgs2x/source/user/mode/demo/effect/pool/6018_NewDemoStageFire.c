/*********************************************

	エフェクト　効果 汎用ステージ炎　のランチャー関数

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
	FVECTOR *pos ;
} Work ;

typedef struct data_t {
	int     d0 ;
	FVECTOR d1 ;
} Data ;

void *NewDemoStageFire_6018Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoStageFire( int ,FVECTOR * ) ;


	work.name = data->d0 ;
	work.pos = &data->d1 ;

	return NewDemoStageFire( work.name,
	                         work.pos
) ;
}


/*********************************************

	エフェクト　効果 汎用ステージ炎　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
