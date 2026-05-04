/*********************************************

	エフェクト　血 落ちる血　のランチャー関数

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
	FVECTOR *pos ;
	int     decay ;
	int     white ;
	int     spread_size ;
	int     add ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	int     d1 ;
	int     d2 ;
	int     d3 ;
} Data ;

void *NewBloodDropsSize_7004Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewBloodDropsSize( FVECTOR *,int ,int ,int ,int  ) ;


	work.pos = &data->d0 ;
	work.decay = data->d1 ;
	work.white = data->d2 ;
	work.spread_size = data->d3 ;
	work.add = ( int ) -1.000000  ;

	return NewBloodDropsSize( work.pos,
	                          work.decay,
	                          work.white,
	                          work.spread_size,
	                          work.add
) ;
}


/*********************************************

	エフェクト　血 落ちる血　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
