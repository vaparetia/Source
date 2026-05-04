//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　レイ_汚水　のランチャー関数

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
	void    *root ;
	int     ray_name ;
} Work ;

typedef struct data_t {
	int     d0 ;
   int bp_pad[3];
	IVECTOR d1 ;
	int     d4 ;
} Data ;

void *NewWaterPollute_demo_d002Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewWaterPollute_demo( int ,void *,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.root = DM_GetObjectMatrix( handle, link->vy ) ;
	work.ray_name = data->d4 ;

	return NewWaterPollute_demo( work.name,
	                             work.root,
	                             work.ray_name
) ;
}


/*********************************************

	エフェクト　レイ_汚水　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
