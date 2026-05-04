/*********************************************

	エフェクト　効果 プラズマ色指定（ポリ）　のランチャー関数

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
	FVECTOR *pos0 ;
	FVECTOR *pos1 ;
	int     radius ;
	int     width ;
	int     branch ;
	int     color ;
	int     life ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	IVECTOR d3 ;
	int     d6 ;
	int     d7 ;
	int     d8 ;
	IVECTOR d9 ;
	int     d10 ;
	int     d11 ;
} Data ;

void *NewPlasmaPolyColor_Demo_602bLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewPlasmaPolyColor_Demo( FVECTOR *,FVECTOR *,int ,int ,int ,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;
	IVECTOR * col ;
	int      max ;
	int      min ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.pos0 = DM_GetObjectPos( handle, link->vy ) ;
	link = &data->d3 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.pos1 = DM_GetObjectPos( handle, link->vy ) ;
	work.radius = data->d6 ;
	work.width = data->d7 ;
	work.branch = data->d8 ;
	col = &data->d9 ;
	work.color = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;
	max = data->d10 ;
	min = data->d11 ;
	work.life = (max - min) ;

	return NewPlasmaPolyColor_Demo( work.pos0,
	                                work.pos1,
	                                work.radius,
	                                work.width,
	                                work.branch,
	                                work.color,
	                                work.life
) ;
}


/*********************************************

	エフェクト　効果 プラズマ色指定（ポリ）　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
