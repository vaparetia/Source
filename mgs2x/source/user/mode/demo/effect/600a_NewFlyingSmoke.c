//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　効果 壁這う煙　のランチャー関数

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
	FVECTOR *vector ;
	int     life ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	FVECTOR d1 ;
	float   d3 ;
	int     d5 ;
	int     d6 ;
} Data ;

void *NewFlyingSmoke_600aLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewFlyingSmoke( FVECTOR *,FVECTOR *,int  ) ;

	FVECTOR * rot ;
	FVECTOR  vec ;
	float    len ;
	int      max ;
	int      min ;

	work.pos = &data->d0 ;
	rot = &data->d1 ;
	DM_GetFMatrixM_ZXY( &vec, rot, 2.000000  ) ;
	len = data->d3 ;
	_sceVu0ScaleVector( &vec, &vec, len ) ;
	work.vector = &vec ;
	max = data->d5 ;
	min = data->d6 ;
	work.life = (max - min) ;

	return NewFlyingSmoke( work.pos,
	                       work.vector,
	                       work.life
) ;
}


/*********************************************

	エフェクト　効果 壁這う煙　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
