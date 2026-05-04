/*********************************************

	エフェクト　水中効果_水面にバンプ波紋　のランチャー関数

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
	float   upper ;
	float   below ;
} Work ;

typedef struct data_t {
	int     d0 ;
	float   d1 ;
	float   d2 ;
} Data ;

void *NewRipBubbleMan_DEMO_0201Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewRipBubbleMan_DEMO( int ,float ,float  ) ;


	work.name = data->d0 ;
	work.upper = data->d1 ;
	work.below = data->d2 ;

	return NewRipBubbleMan_DEMO( work.name,
	                             work.upper,
	                             work.below
) ;
}


/*********************************************

	エフェクト　水中効果_水面にバンプ波紋　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
