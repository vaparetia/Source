//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　装備品 スカートＡ　のランチャー関数

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
	void    *body ;
	int     name ;
} Work ;

typedef struct data_t {
	IVECTOR d0 ;
	int     d3 ;
} Data ;

void *NewScnEvm_SkirtA_1022Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewScnEvm_SkirtA( void *,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	link = &data->d0 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.body = DM_GetObjectObject( handle ) ;
	work.name = data->d3 ;

	return NewScnEvm_SkirtA( work.body,
	                         work.name
) ;
}


/*********************************************

	エフェクト　装備品 スカートＡ　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
