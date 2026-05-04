/*********************************************

	エフェクト　頂点ストリームアニメ　のランチャー関数

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
	int     model ;
	void    *obj ;
	FVECTOR *pos ;
	SVECTOR *rot ;
	FVECTOR *scale ;
	FVECTOR *bound_u ;
	FVECTOR *bound_l ;
	int     mode ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
	IVECTOR d2 ;
	FVECTOR d5 ;
	SVECTOR d6 ;
	FVECTOR d7 ;
	FVECTOR d8 ;
	FVECTOR d9 ;
	int     d10 ;
} Data ;

void *NewPutSTanimeObjectCall_4000Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewPutSTanimeObjectCall( int ,int ,void *,FVECTOR *,SVECTOR *,FVECTOR *,FVECTOR *,FVECTOR *,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	work.model = data->d1 ;
	link = &data->d2 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.obj = DM_GetObjectDgObj( handle, link->vy ) ;
	work.pos = &data->d5 ;
	work.rot = &data->d6 ;
	work.scale = &data->d7 ;
	work.bound_u = &data->d8 ;
	work.bound_l = &data->d9 ;
	work.mode = data->d10 ;

	return NewPutSTanimeObjectCall( work.name,
	                                work.model,
	                                work.obj,
	                                work.pos,
	                                work.rot,
	                                work.scale,
	                                work.bound_u,
	                                work.bound_l,
	                                work.mode
) ;
}


/*********************************************

	エフェクト　頂点ストリームアニメ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
