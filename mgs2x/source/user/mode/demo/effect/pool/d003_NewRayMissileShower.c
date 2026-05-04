/*********************************************

	エフェクト　レイ_拡散ミサイル　のランチャー関数

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
	void    *world ;
	float   radius ;
	float   angle ;
	float   intense ;
	int     col ;
	int     life ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	float   d3 ;
	float   d4 ;
	float   d5 ;
	IVECTOR d6 ;
	int     d7 ;
	int     d8 ;
} Data ;

void *NewRayMissileShower_d003Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewRayMissileShower( void *,float ,float ,float ,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;
	IVECTOR * col ;
	int      max ;
	int      min ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.world = DM_GetObjectMatrix( handle, link->vy ) ;
	work.radius = data->d3 ;
	work.angle = data->d4 ;
	work.intense = data->d5 ;
	col = &data->d6 ;
	work.col = ((((((col->vx * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 ) | ((col->vy * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (col->vz * ( int ) 256.000000 )) | col->vw) ;
	max = data->d7 ;
	min = data->d8 ;
	work.life = (max - min) ;

	return NewRayMissileShower( work.world,
	                            work.radius,
	                            work.angle,
	                            work.intense,
	                            work.col,
	                            work.life
) ;
}


/*********************************************

	エフェクト　レイ_拡散ミサイル　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
