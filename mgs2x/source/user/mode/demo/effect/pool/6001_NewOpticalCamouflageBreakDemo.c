/*********************************************

	エフェクト　効果 光学迷彩　のランチャー関数

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
	void    *objs ;
	int     type ;
	int     start_time ;
	int     end_time ;
	int     color ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	int     d3 ;
	int     d4 ;
	int     d5 ;
	int     d6 ;
	IVECTOR d7 ;
} Data ;

void *NewOpticalCamouflageBreakDemo_6001Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewOpticalCamouflageBreakDemo( void *,int ,int ,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;
	int      fps ;
	IVECTOR * col ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.objs = DM_GetObjectDgObjs( handle ) ;
	fps = (data->d3 / ( int ) 5.000000 ) ;
	work.type = data->d4 ;
	work.start_time = (data->d5 * fps) ;
	work.end_time = (data->d6 * fps) ;
	col = &data->d7 ;
	work.color = ((col->vx | (col->vy * ( int ) 256.000000 )) | ((col->vz * ( int ) 256.000000 ) * ( int ) 256.000000 )) ;

	return NewOpticalCamouflageBreakDemo( work.objs,
	                                      work.type,
	                                      work.start_time,
	                                      work.end_time,
	                                      work.color
) ;
}


/*********************************************

	エフェクト　効果 光学迷彩　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
