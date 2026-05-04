/*********************************************

	エフェクト　フェードＩＯカット　のランチャー関数

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
	int     ini_r ;
	int     ini_g ;
	int     ini_b ;
	int     ini_a ;
	int     r ;
	int     g ;
	int     b ;
	int     a ;
	int     time ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	IVECTOR d1 ;
	int     d2 ;
	int     d3 ;
} Data ;

void *NewFadeInOutForce_Demo_0008Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewFadeInOutForce_Demo( int ,int ,int ,int ,int ,int ,int ,int ,int  ) ;

	IVECTOR * col ;
	int      max ;
	int      min ;

	col = &data->d0 ;
	work.ini_r = col->vx ;
	work.ini_g = col->vy ;
	work.ini_b = col->vz ;
	work.ini_a = col->vw ;
	col = &data->d1 ;
	work.r = col->vx ;
	work.g = col->vy ;
	work.b = col->vz ;
	work.a = col->vw ;
	max = data->d2 ;
	min = data->d3 ;
	work.time = (max - min) ;

	return NewFadeInOutForce_Demo( work.ini_r,
	                               work.ini_g,
	                               work.ini_b,
	                               work.ini_a,
	                               work.r,
	                               work.g,
	                               work.b,
	                               work.a,
	                               work.time
) ;
}


/*********************************************

	エフェクト　フェードＩＯカット　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
