//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　水飛沫 波涛カラー　のランチャー関数

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
	float   scale ;
	int     alpha ;
} Work ;

typedef struct data_t {
	FVECTOR d1 ;
	FVECTOR d2 ;
	float   d5 ;
	int     d6 ;
	IVECTOR d7 ;
} Data ;

void *NewWaveSplash_Demo2_010cLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewWaveSplash_Demo2( FMATRIX *,float ,int  ) ;

	FMATRIX  mat ;
	FVECTOR * pos ;
	FVECTOR * rot ;
	int      temp ;
	IVECTOR * col ;

	_sceVu0UnitMatrix( &mat ) ;
	pos = &data->d1 ;
	rot = &data->d2 ;
	_sceVu0RotMatrix( &mat, &mat, rot ) ;
	_sceVu0TransMatrix( &mat, &mat, pos ) ;
	work.root = &mat ;
	work.scale = data->d5 ;
	temp = data->d6 ;
	col = &data->d7 ;
	work.alpha = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | (temp & ( int ) 255.000000 )) ;

	return NewWaveSplash_Demo2( work.root,
	                            work.scale,
	                            work.alpha
) ;
}


/*********************************************

	エフェクト　水飛沫 波涛カラー　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
