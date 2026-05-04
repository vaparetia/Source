/*********************************************

	エフェクト　煙_汎用立ち昇る煙ダミー　のランチャー関数

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
	int     con_name ;
	float   init_speed ;
	float   rising_speed ;
	float   init_size ;
	float   last_size ;
	int     alpha ;
	int     num ;
} Work ;

typedef struct data_t {
	int     d0 ;
	float   d1 ;
	float   d2 ;
	float   d3 ;
	float   d4 ;
	int     d5 ;
	int     d6 ;
} Data ;

void *NewDemoRisingSmokeDummy_710aLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoRisingSmokeDummy( int ,float ,float ,float ,float ,int ,int  ) ;


	work.con_name = data->d0 ;
	work.init_speed = data->d1 ;
	work.rising_speed = data->d2 ;
	work.init_size = data->d3 ;
	work.last_size = data->d4 ;
	work.alpha = data->d5 ;
	work.num = data->d6 ;

	return NewDemoRisingSmokeDummy( work.con_name,
	                                work.init_speed,
	                                work.rising_speed,
	                                work.init_size,
	                                work.last_size,
	                                work.alpha,
	                                work.num
) ;
}


/*********************************************

	エフェクト　煙_汎用立ち昇る煙ダミー　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
