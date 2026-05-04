/*********************************************

	エフェクト　影 ボリューミックシャドーライン　のランチャー関数

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
	FVECTOR *light_pos ;
	float   reach ;
	float   back_shift ;
	int     model_name ;
	int     add_sub ;
	int     life ;
	int     color ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	IVECTOR d3 ;
	float   d6 ;
	float   d7 ;
	int     d8 ;
	int     d9 ;
	int     d10 ;
	int     d11 ;
	IVECTOR d12 ;
} Data ;

void *NewBodyShadowVolume_Demo_100aLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewBodyShadowVolume_Demo( void *,FVECTOR *,float ,float ,int ,int ,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;
	int      max ;
	int      min ;
	IVECTOR * col ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.objs = DM_GetObjectDgObjs( handle ) ;
	link = &data->d3 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.light_pos = DM_GetObjectPos( handle, link->vy ) ;
	work.reach = data->d6 ;
	work.back_shift = data->d7 ;
	work.model_name = data->d8 ;
	work.add_sub = data->d9 ;
	max = data->d10 ;
	min = data->d11 ;
	work.life = (max - min) ;
	col = &data->d12 ;
	work.color = (((col->vx | (col->vy * ( int ) 256.000000 )) | ((col->vz * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (((col->vw * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 )) ;

	return NewBodyShadowVolume_Demo( work.objs,
	                                 work.light_pos,
	                                 work.reach,
	                                 work.back_shift,
	                                 work.model_name,
	                                 work.add_sub,
	                                 work.life,
	                                 work.color
) ;
}


/*********************************************

	エフェクト　影 ボリューミックシャドーライン　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
