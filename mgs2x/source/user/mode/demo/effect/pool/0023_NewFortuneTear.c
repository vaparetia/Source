/*********************************************

	エフェクト　フォーチュン落ち涙　のランチャー関数

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
	float   spread_y ;
	float   gravity ;
	float   size ;
	int     tear_tex ;
	int     add_RGBA ;
	int     sub_RGBA ;
	int     sprd_num ;
	int     sprd_life_min ;
	int     sprd_life_max ;
	int     sprd_rad ;
	float   sprd_spd_min ;
	float   sprd_spd_max ;
	float   sprd_spd_grav ;
	float   sprd_size_min ;
	float   sprd_size_max ;
	int     sprd_tex ;
	int     sprd_RGBA ;
	int     nSeed ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	FVECTOR d1 ;
	float   d3 ;
	float   d4 ;
	float   d5 ;
	int     d6 ;
	IVECTOR d7 ;
	IVECTOR d8 ;
	int     d9 ;
	int     d10 ;
	int     d11 ;
	int     d12 ;
	float   d13 ;
	float   d14 ;
	float   d15 ;
	float   d16 ;
	float   d17 ;
	int     d18 ;
	IVECTOR d19 ;
	int     d20 ;
} Data ;

void *NewFortuneTear_0023Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewFortuneTear( FVECTOR *,FVECTOR *,float ,float ,float ,int ,int ,int ,int ,int ,int ,int ,float ,float ,float ,float ,float ,int ,int ,int  ) ;

	FVECTOR * rot ;
	FVECTOR  temp ;
	IVECTOR * col ;

	work.pos = &data->d0 ;
	rot = &data->d1 ;
	DM_GetFMatrixM_XYZ( &temp, rot, 2.000000  ) ;
	work.vec = &temp ;
	work.spread_y = data->d3 ;
	work.gravity = data->d4 ;
	work.size = data->d5 ;
	work.tear_tex = data->d6 ;
	col = &data->d7 ;
	work.add_RGBA = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;
	col = &data->d8 ;
	work.sub_RGBA = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;
	work.sprd_num = data->d9 ;
	work.sprd_life_min = data->d10 ;
	work.sprd_life_max = data->d11 ;
	work.sprd_rad = data->d12 ;
	work.sprd_spd_min = data->d13 ;
	work.sprd_spd_max = data->d14 ;
	work.sprd_spd_grav = data->d15 ;
	work.sprd_size_min = data->d16 ;
	work.sprd_size_max = data->d17 ;
	work.sprd_tex = data->d18 ;
	col = &data->d19 ;
	work.sprd_RGBA = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;
	work.nSeed = data->d20 ;

	return NewFortuneTear( work.pos,
	                       work.vec,
	                       work.spread_y,
	                       work.gravity,
	                       work.size,
	                       work.tear_tex,
	                       work.add_RGBA,
	                       work.sub_RGBA,
	                       work.sprd_num,
	                       work.sprd_life_min,
	                       work.sprd_life_max,
	                       work.sprd_rad,
	                       work.sprd_spd_min,
	                       work.sprd_spd_max,
	                       work.sprd_spd_grav,
	                       work.sprd_size_min,
	                       work.sprd_size_max,
	                       work.sprd_tex,
	                       work.sprd_RGBA,
	                       work.nSeed
) ;
}


/*********************************************

	エフェクト　フォーチュン落ち涙　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
