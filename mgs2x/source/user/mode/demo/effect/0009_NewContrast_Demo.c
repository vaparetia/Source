//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　コントラスト　のランチャー関数

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
	int     r ;
	int     g ;
	int     b ;
	int     a ;
	int     nega_posi_flag ;
	int     time ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	int     d1 ;
	int     d2 ;
	int     d3 ;
} Data ;

void *NewContrast_Demo_0009Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewContrast_Demo( int ,int ,int ,int ,int ,int  ) ;

	IVECTOR * col ;
	int      max ;
	int      min ;

	col = &data->d0 ;
	work.r = col->vx ;
	work.g = col->vy ;
	work.b = col->vz ;
	work.a = col->vw ;
	work.nega_posi_flag = data->d1 ;
	max = data->d2 ;
	min = data->d3 ;
	work.time = (max - min) ;

	return NewContrast_Demo( work.r,
	                         work.g,
	                         work.b,
	                         work.a,
	                         work.nega_posi_flag,
	                         work.time
) ;
}


/*********************************************

	エフェクト　コントラスト　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
