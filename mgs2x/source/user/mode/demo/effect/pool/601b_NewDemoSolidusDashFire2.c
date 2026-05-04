/*********************************************

	エフェクト　効果 時間指定ソリダスダッシュ炎　のランチャー関数

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
	int     delay ;
	int     fade_time ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	int     d3 ;
	int     d4 ;
} Data ;

void *NewDemoSolidusDashFire2_601bLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoSolidusDashFire2( FMATRIX *,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.world = DM_GetObjectMatrix( handle, link->vy ) ;
	work.delay = data->d3 ;
	work.fade_time = data->d4 ;

	return NewDemoSolidusDashFire2( work.world,
	                                work.delay,
	                                work.fade_time
) ;
}


/*********************************************

	エフェクト　効果 時間指定ソリダスダッシュ炎　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
