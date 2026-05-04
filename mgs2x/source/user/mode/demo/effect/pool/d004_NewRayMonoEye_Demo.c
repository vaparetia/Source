/*********************************************

	エフェクト　量産レイ_目　のランチャー関数

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
	void    *object ;
	int     con_name ;
	int     color ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
	int     d4 ;
	IVECTOR d5 ;
} Data ;

void *NewRayMonoEye_Demo_d004Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewRayMonoEye_Demo( int ,void *,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;
	IVECTOR * col ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.object = DM_GetObjectMatrix( handle, link->vy ) ;
	work.con_name = data->d4 ;
	col = &data->d5 ;
	work.color = (((col->vx | (col->vy * ( int ) 256.000000 )) | ((col->vz * ( int ) 256.000000 ) * ( int ) 256.000000 )) | (((col->vw * ( int ) 256.000000 ) * ( int ) 256.000000 ) * ( int ) 256.000000 )) ;

	return NewRayMonoEye_Demo( work.name,
	                           work.object,
	                           work.con_name,
	                           work.color
) ;
}


/*********************************************

	エフェクト　量産レイ_目　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
