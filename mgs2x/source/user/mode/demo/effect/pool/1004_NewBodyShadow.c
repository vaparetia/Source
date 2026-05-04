/*********************************************

	エフェクト　影 光源遮り　のランチャー関数

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
	void    *objs ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
} Data ;

void *NewBodyShadow_1004Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewBodyShadow( void * ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.objs = DM_GetObjectDgObjs( handle ) ;

	return NewBodyShadow( work.objs
) ;
}


/*********************************************

	エフェクト　影 光源遮り　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
