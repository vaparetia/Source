//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　煙 汎用　のランチャー関数

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
	FVECTOR *center ;
	float   radius ;
	int     col_r ;
	int     col_g ;
	int     col_b ;
	int     col_a ;
	int     life ;
	int     mode ;
} Work ;

typedef struct data_t {
	FVECTOR d0 ;
	float   d1 ;
	IVECTOR d2 ;
	int     d3 ;
	int     d4 ;
	int     d5 ;
} Data ;

void *NewCommonSmoke_7101Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewCommonSmoke( FVECTOR *,float ,int ,int ,int ,int ,int ,int  ) ;

	IVECTOR * col ;
	int      max ;
	int      min ;

	work.center = &data->d0 ;
	work.radius = data->d1 ;
	col = &data->d2 ;
	work.col_r = col->vx ;
	work.col_g = col->vy ;
	work.col_b = col->vz ;
	work.col_a = col->vw ;
	max = data->d3 ;
	min = data->d4 ;
	work.life = (max - min) ;
	work.mode = data->d5 ;

	return NewCommonSmoke( work.center,
	                       work.radius,
	                       work.col_r,
	                       work.col_g,
	                       work.col_b,
	                       work.col_a,
	                       work.life,
	                       work.mode
) ;
}


/*********************************************

	エフェクト　煙 汎用　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
