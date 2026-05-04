/*********************************************

	エフェクト　コントラストカット　のランチャー関数

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
	int     nega_posi_flag ;
	int     time ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	IVECTOR d1 ;
	int     d2 ;
	int     d3 ;
	int     d4 ;
} Data ;

void *NewContrastForce_Demo_000aLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewContrastForce_Demo( int ,int ,int ,int ,int ,int ,int ,int ,int ,int  ) ;

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
	work.nega_posi_flag = data->d2 ;
	max = data->d3 ;
	min = data->d4 ;
	work.time = (max - min) ;

	return NewContrastForce_Demo( work.ini_r,
	                              work.ini_g,
	                              work.ini_b,
	                              work.ini_a,
	                              work.r,
	                              work.g,
	                              work.b,
	                              work.a,
	                              work.nega_posi_flag,
	                              work.time
) ;
}


/*********************************************

	エフェクト　コントラストカット　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
