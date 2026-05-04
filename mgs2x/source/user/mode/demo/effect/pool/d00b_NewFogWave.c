/*********************************************

	エフェクト　レイ_衝撃波　のランチャー関数

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
	FMATRIX *root ;
	int     size ;
	int     speed ;
	int     height ;
} Work ;

typedef struct data_t {
	FVECTOR d1 ;
	FVECTOR d2 ;
	int     d5 ;
	int     d6 ;
	int     d7 ;
} Data ;

void *NewFogWave_d00bLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewFogWave( FMATRIX *,int ,int ,int  ) ;

	FMATRIX  mat ;
	FVECTOR * pos ;
	FVECTOR * rot ;

	_sceVu0UnitMatrix( &mat ) ;
	pos = &data->d1 ;
	rot = &data->d2 ;
	_sceVu0RotMatrix( &mat, &mat, rot ) ;
	_sceVu0TransMatrix( &mat, &mat, pos ) ;
	work.root = &mat ;
	work.size = data->d5 ;
	work.speed = data->d6 ;
	work.height = data->d7 ;

	return NewFogWave( work.root,
	                   work.size,
	                   work.speed,
	                   work.height
) ;
}


/*********************************************

	エフェクト　レイ_衝撃波　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
