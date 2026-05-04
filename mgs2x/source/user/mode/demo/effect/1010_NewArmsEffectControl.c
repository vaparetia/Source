//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　火器 簡易エフェクト　のランチャー関数

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
	int     arms_id ;
	FMATRIX *world ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
   int bp_pad[2];
	IVECTOR d2 ;
} Data ;

void *NewArmsEffectControl_1010Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewArmsEffectControl( int ,int ,FMATRIX * ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	work.arms_id = data->d1 ;
	link = &data->d2 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.world = DM_GetObjectMatrix( handle, link->vy ) ;

	return NewArmsEffectControl( work.name,
	                             work.arms_id,
	                             work.world
) ;
}


/*********************************************

	エフェクト　火器 簡易エフェクト　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
