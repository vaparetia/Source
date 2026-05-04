//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　効果 マズルフラッシュ　のランチャー関数

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
	int     maz_id ;
	FMATRIX *world ;
	int     silence ;
	int     light_mode ;
} Work ;

typedef struct data_t {
	int     d0 ;
	FVECTOR d2 ;
	FVECTOR d3 ;
	int     d6 ;
} Data ;

void *NewCallMazzle_600dLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewCallMazzle( int ,FMATRIX *,int ,int  ) ;

	FMATRIX  mat ;
	FVECTOR * pos ;
	FVECTOR * rot ;

	work.maz_id = data->d0 ;
	_sceVu0UnitMatrix( &mat ) ;
	pos = &data->d2 ;
	rot = &data->d3 ;
	_sceVu0RotMatrix( &mat, &mat, rot ) ;
	_sceVu0TransMatrix( &mat, &mat, pos ) ;
	work.world = &mat ;
	work.silence = data->d6 ;
	work.light_mode = ( int ) 0.000000  ;

	return NewCallMazzle( work.maz_id,
	                      work.world,
	                      work.silence,
	                      work.light_mode
) ;
}


/*********************************************

	エフェクト　効果 マズルフラッシュ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
