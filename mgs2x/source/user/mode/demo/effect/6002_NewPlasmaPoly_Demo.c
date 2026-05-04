//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　効果 プラズマ（ポリ）　のランチャー関数

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
	int     life ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	IVECTOR d3 ;
	int     d6 ;
	int     d7 ;
	int     d8 ;
	int     d9 ;
	int     d10 ;
} Data ;

void *NewPlasmaPoly_Demo_6002Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewPlasmaPoly_Demo( FVECTOR *,FVECTOR *,int ,int ,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;
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
	max = data->d9 ;
	min = data->d10 ;
	work.life = (max - min) ;

	return NewPlasmaPoly_Demo( work.pos0,
	                           work.pos1,
	                           work.radius,
	                           work.width,
	                           work.branch,
	                           work.life
) ;
}


/*********************************************

	エフェクト　効果 プラズマ（ポリ）　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
