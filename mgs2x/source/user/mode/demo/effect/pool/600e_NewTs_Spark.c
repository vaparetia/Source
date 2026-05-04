/*********************************************

	エフェクト　効果 火花　のランチャー関数

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
	FVECTOR *vec ;
	float   pow ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	FVECTOR d1 ;
	float   d3 ;
} Data ;

void *NewTs_Spark_600eLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewTs_Spark( FVECTOR *,FVECTOR *,float  ) ;

	FVECTOR * rot ;
	FVECTOR  force ;

	work.center = &data->d0 ;
	rot = &data->d1 ;
	DM_GetFMatrixM_ZXY( &force, rot, 2.000000  ) ;
	work.vec = &force ;
	work.pow = data->d3 ;

	return NewTs_Spark( work.center,
	                    work.vec,
	                    work.pow
) ;
}


/*********************************************

	エフェクト　効果 火花　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
