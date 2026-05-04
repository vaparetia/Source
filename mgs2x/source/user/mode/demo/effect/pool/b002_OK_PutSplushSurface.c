/*********************************************

	エフェクト　水効果 単発水飛沫　のランチャー関数

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
	FVECTOR *vec ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	FVECTOR d1 ;
	float   d2 ;
} Data ;

void *OK_PutSplushSurface_b002Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *OK_PutSplushSurface( FVECTOR *,FVECTOR * ) ;

	FVECTOR * rot ;
	float    pow ;
	FVECTOR  temp ;

	work.pos = &data->d0 ;
	rot = &data->d1 ;
	pow = data->d2 ;
	DM_GetFMatrixM_ZXY( &temp, rot, 2.000000  ) ;
	_sceVu0ScaleVector( &temp, &temp, pow ) ;
	work.vec = &temp ;

	return OK_PutSplushSurface( work.pos,
	                            work.vec
) ;
}


/*********************************************

	エフェクト　水効果 単発水飛沫　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
