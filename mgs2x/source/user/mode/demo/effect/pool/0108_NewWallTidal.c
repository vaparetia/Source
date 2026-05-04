/*********************************************

	エフェクト　水飛沫_ライン浸水　のランチャー関数

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
	FVECTOR *pos0 ;
	FVECTOR *pos1 ;
	FVECTOR *dire ;
	float   force ;
	float   width ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	FVECTOR d1 ;
	FVECTOR d2 ;
	float   d4 ;
	float   d5 ;
} Data ;

void *NewWallTidal_0108Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewWallTidal( FVECTOR *,FVECTOR *,FVECTOR *,float ,float  ) ;

	FVECTOR * rot ;
	FVECTOR  temp ;

	work.pos0 = &data->d0 ;
	work.pos1 = &data->d1 ;
	rot = &data->d2 ;
	DM_GetFMatrixM_ZXY( &temp, rot, 2.000000  ) ;
	work.dire = &temp ;
	work.force = data->d4 ;
	work.width = data->d5 ;

	return NewWallTidal( work.pos0,
	                     work.pos1,
	                     work.dire,
	                     work.force,
	                     work.width
) ;
}


/*********************************************

	エフェクト　水飛沫_ライン浸水　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
