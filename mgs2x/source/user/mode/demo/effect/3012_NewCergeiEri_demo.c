//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　物理シム・関節揺らし　のランチャー関数

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
	int     sample_num ;
	void    *object ;
	int     name ;
} Work ;

typedef struct data_t {
	int     d0 ;
   int bp_pad[3];
	IVECTOR d1 ;
	int     d4 ;
} Data ;

void *NewCergeiEri_demo_3012Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewCergeiEri_demo( int ,void *,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.sample_num = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.object = DM_GetEvmObjObject( handle ) ;
	work.name = data->d4 ;

	return NewCergeiEri_demo( work.sample_num,
	                          work.object,
	                          work.name
) ;
}


/*********************************************

	エフェクト　物理シム・関節揺らし　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
