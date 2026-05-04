/*********************************************

	エフェクト　効果 色指定プラズマ（ライン）　のランチャー関数

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
	int     con_name0 ;
	int     con_name1 ;
	int     radius ;
	int     life ;
	int     size ;
	int     color ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
	int     d2 ;
	int     d3 ;
	int     d4 ;
	int     d5 ;
	IVECTOR d6 ;
} Data ;

void *NewDemoPlasmaLineColor_602aLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoPlasmaLineColor( int ,int ,int ,int ,int ,int  ) ;

	int      max ;
	int      min ;
	IVECTOR * col ;

	work.con_name0 = data->d0 ;
	work.con_name1 = data->d1 ;
	work.radius = data->d2 ;
	max = data->d3 ;
	min = data->d4 ;
	work.life = (max - min) ;
	work.size = data->d5 ;
	col = &data->d6 ;
	work.color = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;

	return NewDemoPlasmaLineColor( work.con_name0,
	                               work.con_name1,
	                               work.radius,
	                               work.life,
	                               work.size,
	                               work.color
) ;
}


/*********************************************

	エフェクト　効果 色指定プラズマ（ライン）　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
