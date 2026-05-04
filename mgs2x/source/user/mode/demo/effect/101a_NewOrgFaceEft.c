//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　装備品_オルガ忍シールド　のランチャー関数

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
	void    *control ;
	int     mode ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
	int     d4 ;
} Data ;

void *NewOrgFaceEft_101aLaunch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewOrgFaceEft( int ,void *,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.control = DM_GetObjectControl( handle ) ;
	work.mode = data->d4 ;

	return NewOrgFaceEft( work.name,
	                      work.control,
	                      work.mode
) ;
}


/*********************************************

	エフェクト　装備品_オルガ忍シールド　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
