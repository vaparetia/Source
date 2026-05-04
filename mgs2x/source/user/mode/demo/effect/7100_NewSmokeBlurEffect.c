//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　煙 煙型ブラー　のランチャー関数

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
	int     start_speed ;
	int     end_speed ;
	int     start_size ;
	int     end_size ;
	int     spot_size ;
	int     spot_angle ;
	int     n_prims ;
	int     interval ;
	int     color ;
	int     flag ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	int     d3 ;
	int     d4 ;
	int     d5 ;
	int     d6 ;
	int     d7 ;
	float   d8 ;
	int     d9 ;
	int     d10 ;
	IVECTOR d11 ;
	int     d12 ;
} Data ;

void *NewSmokeBlurEffect_7100Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewSmokeBlurEffect( FMATRIX *,int ,int ,int ,int ,int ,int ,int ,int ,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;
	float    angle ;
	IVECTOR * col ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.world = DM_GetObjectMatrix( handle, link->vy ) ;
	work.start_speed = data->d3 ;
	work.end_speed = data->d4 ;
	work.start_size = data->d5 ;
	work.end_size = data->d6 ;
	work.spot_size = data->d7 ;
	angle = data->d8 ;
	work.spot_angle = ((( int ) 4096.000000  * angle) / ( int ) 360.000000 ) ;
	work.n_prims = data->d9 ;
	work.interval = data->d10 ;
	col = &data->d11 ;
	work.color = ((col->vx | (col->vy * ( int ) 256.000000 )) | ((col->vz * ( int ) 256.000000 ) * ( int ) 256.000000 )) ;
	work.flag = data->d12 ;

	return NewSmokeBlurEffect( work.world,
	                           work.start_speed,
	                           work.end_speed,
	                           work.start_size,
	                           work.end_size,
	                           work.spot_size,
	                           work.spot_angle,
	                           work.n_prims,
	                           work.interval,
	                           work.color,
	                           work.flag
) ;
}


/*********************************************

	エフェクト　煙 煙型ブラー　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
