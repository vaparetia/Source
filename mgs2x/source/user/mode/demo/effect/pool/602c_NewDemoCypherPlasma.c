/*********************************************

	エフェクト　効果 サイファー破壊プラズマ　のランチャー関数

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
	float   width ;
	int     color ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	float   d3 ;
	IVECTOR d4 ;
} Data ;

void *NewDemoCypherPlasma_602cLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoCypherPlasma( void *,float ,int  ) ;

	IVECTOR * link ;
	void    * handle ;
	IVECTOR * col ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.body = DM_GetObjectMatrix( handle, link->vy ) ;
	work.width = data->d3 ;
	col = &data->d4 ;
	work.color = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;

	return NewDemoCypherPlasma( work.body,
	                            work.width,
	                            work.color
) ;
}


/*********************************************

	エフェクト　効果 サイファー破壊プラズマ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
