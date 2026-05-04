/*********************************************

	エフェクト　煙_ハッチガス　のランチャー関数

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
	FVECTOR *pos ;
	FVECTOR *vec ;
	float   width ;
	float   height ;
	int     col ;
} Work ;

typedef struct data_t {
	int     d0 ;
	FVECTOR d1 ;
	FVECTOR d2 ;
	float   d4 ;
	float   d5 ;
	IVECTOR d6 ;
} Data ;

void *NewPeterBlood_Gas_710fLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewPeterBlood_Gas( int ,FVECTOR *,FVECTOR *,float ,float ,int  ) ;

	FVECTOR * rot ;
	FVECTOR  temp ;
	IVECTOR * col ;

	work.name = data->d0 ;
	work.pos = &data->d1 ;
	rot = &data->d2 ;
	DM_GetFMatrixM_XYZ( &temp, rot, 2.000000  ) ;
	work.vec = &temp ;
	work.width = data->d4 ;
	work.height = data->d5 ;
	col = &data->d6 ;
	work.col = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;

	return NewPeterBlood_Gas( work.name,
	                          work.pos,
	                          work.vec,
	                          work.width,
	                          work.height,
	                          work.col
) ;
}


/*********************************************

	エフェクト　煙_ハッチガス　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
