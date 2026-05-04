/*********************************************

	エフェクト　火器_リニアガン溜め　のランチャー関数

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
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
} Data ;

void *NewLinerGunInitEffect_1019Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewLinerGunInitEffect( FMATRIX * ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.world = DM_GetObjectMatrix( handle, link->vy ) ;

	return NewLinerGunInitEffect( work.world
) ;
}


/*********************************************

	エフェクト　火器_リニアガン溜め　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
