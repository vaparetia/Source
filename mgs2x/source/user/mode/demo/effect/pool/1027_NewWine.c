/*********************************************

	エフェクト　装備品 ワイン　のランチャー関数

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
	int     name ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	int     d3 ;
} Data ;

void *NewWine_1027Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewWine( FMATRIX *,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.world = DM_GetObjectMatrix( handle, link->vy ) ;
	work.name = data->d3 ;

	return NewWine( work.world,
	                work.name
) ;
}


/*********************************************

	エフェクト　装備品 ワイン　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
