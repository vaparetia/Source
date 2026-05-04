/*********************************************

	エフェクト　煙_ハリアー旋回筋雲　のランチャー関数

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
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
} Data ;

void *NewDemoHarEffect_7105Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoHarEffect( int ,void * ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.body = DM_GetObjectObject( handle ) ;

	return NewDemoHarEffect( work.name,
	                         work.body
) ;
}


/*********************************************

	エフェクト　煙_ハリアー旋回筋雲　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
