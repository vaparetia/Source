//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　水飛沫 身体　のランチャー関数

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
	void    *control ;
	int     model ;
	int     name ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	int     d4 ;
	int     d5 ;
} Data ;

void *NewBodySplash3_0104Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewBodySplash3( void *,void *,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.objs = DM_GetObjectDgObjs( handle ) ;
	work.control = DM_GetObjectControl( handle ) ;
	work.model = data->d4 ;
	work.name = data->d5 ;

	return NewBodySplash3( work.objs,
	                       work.control,
	                       work.model,
	                       work.name
) ;
}


/*********************************************

	エフェクト　水飛沫 身体　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
