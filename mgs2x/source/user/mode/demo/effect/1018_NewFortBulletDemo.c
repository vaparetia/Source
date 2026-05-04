//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　火器 リニアガン銃弾　のランチャー関数

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
	FVECTOR *aim ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	FVECTOR d3 ;
} Data ;

void *NewFortBulletDemo_1018Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewFortBulletDemo( void *,FVECTOR * ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.objs = DM_GetObjectDgObjs( handle ) ;
	work.aim = &data->d3 ;

	return NewFortBulletDemo( work.objs,
	                          work.aim
) ;
}


/*********************************************

	エフェクト　火器 リニアガン銃弾　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
