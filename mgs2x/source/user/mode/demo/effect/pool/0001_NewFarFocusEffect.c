/*********************************************

	エフェクト　遠景ぼかし　のランチャー関数

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
	int     max_plane ;
	int     near_para ;
	int     far_para ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
	int     d2 ;
	int     d3 ;
} Data ;

void *NewFarFocusEffect_0001Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewFarFocusEffect( int ,int ,int ,int  ) ;


	work.name = data->d0 ;
	work.max_plane = data->d1 ;
	work.near_para = data->d2 ;
	work.far_para = data->d3 ;

	return NewFarFocusEffect( work.name,
	                          work.max_plane,
	                          work.near_para,
	                          work.far_para
) ;
}


/*********************************************

	エフェクト　遠景ぼかし　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
