//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　水効果 小粒水飛沫　のランチャー関数

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
	void    *cntrl ;
	int     mdl_code ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	int     d4 ;
} Data ;

void *NewDropBodySplush_b001Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDropBodySplush( void *,void *,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.objs = DM_GetObjectDgObjs( handle ) ;
	work.cntrl = DM_GetObjectControl( handle ) ;
	work.mdl_code = data->d4 ;

	return NewDropBodySplush( work.objs,
	                          work.cntrl,
	                          work.mdl_code
) ;
}


/*********************************************

	エフェクト　水効果 小粒水飛沫　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
