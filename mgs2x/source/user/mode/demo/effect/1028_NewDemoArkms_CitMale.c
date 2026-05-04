//----------------------------------------------------------------------------
//BP - added stdafx.h to get precompiled MGS_SysCommon.h in Windows
#include "stdafx.h"
//BP
//----------------------------------------------------------------------------
/*********************************************

	エフェクト　人体 ＮＹ男市民　のランチャー関数

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
	void    *object ;
	int     type ;
	int     flag ;
} Work ;

typedef struct data_t {
	int     d0 ;
	IVECTOR d1 ;
	int     d4 ;
	int     d5 ;
} Data ;

void *NewDemoArkms_CitMale_1028Launch( int id, void *argv )
{
	Work  work ;
	Data *data = (Data *)argv ;

	extern void *NewDemoArkms_CitMale( int ,void *,int ,int  ) ;

	IVECTOR * link ;
	void    * handle ;

	work.name = data->d0 ;
	link = &data->d1 ;
	handle = DM_GetObjectHandle( link->vx ) ;
	work.object = DM_GetObjectObject( handle ) ;
	work.type = data->d4 ;
	work.flag = data->d5 ;

	return NewDemoArkms_CitMale( work.name,
	                             work.object,
	                             work.type,
	                             work.flag
) ;
}


/*********************************************

	エフェクト　人体 ＮＹ男市民　のランチャー関数

	Created by DEMO Composer.
 *********************************************/
