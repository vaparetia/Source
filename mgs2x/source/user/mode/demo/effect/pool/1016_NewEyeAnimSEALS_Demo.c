/*********************************************

	エフェクト　シールズ_目パチ　のランチャー関数

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
	void    *body ;
	int     name ;
	int     type ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	int     d3 ;
	int     d4 ;
} Data ;

void *NewEyeAnimSEALS_Demo_1016Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewEyeAnimSEALS_Demo( void *,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.body = DM_GetObjectObject( handle ) ;
	work.name = data->d3 ;
	work.type = data->d4 ;

	return NewEyeAnimSEALS_Demo( work.body,
	                             work.name,
	                             work.type
) ;
}


/*********************************************

	エフェクト　シールズ_目パチ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
