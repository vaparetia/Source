//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　モデル 補間残像　のランチャー関数

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
	int     disp_f ;
	SVECTOR *color ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	IVECTOR d3 ;
	int     d6 ;
	IVECTOR d7 ;
} Data ;

void *NewInterPoly_Demo_4002Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewInterPoly_Demo( FVECTOR *,FVECTOR *,int ,SVECTOR * ) ;

	IVECTOR * link ;
	void    * handle ;
	IVECTOR * col ;
	SVECTOR  cols ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.pos0 = DM_GetObjectPos( handle, link->vy ) ;
	link = &data->d3 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.pos1 = DM_GetObjectPos( handle, link->vy ) ;
	work.disp_f = data->d6 ;
	col = &data->d7 ;
	DM_IVecToSVector( col, &cols ) ;
	work.color = &cols ;

	return NewInterPoly_Demo( work.pos0,
	                          work.pos1,
	                          work.disp_f,
	                          work.color
) ;
}


/*********************************************

	エフェクト　モデル 補間残像　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
