//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　フレームスキップスロー　のランチャー関数

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
	int     time ;
	int     capture_interval ;
	int     bright_time ;
	int     alpha_time ;
	int     flag ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
	int     d2 ;
	int     d3 ;
	int     d4 ;
	int     d5 ;
} Data ;

void *NewCrossFadeEffectCustom_0013Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewCrossFadeEffectCustom( int ,int ,int ,int ,int  ) ;

	int      max ;
	int      min ;
	float    fps ;
	int      temp ;

	max = data->d0 ;
	min = data->d1 ;
	fps = (( float ) 300.000000  / data->d2) ;
	work.time = ((max - min) * fps) ;
	temp = data->d3 ;
	work.capture_interval = (temp * ( int ) 10.000000 ) ;
	temp = data->d4 ;
	work.bright_time = (temp * ( int ) 10.000000 ) ;
	temp = data->d5 ;
	work.alpha_time = (temp * ( int ) 10.000000 ) ;
	work.flag = ( int ) 0.000000  ;

	return NewCrossFadeEffectCustom( work.time,
	                                 work.capture_interval,
	                                 work.bright_time,
	                                 work.alpha_time,
	                                 work.flag
) ;
}


/*********************************************

	エフェクト　フレームスキップスロー　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
