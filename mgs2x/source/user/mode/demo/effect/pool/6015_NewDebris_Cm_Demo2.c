/*********************************************

	エフェクト　効果 コモデル破片ダミー　のランチャー関数

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
	int     name1 ;
	int     name2 ;
	float   pow ;
	int     num ;
	int     objcode ;
	int     color ;
	float   scale ;
	int     flags ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
	float   d2 ;
	int     d3 ;
	int     d4 ;
	IVECTOR d5 ;
	float   d6 ;
	int     d7 ;
} Data ;

void *NewDebris_Cm_Demo2_6015Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDebris_Cm_Demo2( int ,int ,float ,int ,int ,int ,float ,int  ) ;

	IVECTOR * col ;

	work.name1 = data->d0 ;
	work.name2 = data->d1 ;
	work.pow = data->d2 ;
	work.num = data->d3 ;
	work.objcode = data->d4 ;
	col = &data->d5 ;
	work.color = (((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) ;
	work.scale = data->d6 ;
	work.flags = data->d7 ;

	return NewDebris_Cm_Demo2( work.name1,
	                           work.name2,
	                           work.pow,
	                           work.num,
	                           work.objcode,
	                           work.color,
	                           work.scale,
	                           work.flags
) ;
}


/*********************************************

	エフェクト　効果 コモデル破片ダミー　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
