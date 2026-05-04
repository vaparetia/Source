/*********************************************

	エフェクト　全画面_集中ブラー　のランチャー関数

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
	int     color ;
	float   pix_max ;
	int     name ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
	float   d2 ;
	int     d3 ;
} Data ;

void *NewScrConcentrateBlur_0015Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewScrConcentrateBlur( int ,int ,float ,int  ) ;

	IVECTOR * col ;

	work.con_name = data->d0 ;
	col = &data->d1 ;
	work.color = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;
	work.pix_max = data->d2 ;
	work.name = data->d3 ;

	return NewScrConcentrateBlur( work.con_name,
	                              work.color,
	                              work.pix_max,
	                              work.name
) ;
}


/*********************************************

	エフェクト　全画面_集中ブラー　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
