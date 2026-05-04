/*********************************************

	エフェクト　水効果_水中血　のランチャー関数

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
	FVECTOR *rot ;
	float   pow ;
	int     mode ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	FVECTOR d1 ;
	float   d3 ;
} Data ;

void *NewBloodWater_demo_b007Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewBloodWater_demo( FVECTOR *,FVECTOR *,float ,int  ) ;

	FVECTOR * rot ;
	FVECTOR  temp ;

	work.pos = &data->d0 ;
	rot = &data->d1 ;
	DM_GetFMatrixM_ZXY( &temp, rot, 2.000000  ) ;
	work.rot = &temp ;
	work.pow = data->d3 ;
	work.mode = ( int ) 0.000000  ;

	return NewBloodWater_demo( work.pos,
	                           work.rot,
	                           work.pow,
	                           work.mode
) ;
}


/*********************************************

	エフェクト　水効果_水中血　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
