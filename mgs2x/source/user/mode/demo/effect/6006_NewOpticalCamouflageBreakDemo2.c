//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　効果 光学迷彩２　のランチャー関数

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
	void    *objs ;
	int     type ;
	int     start_time ;
	int     end_time ;
	int     color ;
} Work ;

typedef struct data_t {
	int     d0 ;
   int bp_pad[3];
	IVECTOR d1 ;
	int     d4 ;
	int     d5 ;
	int     d6 ;
	int     d7 ;
	int     d8 ;
   int bp_pad1[3];
	IVECTOR d9 ;
} Data ;

void *NewOpticalCamouflageBreakDemo2_6006Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewOpticalCamouflageBreakDemo2( int ,void *,int ,int ,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;
	float    fps ;
	int      frame ;
	int      min ;
	int      max ;
	IVECTOR * col ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.objs = DM_GetObjectDgObjs( handle ) ;
	fps = (( float ) 300.000000  / data->d4) ;
	work.type = data->d5 ;
	frame = (((data->d6 * ( int ) 2.000000 ) * ( int ) 5.000000 ) / fps) ;
	min = data->d7 ;
	max = data->d8 ;
	work.start_time = ((frame - min) * fps) ;
	work.end_time = ((max - min) * fps) ;
	col = &data->d9 ;
	work.color = ((col->vx | (col->vy * ( int ) 256.000000 )) | ((col->vz * ( int ) 256.000000 ) * ( int ) 256.000000 )) ;

	return NewOpticalCamouflageBreakDemo2( work.name,
	                                       work.objs,
	                                       work.type,
	                                       work.start_time,
	                                       work.end_time,
	                                       work.color
) ;
}


/*********************************************

	エフェクト　効果 光学迷彩２　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
