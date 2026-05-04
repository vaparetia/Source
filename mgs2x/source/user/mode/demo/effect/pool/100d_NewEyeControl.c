/*********************************************

	エフェクト　視線制御　のランチャー関数

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
	void    *target ;
	FVECTOR *rmax ;
	FVECTOR *rmin ;
	FVECTOR *lmax ;
	FVECTOR *lmin ;
	int     flag ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
	IVECTOR d4 ;
	float   d7 ;
	float   d8 ;
	float   d10 ;
	float   d11 ;
	float   d13 ;
	float   d14 ;
	float   d16 ;
	float   d17 ;
} Data ;

void *NewEyeControl_100dLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewEyeControl( int ,void *,void *,FVECTOR *,FVECTOR *,FVECTOR *,FVECTOR *,int  ) ;

	IVECTOR * link ;
	void    * handle ;
	FVECTOR  temp0 ;
	FVECTOR  temp1 ;
	FVECTOR  temp2 ;
	FVECTOR  temp3 ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.body = DM_GetObjectObject( handle ) ;
	link = &data->d4 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.target = DM_GetObjectMatrix( handle, link->vy ) ;
	DM_GetRotToRadVec( &temp0, data->d7, data->d8, 0.000000 , 0.000000  ) ;
	work.rmax = &temp0 ;
	DM_GetRotToRadVec( &temp1, data->d10, data->d11, 0.000000 , 0.000000  ) ;
	work.rmin = &temp1 ;
	DM_GetRotToRadVec( &temp2, data->d13, data->d14, 0.000000 , 0.000000  ) ;
	work.lmax = &temp2 ;
	DM_GetRotToRadVec( &temp3, data->d16, data->d17, 0.000000 , 0.000000  ) ;
	work.lmin = &temp3 ;
	work.flag = ( int ) 0.000000  ;

	return NewEyeControl( work.name,
	                      work.body,
	                      work.target,
	                      work.rmax,
	                      work.rmin,
	                      work.lmax,
	                      work.lmin,
	                      work.flag
) ;
}


/*********************************************

	エフェクト　視線制御　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
