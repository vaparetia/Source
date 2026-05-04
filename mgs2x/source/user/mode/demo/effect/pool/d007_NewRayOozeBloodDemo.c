/*********************************************

	エフェクト　レイ_滲み血　のランチャー関数

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
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
} Data ;

void *NewRayOozeBloodDemo_d007Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewRayOozeBloodDemo( int ,void * ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.objs = DM_GetObjectDgObjs( handle ) ;

	return NewRayOozeBloodDemo( work.name,
	                            work.objs
) ;
}


/*********************************************

	エフェクト　レイ_滲み血　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
