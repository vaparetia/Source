/*********************************************

	エフェクト　効果 メッセージ起動Ａ　のランチャー関数

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
	FMATRIX *world ;
} Work ;

typedef struct data_t {
	int     d0 ;
	FVECTOR d2 ;
	FVECTOR d3 ;
} Data ;

void *NewMesgBomb1_600fLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewMesgBomb1( int ,FMATRIX * ) ;

	FMATRIX  mat ;
	FVECTOR * pos ;
	FVECTOR * rot ;

	work.name = data->d0 ;
	_sceVu0UnitMatrix( &mat ) ;
	pos = &data->d2 ;
	rot = &data->d3 ;
	_sceVu0RotMatrix( &mat, &mat, rot ) ;
	_sceVu0TransMatrix( &mat, &mat, pos ) ;
	work.world = &mat ;

	return NewMesgBomb1( work.name,
	                     work.world
) ;
}


/*********************************************

	エフェクト　効果 メッセージ起動Ａ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
