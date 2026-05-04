//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　沈む太陽　のランチャー関数

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
	FMATRIX *root ;
	float   speed ;
	float   scale ;
	int     w ;
	int     h ;
	int     col ;
	int     alpha ;
} Work ;

typedef struct data_t {
	int     d0 ;
	FVECTOR d2 ;
	FVECTOR d3 ;
	float   d6 ;
	float   d7 ;
	int     d8 ;
	int     d9 ;
	IVECTOR d10 ;
	int     d11 ;
} Data ;

void *NewDemoSun_001fLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoSun( int ,FMATRIX *,float ,float ,int ,int ,int ,int  ) ;

	FMATRIX  mat ;
	FVECTOR * pos ;
	FVECTOR * rot ;
	IVECTOR * col ;

	work.name = data->d0 ;
	_sceVu0UnitMatrix( &mat ) ;
	pos = &data->d2 ;
	rot = &data->d3 ;
	_sceVu0RotMatrix( &mat, &mat, rot ) ;
	_sceVu0TransMatrix( &mat, &mat, pos ) ;
	work.root = &mat ;
	work.speed = data->d6 ;
	work.scale = data->d7 ;
	work.w = data->d8 ;
	work.h = data->d9 ;
	col = &data->d10 ;
	work.col = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;
	work.alpha = data->d11 ;

	return NewDemoSun( work.name,
	                   work.root,
	                   work.speed,
	                   work.scale,
	                   work.w,
	                   work.h,
	                   work.col,
	                   work.alpha
) ;
}


/*********************************************

	エフェクト　沈む太陽　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
