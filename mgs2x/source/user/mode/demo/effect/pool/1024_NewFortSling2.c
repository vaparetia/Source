/*********************************************

	エフェクト　装備品 裸フォーチュンスリング　のランチャー関数

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
	void    *object ;
	void    *body ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	IVECTOR d3 ;
} Data ;

void *NewFortSling2_1024Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewFortSling2( void *,void * ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.object = DM_GetObjectObject( handle ) ;
	link = &data->d3 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.body = DM_GetObjectDgObjs( handle ) ;

	return NewFortSling2( work.object,
	                      work.body
) ;
}


/*********************************************

	エフェクト　装備品 裸フォーチュンスリング　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
