/*********************************************

	エフェクト　効果 ノード端末プラズマ　のランチャー関数

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
	int     color ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
	IVECTOR d4 ;
} Data ;

void *NewDemoBodyPlasma_6020Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoBodyPlasma( int ,void *,int  ) ;

	IVECTOR * link ;
	void    * handle ;
	IVECTOR * col ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.body = DM_GetObjectObject( handle ) ;
	col = &data->d4 ;
	work.color = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;

	return NewDemoBodyPlasma( work.name,
	                          work.body,
	                          work.color
) ;
}


/*********************************************

	エフェクト　効果 ノード端末プラズマ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
