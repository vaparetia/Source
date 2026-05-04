//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　血 血飛沫　のランチャー関数

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
	FVECTOR *pos ;
	FVECTOR *force ;
	int     size ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	FVECTOR d3 ;
	FVECTOR d4 ;
	int     d5 ;
} Data ;

void *NewBlood2_Demo_7002Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewBlood2_Demo( FMATRIX *,FVECTOR *,FVECTOR *,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.world = DM_GetObjectMatrix( handle, link->vy ) ;
	work.pos = &data->d3 ;
	work.force = &data->d4 ;
	work.size = data->d5 ;

	return NewBlood2_Demo( work.world,
	                       work.pos,
	                       work.force,
	                       work.size
) ;
}


/*********************************************

	エフェクト　血 血飛沫　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
