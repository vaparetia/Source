/*********************************************

	エフェクト　物理シム・オルガ　のランチャー関数

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
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
} Data ;

void *NewEvmMMOrga_called_3011Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewEvmMMOrga_called( void * ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.object = DM_GetEvmObjObject( handle ) ;

	return NewEvmMMOrga_called( work.object
) ;
}


/*********************************************

	エフェクト　物理シム・オルガ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
