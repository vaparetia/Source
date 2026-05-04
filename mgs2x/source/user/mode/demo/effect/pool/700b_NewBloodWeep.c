/*********************************************

	エフェクト　血_天井滴り血　のランチャー関数

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
	FMATRIX *world ;
	int     nLife ;
	int     nDrip ;
	float   fKillY ;
	float   speed ;
	float   drop_size ;
	float   drip_rad ;
	float   nLenter ;
	int     color ;
	int     nSeed ;
	int     nFlag ;
} Work ;

typedef struct data_t {
	FVECTOR d1 ;
	FVECTOR d2 ;
	int     d5 ;
	int     d6 ;
	float   d7 ;
	float   d8 ;
	float   d9 ;
	float   d10 ;
	float   d11 ;
	IVECTOR d12 ;
	int     d13 ;
	int     d14 ;
} Data ;

void *NewBloodWeep_700bLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewBloodWeep( FMATRIX *,int ,int ,float ,float ,float ,float ,float ,int ,int ,int  ) ;

	FMATRIX  mat ;
	FVECTOR * pos ;
	FVECTOR * rot ;
	IVECTOR * col ;

	_sceVu0UnitMatrix( &mat ) ;
	pos = &data->d1 ;
	rot = &data->d2 ;
	_sceVu0RotMatrix( &mat, &mat, rot ) ;
	_sceVu0TransMatrix( &mat, &mat, pos ) ;
	work.world = &mat ;
	work.nLife = data->d5 ;
	work.nDrip = data->d6 ;
	work.fKillY = data->d7 ;
	work.speed = data->d8 ;
	work.drop_size = data->d9 ;
	work.drip_rad = data->d10 ;
	work.nLenter = data->d11 ;
	col = &data->d12 ;
	work.color = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;
	work.nSeed = data->d13 ;
	work.nFlag = data->d14 ;

	return NewBloodWeep( work.world,
	                     work.nLife,
	                     work.nDrip,
	                     work.fKillY,
	                     work.speed,
	                     work.drop_size,
	                     work.drip_rad,
	                     work.nLenter,
	                     work.color,
	                     work.nSeed,
	                     work.nFlag
) ;
}


/*********************************************

	エフェクト　血_天井滴り血　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
