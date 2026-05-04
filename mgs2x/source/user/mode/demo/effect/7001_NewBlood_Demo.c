//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　血 任意血　のランチャー関数

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
	FMATRIX *world ;
	FVECTOR *force ;
	int     size ;
	int     time ;
	int     pat ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	FVECTOR d3 ;
	int     d4 ;
	int     d5 ;
	int     d6 ;
	int     d7 ;
} Data ;

void *NewBlood_Demo_7001Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewBlood_Demo( FMATRIX *,FVECTOR *,int ,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;
	int      max ;
	int      min ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.world = DM_GetObjectMatrix( handle, link->vy ) ;
	work.force = &data->d3 ;
	work.size = data->d4 ;
	max = data->d5 ;
	min = data->d6 ;
	work.time = (max - min) ;
	work.pat = data->d7 ;

	return NewBlood_Demo( work.world,
	                      work.force,
	                      work.size,
	                      work.time,
	                      work.pat
) ;
}


/*********************************************

	エフェクト　血 任意血　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
