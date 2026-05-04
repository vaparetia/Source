/*********************************************

	エフェクト　効果 衝撃波　のランチャー関数

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
	float   radius ;
	int     color ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	FVECTOR d1 ;
	float   d3 ;
	IVECTOR d4 ;
} Data ;

void *NewSonicWave_6012Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewSonicWave( FVECTOR *,FVECTOR *,float ,int  ) ;

	FVECTOR * rot ;
	FVECTOR  temp ;
	IVECTOR * col ;

	work.pos = &data->d0 ;
	rot = &data->d1 ;
	DM_GetFMatrixM_ZXY( &temp, rot, 2.000000  ) ;
	work.vec = &temp ;
	work.radius = data->d3 ;
	col = &data->d4 ;
	work.color = (((col->vx | (col->vy * ( int ) 256.000000 )) | ((col->vz * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (((col->vw * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 )) ;

	return NewSonicWave( work.pos,
	                     work.vec,
	                     work.radius,
	                     work.color
) ;
}


/*********************************************

	エフェクト　効果 衝撃波　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
