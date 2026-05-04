//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　手錠モデル表示　のランチャー関数

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
	int     model_name ;
	void    *object1 ;
	int     objnum1 ;
	void    *object2 ;
	int     objnum2 ;
} Work ;

typedef struct data_t {
	int     d0 ;
	int     d1 ;
	IVECTOR d2 ;
	IVECTOR d5 ;
} Data ;

void *NewHandcuff_called_3015Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewHandcuff_called( int ,int ,void *,int ,void *,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	work.model_name = data->d1 ;
	link = &data->d2 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.object1 = DM_GetObjectObject( handle ) ;
	work.objnum1 = link->vy ;
	link = &data->d5 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.object2 = DM_GetObjectObject( handle ) ;
	work.objnum2 = link->vy ;

	return NewHandcuff_called( work.name,
	                           work.model_name,
	                           work.object1,
	                           work.objnum1,
	                           work.object2,
	                           work.objnum2
) ;
}


/*********************************************

	エフェクト　手錠モデル表示　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
