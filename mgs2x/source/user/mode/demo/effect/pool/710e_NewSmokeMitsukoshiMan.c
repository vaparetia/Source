/*********************************************

	エフェクト　煙_三越煙　のランチャー関数

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
	float   size ;
	int     col ;
	float   wind_param ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	float   d1 ;
	IVECTOR d2 ;
	float   d3 ;
} Data ;

void *NewSmokeMitsukoshiMan_710eLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewSmokeMitsukoshiMan( FVECTOR *,float ,int ,float  ) ;

	IVECTOR * col ;

	work.center = &data->d0 ;
	work.size = data->d1 ;
	col = &data->d2 ;
	work.col = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;
	work.wind_param = data->d3 ;

	return NewSmokeMitsukoshiMan( work.center,
	                              work.size,
	                              work.col,
	                              work.wind_param
) ;
}


/*********************************************

	エフェクト　煙_三越煙　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
