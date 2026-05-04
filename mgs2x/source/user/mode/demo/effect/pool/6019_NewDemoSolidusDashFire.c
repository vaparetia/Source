/*********************************************

	エフェクト　効果 ソリダスダッシュ炎　のランチャー関数

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
	int     *flag ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
} Data ;

void *NewDemoSolidusDashFire_6019Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoSolidusDashFire( FMATRIX *,int * ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.world = DM_GetObjectMatrix( handle, link->vy ) ;
	work.flag = DM_NULL(  ) ;

	return NewDemoSolidusDashFire( work.world,
	                               work.flag
) ;
}


/*********************************************

	エフェクト　効果 ソリダスダッシュ炎　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
