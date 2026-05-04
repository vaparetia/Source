/*********************************************

	エフェクト　フォグ　のランチャー関数

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
	int     r ;
	int     g ;
	int     b ;
	float   near_para ;
	float   far_para ;
	int     time ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	float   d1 ;
	float   d2 ;
	int     d3 ;
	int     d4 ;
} Data ;

void *NewFogSet_Demo_0000Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewFogSet_Demo( int ,int ,int ,float ,float ,int  ) ;

	IVECTOR * col ;
	int      max ;
	int      min ;

	col = &data->d0 ;
	work.r = col->vx ;
	work.g = col->vy ;
	work.b = col->vz ;
	work.near_para = data->d1 ;
	work.far_para = data->d2 ;
	max = data->d3 ;
	min = data->d4 ;
	work.time = (max - min) ;

	return NewFogSet_Demo( work.r,
	                       work.g,
	                       work.b,
	                       work.near_para,
	                       work.far_para,
	                       work.time
) ;
}


/*********************************************

	エフェクト　フォグ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
