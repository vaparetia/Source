/*********************************************

	エフェクト　効果 ソリダス刀光　のランチャー関数

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
	FMATRIX *world ;
	int     kind ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
	int     d4 ;
} Data ;

void *NewDemoSolidusBladeLight_6026Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoSolidusBladeLight( int ,FMATRIX *,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.world = DM_GetObjectMatrix( handle, link->vy ) ;
	work.kind = data->d4 ;

	return NewDemoSolidusBladeLight( work.name,
	                                 work.world,
	                                 work.kind
) ;
}


/*********************************************

	エフェクト　効果 ソリダス刀光　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
