/*********************************************

	エフェクト　効果 跳弾　のランチャー関数

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
	FVECTOR *color ;
} Work ;

typedef struct data_t {
	FVECTOR d1 ;
	FVECTOR d2 ;
	IVECTOR d5 ;
} Data ;

void *NewSpark_6000Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewSpark( FMATRIX *,FVECTOR * ) ;

	FMATRIX  mat ;
	FVECTOR * pos ;
	FVECTOR * rot ;
	IVECTOR * c_ivec ;
	FVECTOR  c_fvec ;

	_sceVu0UnitMatrix( &mat ) ;
	pos = &data->d1 ;
	rot = &data->d2 ;
	_sceVu0RotMatrix( &mat, &mat, rot ) ;
	_sceVu0TransMatrix( &mat, &mat, pos ) ;
	work.world = &mat ;
	c_ivec = &data->d5 ;
	_sceVu0ITOF0Vector( &c_fvec, c_ivec ) ;
	work.color = &c_fvec ;

	return NewSpark( work.world,
	                 work.color
) ;
}


/*********************************************

	エフェクト　効果 跳弾　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
