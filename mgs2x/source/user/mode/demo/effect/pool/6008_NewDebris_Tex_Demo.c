/*********************************************

	エフェクト　効果 スプライト破片　のランチャー関数

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
	FVECTOR *bound0 ;
	FVECTOR *bound1 ;
	FVECTOR *force ;
	float   pow ;
	int     num ;
	int     objcode ;
	int     color ;
	float   w ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	FVECTOR d1 ;
	FVECTOR d2 ;
	float   d4 ;
	int     d5 ;
	int     d6 ;
	IVECTOR d7 ;
	float   d8 ;
} Data ;

void *NewDebris_Tex_Demo_6008Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDebris_Tex_Demo( FVECTOR *,FVECTOR *,FVECTOR *,float ,int ,int ,int ,float  ) ;

	FVECTOR * rot ;
	FVECTOR  vec ;
	IVECTOR * col ;

	work.bound0 = &data->d0 ;
	work.bound1 = &data->d1 ;
	rot = &data->d2 ;
	DM_GetFMatrixM_ZXY( &vec, rot, 2.000000  ) ;
	work.force = &vec ;
	work.pow = data->d4 ;
	work.num = data->d5 ;
	work.objcode = data->d6 ;
	col = &data->d7 ;
	work.color = (((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) ;
	work.w = data->d8 ;

	return NewDebris_Tex_Demo( work.bound0,
	                           work.bound1,
	                           work.force,
	                           work.pow,
	                           work.num,
	                           work.objcode,
	                           work.color,
	                           work.w
) ;
}


/*********************************************

	エフェクト　効果 スプライト破片　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
