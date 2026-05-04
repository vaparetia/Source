/*********************************************

	エフェクト　血 水中死体遭遇　のランチャー関数

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
	float   width ;
	float   height ;
	int     col ;
} Work ;

typedef struct data_t {
	int     d0 ;
	float   d1 ;
	float   d2 ;
	IVECTOR d3 ;
} Data ;

void *NewFluidFlowMan_7009Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewFluidFlowMan( int ,float ,float ,int  ) ;

	IVECTOR * col ;

	work.name = data->d0 ;
	work.width = data->d1 ;
	work.height = data->d2 ;
	col = &data->d3 ;
	work.col = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;

	return NewFluidFlowMan( work.name,
	                        work.width,
	                        work.height,
	                        work.col
) ;
}


/*********************************************

	エフェクト　血 水中死体遭遇　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
