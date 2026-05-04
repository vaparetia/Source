//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　水効果_球水飛沫　のランチャー関数

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
	FVECTOR *pos ;
	float   rad ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	float   d3 ;
} Data ;

void *NewAutoSplush_b005Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewAutoSplush( FVECTOR *,float  ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.pos = DM_GetObjectPos( handle, link->vy ) ;
	work.rad = data->d3 ;

	return NewAutoSplush( work.pos,
	                      work.rad
) ;
}


/*********************************************

	エフェクト　水効果_球水飛沫　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
