//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　装備品 スネーク　のランチャー関数

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
	void    *parent ;
	void    *body ;
	int     at_id ;
	int     flag ;
} Work ;

typedef struct data_t {
	IVECTOR d1 ;
	int     d4 ;
} Data ;

void *NewPutAttachments_1006Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewPutAttachments( void *,void *,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.parent = DM_NULL(  ) ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.body = DM_GetObjectObject( handle ) ;
	work.at_id = ( int ) 1.000000  ;
	work.flag = data->d4 ;

	return NewPutAttachments( work.parent,
	                          work.body,
	                          work.at_id,
	                          work.flag
) ;
}


/*********************************************

	エフェクト　装備品 スネーク　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
