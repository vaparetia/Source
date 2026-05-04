/*********************************************

	エフェクト　効果 ソリダス集束パーティクル　のランチャー関数

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
	void    *body ;
	void    *evmobj ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
	IVECTOR d4 ;
} Data ;

void *NewDemoSolidusEnergyPrim_6027Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoSolidusEnergyPrim( int ,void *,void * ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.body = DM_GetObjectObject( handle ) ;
	link = &data->d4 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.evmobj = DM_GetEvmObjObject( handle ) ;

	return NewDemoSolidusEnergyPrim( work.name,
	                                 work.body,
	                                 work.evmobj
) ;
}


/*********************************************

	エフェクト　効果 ソリダス集束パーティクル　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
