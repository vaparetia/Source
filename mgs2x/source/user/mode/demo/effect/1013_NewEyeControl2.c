//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　視線制御 ダミーポイント　のランチャー関数

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
	int     target_name ;
	FVECTOR *rmax ;
	FVECTOR *rmin ;
	FVECTOR *lmax ;
	FVECTOR *lmin ;
	int     flag ;
} Work ;

typedef struct data_t {
	int     d0 ;
   int bp_pad[3];
	IVECTOR d1 ;
	int     d4 ;
	float   d5 ;
	float   d6 ;
	float   d7 ;
	float   d8 ;
} Data ;

void *NewEyeControl2_1013Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewEyeControl2( int ,void *,int ,FVECTOR *,FVECTOR *,FVECTOR *,FVECTOR *,int  ) ;

	IVECTOR * link ;
	void    * handle ;
	float    upper ;
	float    under ;
	float    inner ;
	float    outer ;
	FVECTOR  temp0 ;
	FVECTOR  temp1 ;
	FVECTOR  temp2 ;
	FVECTOR  temp3 ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.body = DM_GetObjectObject( handle ) ;
	work.target_name = data->d4 ;
	upper = data->d5 ;
	under = data->d6 ;
	inner = data->d7 ;
	outer = data->d8 ;
	DM_GetRotToRadVec( &temp0, under, inner, 0.000000 , 0.000000  ) ;
	work.rmax = &temp0 ;
	DM_GetRotToRadVec( &temp1, upper, outer, 0.000000 , 0.000000  ) ;
	work.rmin = &temp1 ;
	DM_GetRotToRadVec( &temp2, under, (outer * -1.000000 ), 0.000000 , 0.000000  ) ;
	work.lmax = &temp2 ;
	DM_GetRotToRadVec( &temp3, upper, (inner * -1.000000 ), 0.000000 , 0.000000  ) ;
	work.lmin = &temp3 ;
	work.flag = ( int ) 0.000000  ;

	return NewEyeControl2( work.name,
	                       work.body,
	                       work.target_name,
	                       work.rmax,
	                       work.rmin,
	                       work.lmax,
	                       work.lmin,
	                       work.flag
) ;
}


/*********************************************

	エフェクト　視線制御 ダミーポイント　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
