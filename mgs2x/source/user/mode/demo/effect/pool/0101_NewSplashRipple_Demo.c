/*********************************************

	エフェクト　水飛沫 風紋　のランチャー関数

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
	FVECTOR *center ;
	float   radius ;
	float   direction ;
	float   angle ;
	float   size ;
	int     multiple ;
	int     time ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	float   d1 ;
	float   d2 ;
	float   d3 ;
	float   d4 ;
	int     d5 ;
	int     d6 ;
	int     d7 ;
} Data ;

void *NewSplashRipple_Demo_0101Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewSplashRipple_Demo( FVECTOR *,float ,float ,float ,float ,int ,int  ) ;

	int      max ;
	int      min ;

	work.center = &data->d0 ;
	work.radius = data->d1 ;
	work.direction = data->d2 ;
	work.angle = data->d3 ;
	work.size = data->d4 ;
	work.multiple = data->d5 ;
	max = data->d6 ;
	min = data->d7 ;
	work.time = (max - min) ;

	return NewSplashRipple_Demo( work.center,
	                             work.radius,
	                             work.direction,
	                             work.angle,
	                             work.size,
	                             work.multiple,
	                             work.time
) ;
}


/*********************************************

	エフェクト　水飛沫 風紋　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
