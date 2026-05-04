/*********************************************

	エフェクト　煙_汎用立ち昇る煙固定型　のランチャー関数

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
	SVECTOR *rot ;
	float   init_speed ;
	float   rising_speed ;
	float   init_size ;
	float   last_size ;
	int     alpha ;
	int     num ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	SVECTOR d1 ;
	float   d2 ;
	float   d3 ;
	float   d4 ;
	float   d5 ;
	int     d6 ;
	int     d7 ;
} Data ;

void *NewDemoRisingSmokeFix_7109Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoRisingSmokeFix( FVECTOR *,SVECTOR *,float ,float ,float ,float ,int ,int  ) ;


	work.pos = &data->d0 ;
	work.rot = &data->d1 ;
	work.init_speed = data->d2 ;
	work.rising_speed = data->d3 ;
	work.init_size = data->d4 ;
	work.last_size = data->d5 ;
	work.alpha = data->d6 ;
	work.num = data->d7 ;

	return NewDemoRisingSmokeFix( work.pos,
	                              work.rot,
	                              work.init_speed,
	                              work.rising_speed,
	                              work.init_size,
	                              work.last_size,
	                              work.alpha,
	                              work.num
) ;
}


/*********************************************

	エフェクト　煙_汎用立ち昇る煙固定型　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
