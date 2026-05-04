//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　水効果_キャラ波影響　のランチャー関数

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
	void    *object ;
	int     type ;
	int     flag ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	int     d3 ;
} Data ;

void *NewSeaSurfaceReactionObject_b004Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewSeaSurfaceReactionObject( void *,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.object = DM_GetObjectObject( handle ) ;
	work.type = data->d3 ;
	work.flag = ( int ) 0.000000  ;

	return NewSeaSurfaceReactionObject( work.object,
	                                    work.type,
	                                    work.flag
) ;
}


/*********************************************

	エフェクト　水効果_キャラ波影響　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
