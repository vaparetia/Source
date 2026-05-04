/*********************************************

	エフェクト　水飛沫 足元　のランチャー関数

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
	void    *control ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
} Data ;

void *NewFootSplash_0103Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewFootSplash( void *,void * ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.object = DM_GetObjectObject( handle ) ;
	work.control = DM_GetObjectControl( handle ) ;

	return NewFootSplash( work.object,
	                      work.control
) ;
}


/*********************************************

	エフェクト　水飛沫 足元　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
